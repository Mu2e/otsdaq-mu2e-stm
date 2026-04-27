#ifndef OTS_STMDQMJSONPRODUCER_HH
#define OTS_STMDQMJSONPRODUCER_HH

#include "art/Framework/Core/EDAnalyzer.h"

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Comment.h"

#include <unordered_map>
#include <vector>
#include <deque>
#include <string>
#include <chrono>

namespace ots {

class STMDQMJsonProducer : public art::EDAnalyzer {
public:
  struct Config {
    fhicl::Atom<std::string> moduleTag { fhicl::Name("moduleTag") };
    fhicl::Atom<std::string> jsonBasePath { fhicl::Name("jsonBasePath") };
    fhicl::Atom<std::string> logBasePath { fhicl::Name("logBasePath") };

    fhicl::Atom<size_t> fitWindow {
      fhicl::Name("fitWindow"),
      fhicl::Comment("Sliding window size"),
      200
    };
  };

  // ================= SPECTRUM =================
  struct SpectrumState {
    static const int NBINS = 2048;
    std::vector<int> hist;

    struct Peak {
      std::string name;
      double energy;
      double true_adc;
      double sigma;

      double mean = 0;
      double fitted_sigma = 0;
      double area = 0;
      double rate = 0;
    };

    std::vector<Peak> peaks;

    double res_A = 0;
    double res_B = 0;
    double bg_C = 1.0;
    double bg_tau = 600.0;
    double chi2 = 0;
    double chi2_ndf = 0;

    std::vector<double> fft_power;

    SpectrumState();
  };

  using Parameters = art::EDAnalyzer::Table<Config>;
  explicit STMDQMJsonProducer(Parameters const& p);

  void analyze(art::Event const& e) override;

private:
  Config cfg_;

  // ================= CHANNEL =================
  struct ChannelState {
    uint64_t ewt = 0;
    uint64_t adc = 0;
    uint64_t dtc = 0;

    uint64_t prevEWT = 0;
    uint64_t prevDTC = 0;
    uint64_t prevADC = 0;

    bool onSpill = true;
    bool prevSpill = true;

    int windowEvents = 0;
    uint64_t firstEWT = 0;
    uint64_t lastEWT = 0;

    int acc_ewt_small = 0;
    int acc_ewt_trans = 0;
    int acc_spill_trans = 0;
    int acc_dtc_jumps = 0;
    int acc_adc_jumps = 0;

    bool alarm_ewt_error = false;
    bool alarm_transition_error = false;
    bool alarm_clock_error = false;
    bool alarm_correlation_error = false;
    bool prev_alarm_state = false;

    std::vector<std::string> alarmBuffer;

    std::chrono::steady_clock::time_point lastWrite =
      std::chrono::steady_clock::now();

    int lastRun = -1;
    bool initialized = false;
  };

  // ================= STORAGE =================
  std::unordered_map<int, ChannelState> channels_;
  std::unordered_map<int, SpectrumState> spectra_;
  std::unordered_map<int, std::deque<std::pair<double,double>>> fitWindows_;
  std::unordered_map<uint32_t, int> fragToChannel_;

  // ================= HELPERS =================
  double sigmaFromEnergy(double E, double true_adc);
  void fitPeaks(SpectrumState &s, double dt);
  void fitResolution(SpectrumState &s);
  void computeCalibration(SpectrumState &s, double &a, double &b);
  void buildModel(const SpectrumState &s, std::vector<double> &model);
  void computeResiduals(const SpectrumState &s,
                        const std::vector<double> &model,
                        std::vector<double> &residuals);
  void computeChi2(SpectrumState &s,
                   const std::vector<double> &model);
  void computeFFT(const std::vector<double> &residuals,
                  std::vector<double> &power);

  double computeSlope(std::deque<std::pair<double,double>> const&) const;

  std::string makeJsonPath(int ch) const;
  std::string makeLogPath(int ch, int run) const;
};

} // namespace ots

#endif
