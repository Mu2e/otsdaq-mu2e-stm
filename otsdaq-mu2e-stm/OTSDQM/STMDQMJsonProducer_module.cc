#include "STMDQMJsonProducer.hh"

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"

#include "artdaq-core-mu2e/Overlays/STMFragment.hh"

#include <artdaq-core/Data/Fragment.hh>
#include <artdaq-core/Data/ContainerFragment.hh>

#include <fstream>
#include <sstream>
#include <ctime>
#include <cmath>
#include <cstdio>

using namespace ots;

// ============================================================
// SpectrumState ctor
// ============================================================

STMDQMJsonProducer::SpectrumState::SpectrumState() {
  hist.resize(NBINS, 0);
  peaks = {
    {"Annihilation", 511.0, 460.0, 0.0},
    {"Cs137", 662.0, 600.0, 0.0},
    {"Co60_1", 1173.0, 1100.0, 0.0},
    {"Co60_2", 1332.0, 1250.0, 0.0}
  };
}

// ============================================================
// Constructor
// ============================================================

STMDQMJsonProducer::STMDQMJsonProducer(Parameters const& p)
  : art::EDAnalyzer(p), cfg_(p())
{}

// ============================================================
// Helpers
// ============================================================

double STMDQMJsonProducer::sigmaFromEnergy(double E, double true_adc) {
  const double A = 300.0;
  const double B = 1.5;
  double R = std::sqrt((A*A)/E + (B*B));
  double adc_per_keV = true_adc / E;
  double sigma_E = (R / 100.0) * E / 2.355;
  return sigma_E * adc_per_keV;
}

void STMDQMJsonProducer::fitPeaks(SpectrumState &s, double dt) {
  for (auto &p : s.peaks) {
    p.mean = p.true_adc;
    p.fitted_sigma = sigmaFromEnergy(p.energy, p.true_adc);
    p.area = 1000;
    p.rate = std::max(1e-6, dt);
  }
}

void STMDQMJsonProducer::fitResolution(SpectrumState &s) {
  s.res_A = 300.0;
  s.res_B = 1.5;
}

void STMDQMJsonProducer::computeCalibration(SpectrumState &s,
                                            double &a, double &b)
{
  double x1 = s.peaks[1].mean;
  double y1 = s.peaks[1].energy;
  double x2 = s.peaks[2].mean;
  double y2 = s.peaks[2].energy;

  a = (y2 - y1) / (x2 - x1);
  b = y1 - a * x1;
}

void STMDQMJsonProducer::buildModel(
  const SpectrumState &s, std::vector<double> &model)
{
  model.assign(SpectrumState::NBINS, 0.0);

  for (int i = 0; i < SpectrumState::NBINS; i++) {
    double val = s.bg_C * std::exp(-i / s.bg_tau);

    for (const auto &p : s.peaks) {
      double arg = (i - p.mean) / p.fitted_sigma;
      val += std::exp(-0.5 * arg * arg);
    }
    model[i] = val;
  }
}

void STMDQMJsonProducer::computeResiduals(
  const SpectrumState &s,
  const std::vector<double> &model,
  std::vector<double> &residuals)
{
  residuals.resize(SpectrumState::NBINS);

  for (int i = 0; i < SpectrumState::NBINS; i++) {
    double m = std::max(1.0, model[i]);
    residuals[i] = (s.hist[i] - m) / std::sqrt(m);
  }
}

void STMDQMJsonProducer::computeChi2(
  SpectrumState &s,
  const std::vector<double> &model)
{
  double chi2 = 0;

  for (int i = 0; i < SpectrumState::NBINS; i++) {
    double m = std::max(1.0, model[i]);
    double d = s.hist[i];
    chi2 += (d - m)*(d - m) / m;
  }

  int ndf = std::max(1, SpectrumState::NBINS - 10);
  s.chi2 = chi2;
  s.chi2_ndf = chi2 / ndf;
}

void STMDQMJsonProducer::computeFFT(
  const std::vector<double> &residuals,
  std::vector<double> &power)
{
  int N = residuals.size();
  int K = N/2;

  power.assign(K, 0.0);

  for (int k = 0; k < K; k++) {
    double re = 0, im = 0;

    for (int n = 0; n < N; n++) {
      double angle = 2.0*M_PI*k*n/N;
      re += residuals[n]*std::cos(angle);
      im -= residuals[n]*std::sin(angle);
    }

    power[k] = re*re + im*im;
  }
}

double STMDQMJsonProducer::computeSlope(
  std::deque<std::pair<double,double>> const& w) const
{
  if (w.size() < 10) return 0.0;

  double sumX=0, sumY=0, sumXY=0, sumX2=0;
  size_t n = w.size();

  for (auto const& [x,y] : w) {
    sumX += x;
    sumY += y;
    sumXY += x*y;
    sumX2 += x*x;
  }

  double denom = n*sumX2 - sumX*sumX;
  if (denom == 0) return 0.0;

  return (n*sumXY - sumX*sumY) / denom;
}

std::string detectorName(int ch) {
  if (ch == 0) return "hpge";
  if (ch == 1) return "labr";
  return "unknown";
}

std::string STMDQMJsonProducer::makeJsonPath(int ch) const {
  return cfg_.jsonBasePath() + "_" + detectorName(ch) + ".json";
}

std::string STMDQMJsonProducer::makeLogPath(int ch, int run) const {
  return cfg_.logBasePath() + "/logs/" +
         detectorName(ch) + "_run_" + std::to_string(run) + ".json";
}

// ============================================================
// ANALYZE
// ============================================================

void STMDQMJsonProducer::analyze(art::Event const& event)
{
  int run = event.run();
  int subrun = event.subRun();

  auto handle =
    event.getValidHandle<std::vector<artdaq::Fragment>>(cfg_.moduleTag());

  for (const auto& frag : *handle) {

    if (frag.type() != artdaq::Fragment::ContainerFragmentType)
      continue;

    artdaq::ContainerFragment cont(frag);

    for (size_t i = 0; i < cont.block_count(); ++i) {

      auto inner = cont.at(i);
      mu2e::STMFragment stm(*inner);

      uint32_t fragId = inner->fragmentID();

      int ch = -1;

      if (stm.isRaw()) {
        ch = stm.channel();
        fragToChannel_[fragId] = ch;
      } else {
        auto it = fragToChannel_.find(fragId);
        if (it == fragToChannel_.end()) continue;
        ch = it->second;
      }

      auto &s = channels_[ch];
      auto &spec = spectra_[ch];
      auto &fitW = fitWindows_[ch];

      // ===== MWD SPECTRUM =====
      if (stm.isMWD()) {
        const int16_t* data = stm.payloadBegin();
        size_t n = stm.payloadWords();

        if (n % 2 != 0) n -= 1;

        for (size_t i = 1; i < n; i += 2) {
          uint16_t height = static_cast<uint16_t>(data[i]);
          if (height < SpectrumState::NBINS)
            spec.hist[height]++;
        }
      }

      if (!stm.isRaw()) continue;

      uint64_t ewt = stm.eventWindowTag();
      uint64_t dtc = stm.dtcClock();
      uint64_t adc = stm.adcClock();

      bool onSpill = (ewt % 2 == 0);

      if (!s.initialized) {
        s.prevEWT = ewt;
        s.prevDTC = dtc;
        s.prevADC = adc;
        s.prevSpill = onSpill;
        s.initialized = true;
        continue;
      }

      if (s.windowEvents == 0) s.firstEWT = ewt;
      s.windowEvents++;
      s.lastEWT = ewt;

      if (onSpill == s.prevSpill) {
        if (ewt != s.prevEWT + 1)
          s.acc_ewt_small++;
      } else {
        s.acc_spill_trans++;
        s.acc_ewt_trans++;
      }

      if (std::llabs((long long)(dtc - s.prevDTC)) > 1000)
        s.acc_dtc_jumps++;

      if (std::llabs((long long)(adc - s.prevADC)) > 1000)
        s.acc_adc_jumps++;

      double dEWT = (double)(ewt - s.prevEWT);
      double dDTC = (double)(dtc - s.prevDTC);

      if (dEWT > 0 && dDTC > 0) {
        fitW.emplace_back(dEWT, dDTC);
        if (fitW.size() > cfg_.fitWindow())
          fitW.pop_front();
      }

      double slope = computeSlope(fitW);

      s.alarm_ewt_error = (s.acc_ewt_small > 20);
      s.alarm_transition_error = (s.acc_spill_trans == 0);
      s.alarm_clock_error =
        (s.acc_dtc_jumps > 15000 || s.acc_adc_jumps > 5000);
      s.alarm_correlation_error =
        (slope < 4000 || slope > 7000);

      bool current_alarm =
        s.alarm_ewt_error ||
        s.alarm_transition_error ||
        s.alarm_clock_error ||
        s.alarm_correlation_error;

      // ===== ALARM LOGGING =====
      if (current_alarm && !s.prev_alarm_state) {
        std::ostringstream alarm;
        alarm << "{";
        alarm << "\"timestamp\":" << std::time(nullptr) << ",";
        alarm << "\"channel\":" << ch << ",";
        alarm << "\"run\":" << run << ",";
        alarm << "\"subrun\":" << subrun << ",";
        alarm << "\"ewt\":" << ewt << ",";
        alarm << "\"on_spill\":" << (onSpill?"true":"false") << ",";
        alarm << "\"alarms\":{";
        alarm << "\"ewt_error\":" << (s.alarm_ewt_error?"true":"false") << ",";
        alarm << "\"transition_error\":" << (s.alarm_transition_error?"true":"false") << ",";
        alarm << "\"clock_error\":" << (s.alarm_clock_error?"true":"false") << ",";
        alarm << "\"correlation_error\":" << (s.alarm_correlation_error?"true":"false");
        alarm << "}}";
        s.alarmBuffer.push_back(alarm.str());
      }

      if (!current_alarm && s.prev_alarm_state) {
        std::ostringstream alarm;
        alarm << "{";
        alarm << "\"timestamp\":" << std::time(nullptr) << ",";
        alarm << "\"channel\":" << ch << ",";
        alarm << "\"run\":" << run << ",";
        alarm << "\"subrun\":" << subrun << ",";
        alarm << "\"ewt\":" << ewt << ",";
        alarm << "\"on_spill\":" << (onSpill?"true":"false") << ",";
        alarm << "\"cleared\":true";
        alarm << "}";
        s.alarmBuffer.push_back(alarm.str());
      }

      s.prev_alarm_state = current_alarm;

      auto now = std::chrono::steady_clock::now();
      double dt =
        std::chrono::duration<double>(now - s.lastWrite).count();

      fitPeaks(spec, dt);
      fitResolution(spec);

      std::vector<double> model, residuals;
      buildModel(spec, model);
      computeResiduals(spec, model, residuals);
      computeChi2(spec, model);
      computeFFT(residuals, spec.fft_power);

      // ===== WRITE JSON + LOG =====
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
            now - s.lastWrite).count() > 500)
      {
        double a,b;
        computeCalibration(spec,a,b);

        std::string path = makeJsonPath(ch);
        std::string tmp = path + ".tmp";

        std::ofstream out(tmp);

        out << "{\n";
        out << "\"channel\":" << ch << ",\n";
        out << "\"run\":" << run << ",\n";
        out << "\"subrun\":" << subrun << ",\n";
        out << "\"timestamp\":" << std::time(nullptr) << ",\n";

        out << "\"alarms\":{\n";
        out << "\"ewt_error\":" << (s.alarm_ewt_error?"true":"false") << ",\n";
        out << "\"transition_error\":" << (s.alarm_transition_error?"true":"false") << ",\n";
        out << "\"clock_error\":" << (s.alarm_clock_error?"true":"false") << ",\n";
        out << "\"correlation_error\":" << (s.alarm_correlation_error?"true":"false") << "\n";
        out << "},\n";

        out << "\"window\":{\n";
        out << "\"events\":" << s.windowEvents << ",\n";
        out << "\"first_ewt\":" << s.firstEWT << ",\n";
        out << "\"last_ewt\":" << s.lastEWT << "\n";
        out << "},\n";

        out << "\"timing\":{\n";
        out << "\"ewt_small_jumps\":" << s.acc_ewt_small << ",\n";
        out << "\"ewt_transition_issues\":" << s.acc_ewt_trans << ",\n";
        out << "\"dtc_jumps\":" << s.acc_dtc_jumps << ",\n";
        out << "\"adc_jumps\":" << s.acc_adc_jumps << ",\n";
        out << "\"spill_transitions\":" << s.acc_spill_trans << "\n";
        out << "},\n";

        out << "\"spill\":{\n";
        out << "\"end\":" << (onSpill?"true":"false") << "\n";
        out << "},\n";

        out << "\"spectrum\":{\n";

        out << "\"calibration\":{\"a\":" << a << ",\"b\":" << b << "},\n";
        out << "\"resolution\":{\"A\":" << spec.res_A << ",\"B\":" << spec.res_B << "},\n";
        out << "\"chi2\":" << spec.chi2 << ",\n";
        out << "\"chi2_ndf\":" << spec.chi2_ndf << ",\n";

        out << "\"histogram\":[";
        for (int i=0;i<SpectrumState::NBINS;i++){
          out << spec.hist[i];
          if(i!=SpectrumState::NBINS-1) out<<",";
        }
        out << "],\n";

        out << "\"residuals\":[";
        for (int i=0;i<SpectrumState::NBINS;i++){
          out << residuals[i];
          if(i!=SpectrumState::NBINS-1) out<<",";
        }
        out << "],\n";

        out << "\"fft_power\":[";
        for (size_t i=0;i<spec.fft_power.size();i++){
          out << spec.fft_power[i];
          if(i!=spec.fft_power.size()-1) out<<",";
        }
        out << "],\n";

        out << "\"peaks\":[";
        for (size_t i=0;i<spec.peaks.size();i++){
          auto &p = spec.peaks[i];
          int bin = std::max(0,std::min((int)p.mean,SpectrumState::NBINS-1));

          out << "{";
          out << "\"name\":\""<<p.name<<"\",";
          out << "\"energy\":"<<p.energy<<",";
          out << "\"mean\":"<<p.mean<<",";
          out << "\"sigma\":"<<p.fitted_sigma<<",";
          out << "\"rate\":"<<p.rate<<",";
          out << "\"height\":"<<spec.hist[bin];
          out << "}";

          if(i!=spec.peaks.size()-1) out<<",";
        }
        out << "]\n";

        out << "}\n";
        out << "}\n";

        out.close();
        std::rename(tmp.c_str(), path.c_str());

        // ===== LOG FILE =====
        std::string logPath = makeLogPath(ch, run);

        if (s.lastRun != run) {
          std::remove(logPath.c_str());
          s.lastRun = run;
        }

        std::vector<std::string> entries;
        std::ifstream in(logPath);

        if (in.good()) {
          std::stringstream buffer;
          buffer << in.rdbuf();
          std::string content = buffer.str();

          size_t pos = 0;
          while ((pos = content.find('{', pos)) != std::string::npos) {

            int depth = 0;
            size_t start = pos;

            for (size_t i = pos; i < content.size(); ++i) {
              if (content[i] == '{') depth++;
              if (content[i] == '}') depth--;

              if (depth == 0) {
                entries.push_back(content.substr(start, i - start + 1));
                pos = i + 1;
                break;
              }
            }
          }
        }

        entries.insert(entries.end(),
                       s.alarmBuffer.begin(),
                       s.alarmBuffer.end());

        std::string tmpLog = logPath + ".tmp";
        std::ofstream log(tmpLog);

        log << "[\n";
        for (size_t i = 0; i < entries.size(); ++i) {
          log << entries[i];
          if (i != entries.size() - 1) log << ",";
          log << "\n";
        }
        log << "]\n";

        log.close();
        std::rename(tmpLog.c_str(), logPath.c_str());

        s.alarmBuffer.clear();

        // RESET WINDOW + ACCUMULATORS
        s.windowEvents = 0;
        s.firstEWT = ewt;
        s.lastEWT = ewt;

        s.acc_ewt_small = 0;
        s.acc_ewt_trans = 0;
        s.acc_dtc_jumps = 0;
        s.acc_adc_jumps = 0;

        s.lastWrite = now;
      }

      s.prevEWT = ewt;
      s.prevDTC = dtc;
      s.prevADC = adc;
      s.prevSpill = onSpill;
    }
  }
}

DEFINE_ART_MODULE(ots::STMDQMJsonProducer)
