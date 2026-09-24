#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <iomanip>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TF1.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include "TLegend.h"
#include "TRandom3.h"
#include "TLine.h"


struct SumTF1 {

  SumTF1(const std::vector<TF1 *> & flist) : fFuncList(flist) {}

  double operator() (const double * x, const double *p) {
    double result = 0;
    for (unsigned int i = 0; i < fFuncList.size(); ++i)
      result += fFuncList[i]->EvalPar(x,p);
    return result;
  }

  std::vector<TF1*> fFuncList;

};



double Bremsfunc(double* t, double* p){
  double A = p[0];
  double B = p[1];
  double C = p[2];
  double D = p[3];
  double f = A/(exp(B*t[0])+C)+D;
  return f;
};

void corset( const TMatrixD& V, TMatrixD& C ){
  // calculate sqrt(V) as lower diagonal matrix
  // if we dont know the number of params
  //int NP = V.GetNcols();
  // if we know number of params
  int NP = 4;
  for( int i = 0; i < NP; ++i ) {
    for( int j = 0; j < NP; ++j ) {
      C[i][j] = 0;
    }
  }
  for( int j = 0; j < NP; ++j ) {
    // diagonal terms first
    double Ck = 0;
    for( int k = 0; k < j; ++k ) {
      Ck += C[j][k] * C[j][k];
    } // k
    C[j][j] = sqrt( fabs( V[j][j] - Ck ) );
    // off-diagonal terms
    for( int i = j+1; i < NP; ++i ) {
      Ck = 0;
      for( int k = 0; k < j; ++k ) {
	Ck += C[i][k] * C[j][k];
      } //k
      C[i][j] = ( V[i][j] - Ck ) / C[j][j];
    }// i
  } // j
}

void corgen( const TMatrixD& C, double *x){
  //gRandom->SetSeed(0);
  std::cout<<"seed: "<<gRandom->GetSeed()<<std::endl;
  int NP = 4;
  double *z =new double[NP];
  // np random numbers from unit Gaussian
  for( int i = 0; i < NP; ++i )
    z[i] = gRandom->Gaus( 0.0, 1.0 );
  // fill values
  for( int i = 0; i < NP; ++i ) {
    x[i] = 0;
    for( int j = 0; j <= i; ++j ) {
      x[i] += C[i][j] * z[j];
    } // j
  } // i
  delete [] z; //free the array
}

//Brems rate: kHz, TimeSim: s, mean and sigma: MeV
double Signal_over_Background(double Bremsrate_kHz, double TimeSim, double meanE, double sigma, double sigmacut){

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //This seed is for fill random
  //double seed=1;
  //gRandom->SetSeed(seed);

  gRandom->SetSeed (0);

  std::cout<<"seed: "<<gRandom->GetSeed()<<std::endl;

  int dimpeaks;
  if (meanE==0){dimpeaks=4;}
  else {dimpeaks=1;}

  //****************************
  //-----SET THESE VARIABLES----
  //****************************
  bool debugout = true;
  bool debugout2 = false;
  bool debugoutlog = true;
  bool drawplot = true;
  bool draw_just_signal = true;
  bool doscale = false;
  //Plot range in x
  //double xx1[2]={0.05,0.5};
  double xx1[2]={0.05,1.9};
  //Function range
  double frange[2] = {0.04,2};
  double photopeak_range[dimpeaks][2];
  double mean_E[dimpeaks];
  if (meanE==0){
    mean_E[0]=0.066;
    mean_E[1]=0.347;
    mean_E[2]=0.844;
    mean_E[3]=1.809;
  }
  else {
    mean_E[0]=meanE;
  }
  //Best fit parameters for Bremsstrahlung
  double p0 = 0.00157448;
  double p1 = 2.37638;
  double p2 = -0.975819;
  double p3 = 0.000122886;
  //Number of fit parameters
  int NP = 4;
  //Number of histo bins
  int nbins = 20000;
  double binning = (frange[1]-frange[0])/nbins;
  double fitbrems_binning = 0.00196;
  //Number of fits to sample from covariance matrix
  int numfits = 100;
  //Assume these parameters
  double prot_per_maininjectorcycle = 8e12;
  double maininjectortime = 1.4;
  double STM_accept = 8.03e-10;
  double stoppedmuons_proton = 0.0016;
  double percentage_66keV = 0.62;
  double percentage_347keV = 0.8;
  double percentage_844keV = 0.05704;
  double percentage_1809keV = 0.3162;
  //*************************************

  //Rate Hz
  double rateHz = Bremsrate_kHz*1000;

  //Number of bremsstrahlung photons in the sample
  double pulseNumBrems = rateHz*TimeSim;


  double pulseNumXrays[dimpeaks];

  double rate_prot_s = prot_per_maininjectorcycle/maininjectortime;
  double POT = rate_prot_s*TimeSim;
  double scalePOT = 1./POT;
  double rate_stoppedmuons_s = rate_prot_s*stoppedmuons_proton;
  double rate_66keV_s = rate_stoppedmuons_s*percentage_66keV;
  double rate_347keV_s = rate_stoppedmuons_s*percentage_347keV;
  double rate_844keV_s = rate_stoppedmuons_s*percentage_844keV;
  double rate_1809keV_s = rate_stoppedmuons_s*percentage_1809keV;
  //Rate Hz from X-rays at the STM
  double rate_66keV_s_STM = rate_66keV_s*STM_accept;
  double rate_347keV_s_STM = rate_347keV_s*STM_accept;
  double rate_844keV_s_STM = rate_844keV_s*STM_accept;
  double rate_1809keV_s_STM = rate_1809keV_s*STM_accept;

  if(drawplot==true){TCanvas* c1 = new TCanvas("");}

  if(debugout==true){
    std::cout<<"Theoretical pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;
    std::cout<<"--Sample number of pulses with a Poisson distribution---"<<std::endl;
  }
  pulseNumBrems = gRandom->Poisson(pulseNumBrems);
  if(debugout==true){std::cout<<"Number of pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;}

  //Bremsstrahlung
  TF1* fbrems = new TF1("fbrems",Bremsfunc,frange[0],frange[1],NP);
  fbrems->SetNpx(300000);
  //Get the integral for binning chosen in fitting the bremsstrahlung background (fitbrems_binning = 0.00196)
  double fitpar_check[4] = {p0, p1, p2,p3};
  fbrems->SetParameters(fitpar_check[0],fitpar_check[1],fitpar_check[2],fitpar_check[3]);
  double bckbef_function_fullrange = fbrems->Integral(frange[0],frange[1]);
  std::cout<<"Integral of brems background for 1 brems pulse: "<<bckbef_function_fullrange<<std::endl;
  //Correct by binning if new binning is chosen and redefine height for the brems function
  double height_brems = (binning/fitbrems_binning)*pulseNumBrems*p0;
  double height_brems2 = (binning/fitbrems_binning)*pulseNumBrems*p3;

  //scale
  if(doscale==true){
    height_brems = height_brems*scalePOT;
    height_brems2 = height_brems2*scalePOT;
  }

  double fitpar[4] = {height_brems, p1, p2,height_brems2};
  fbrems->SetParameters(fitpar[0],fitpar[1],fitpar[2],fitpar[3]);
  double bck = fbrems->Integral(frange[0],frange[1]);
  if(debugout==true){std::cout<<"Integral for brems photons: "<<bck<<std::endl;}


  if(debugout==true){
    std::cout<<"Simulation time: "<<TimeSim<<" s, Total POT: "<<POT<<std::endl;
    std::cout<<"Mean energy: "<<meanE<<" MeV"<<std::endl;
    std::cout<<"Detector resolution: "<<sigma<<" MeV"<<std::endl;
    std::cout<<"Bremsstrahlung Rate: "<<rateHz<<" Hz"<<std::endl;
    std::cout<<"Bremsstrahlung pulses: "<<pulseNumBrems<<std::endl;

    std::cout<<"Protons per main injector cycle: "<<prot_per_maininjectorcycle<<std::endl;
    //std::cout<<""<<std::endl;
    std::cout<<"STM acceptance: "<<STM_accept<<" prot per s: "<<rate_prot_s<<" stopped muons per second: "<<rate_stoppedmuons_s<<std::endl;
    //std::cout<<""<<std::endl;
    std::cout<<"rate_66keV_s: "<<rate_66keV_s<<" rate_347keV_s: "<<rate_347keV_s<<" rate_844keV_s: "<<rate_844keV_s<<" rate_1809keV_s: "<<rate_1809keV_s<<std::endl;
    //std::cout<<""<<std::endl;
    std::cout<<"rate_66keV_s_STM: "<<rate_66keV_s_STM<<" rate_347keV_s_STM: "<<rate_347keV_s_STM<<" rate_844keV_s_STM: "<<rate_844keV_s_STM<<" rate_1809keV_s_STM: "<<rate_1809keV_s_STM<<std::endl;
  }

  if(meanE==0.066){pulseNumXrays[0] = rate_66keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"66keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
  }
  else if(meanE==0.347){pulseNumXrays[0] = rate_347keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"347keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
  }
  else if(meanE==0.844){pulseNumXrays[0] = rate_844keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"844keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
  }
  else if(meanE==1.809){pulseNumXrays[0] = rate_1809keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"1809keV XRay Rate in theory: "<<pulseNumXrays[0]<<" pulses"<<std::endl;}
  }
  else if(meanE==0){
    pulseNumXrays[0]=rate_66keV_s_STM*TimeSim;
    pulseNumXrays[1]=rate_347keV_s_STM*TimeSim;
    pulseNumXrays[2]=rate_844keV_s_STM*TimeSim;
    pulseNumXrays[3]=rate_1809keV_s_STM*TimeSim;
  }
  else{std::cout<<"XRay energy not valid"<<std::endl; exit(0);}

  //Vector with signal + Bremsstrahlung
  std::vector<TF1 *> v;
  //push back bremsstrahlung
  v.push_back(fbrems);

  TF1* fsignal[dimpeaks];
  double integral_gaus;
  double s[dimpeaks];
  double sum_integral[dimpeaks];

  for(int i =0; i < dimpeaks; i++){
    if(debugout==true){
      std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays[i]<<std::endl;
    }
    pulseNumXrays[i] = gRandom->Poisson(pulseNumXrays[i]);
    //Number of pulses in X-Ray peak
    if(debugout==true){
      std::cout<<"--Sample number of pulses with a Poisson distribution---"<<"MeanE: "<<mean_E[i]<<std::endl;
      std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays[i]<<std::endl;

      double global_counts_POT_ST = pulseNumXrays[i]/(STM_accept*POT);
      double global_counts_POT_STM = pulseNumXrays[i]/(POT);
      std::cout<<"CHECK ST XRAYs WITHOUT ACCEPTANCE AND RESOLUTION EFFECT: "<<global_counts_POT_ST<<" counts/POT at ST"<<std::endl;
      std::cout<<"CHECK STM XRAYs WITH ACCEPTANCE AND WITHOUT RESOLUTION EFFECT: "<<global_counts_POT_STM<<" counts/POT at STM"<<std::endl;
    }

    //Generate the signal as a Gaussian
    fsignal[i] = new TF1("fsignal","gaus",frange[0],frange[1]);
    fsignal[i]->SetNpx(300000);
    double height_gaus = pulseNumXrays[i]*binning/(sigma*sqrt(3.14159*2));
    //scale
    if(doscale==true){
      height_gaus = height_gaus*scalePOT;
    }
    fsignal[i]->SetParameters(height_gaus,mean_E[i],sigma);
    s[i] = fsignal[i]->Integral(frange[0],frange[1]);
    if(debugout==true){std::cout<<"Integral for gaus-signal: "<<s[i]<<std::endl;}

    //push back signal
    v.push_back(fsignal[i]);
  }


  TF1 * fsum = new TF1("fsum",SumTF1(v),frange[0],frange[1],0);
  fsum->SetNpx(300000);

  //Check with best fit
  for(int i =0; i < dimpeaks; i++){

    photopeak_range[i][0] = mean_E[i]-sigmacut*sigma;
    photopeak_range[i][1] = mean_E[i]+sigmacut*sigma;

    sum_integral[i] = fsum->Integral(photopeak_range[i][0],photopeak_range[i][1]);
    double bestfit_integral = fbrems->Integral(photopeak_range[i][0],photopeak_range[i][1]);
    double counts_photopeak_bestfit = sum_integral[i] - bestfit_integral;
    if(debugout==true){std::cout<<"Real number of counts in photopeak, BEST FIT: "<<counts_photopeak_bestfit<<std::endl;}

    int bestfit = int(counts_photopeak_bestfit);
    int realcounts = int(s[i]);
    //with a margin of +-1 count
    if((bestfit!=realcounts)&&((bestfit+1)!=realcounts)&&((bestfit-1)!=realcounts)){
      std::cout<<"sigmacut has to be bigger in order to recover the original number of counts in the photopeak with the best fit..."<<std::endl; exit(0);}
  }
  //****************************
  //-----PLOTTING---------------
  //****************************
  double ymax, ymax1, ymax2;
  if(meanE==0){
    ymax1 = fsum->GetHistogram()->GetMaximum();
    ymax2 = fsum->Eval(xx1[0]);}
  else{
    ymax1 = fsum->GetHistogram()->GetMaximum();
    ymax2 = fsum->Eval(xx1[0]);}
  if(ymax2>ymax1){ymax=ymax2;}
  else{ymax=ymax1;}
  double yy1[2]={0,ymax};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("E_{#gamma} (STM) [MeV]");
  string yaxis_str;
  char*  yaxis_char;
  if(doscale==true){
    yaxis_str = "Counts / POT";
    yaxis_char = const_cast<char*>(yaxis_str.c_str());
  }
  else{
    yaxis_str = "Counts/run time "+std::to_string(int(TimeSim))+" s";
    yaxis_char = const_cast<char*>(yaxis_str.c_str());
  }
  graph1->GetYaxis()->SetTitle(yaxis_char);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);

  std::string str_latex = "~ 6#times10^{12} POT/s, 10^{10} stopped #mu/s";
  char* char_latex = const_cast<char*>(str_latex.c_str());
  TLatex latex;
  if(drawplot==true){graph1->Draw("ap"); latex.DrawLatexNDC(.35,.85,char_latex);}

  for(int i =0; i < dimpeaks; i++){
    fsignal[i]->SetFillColor(kCyan-3);
    fsignal[i]->SetLineColor(kCyan-3);
    fsignal[i]->SetFillStyle(3001);
  }

  fsum->SetFillColor(kOrange-3);
  fsum->SetLineColor(kOrange-3);
  fsum->SetFillStyle(3001);

  fbrems->SetLineColor(kRed);
  fbrems->SetLineWidth(1);
  fbrems->SetLineStyle(1);

  //Compute different fits
  //Covariance matrix
  TMatrixD V(4,4);
  V[0][0] = 1.976e-09;
  V[0][1] = 2.09e-06;
  V[0][2] = 8.283e-08;
  V[0][3] = 1.143e-10;
  V[1][0] = 2.09e-06;
  V[1][1] = 0.002278;
  V[1][2] = 8.213e-05;
  V[1][3] = 1.305e-07;
  V[2][0] = 8.283e-08;
  V[2][1] = 8.213e-05;
  V[2][2] = 4.44e-06;
  V[2][3] = 4.071e-09;
  V[3][0] = 1.143e-10;
  V[3][1] = 1.305e-07;
  V[3][2] = 4.071e-09;
  V[3][3] = 9.624e-12;

  TMatrixD sqrtCov( NP, NP );
  corset( V, sqrtCov );

  //Fit functions
  TF1* fx;
  //sigmaE histogram set a high range for all the counts
  double xup_histo = pulseNumXrays[0]*10000;
  TH1F* hsigmaE = new TH1F("hsigmaE","",1000,0,xup_histo);
  double sigma_counts, mean_counts, accuracy =0;

  if(meanE!=0){
    for (int loop = 0; loop < numfits; ++loop){
      double *values = new double[NP];
      corgen (sqrtCov, values);
      if(debugout2==true){std::cout<<values[0]<<" "<<values[1]<<" "<<values[2]<<" "<<values[3]<<std::endl;}
      //--sum the mean to the parameters
      for (int i=0;i<NP;i++) {
	values[i]+=fitpar[i];
      }

      ///Plot all the new fits
      fx = new TF1("fx",Bremsfunc,frange[0],frange[1],NP);

      fx->SetNpx(300000);
      fx->SetLineColor(kCyan-3);
      fx->FixParameter(0, values[0]);
      fx->FixParameter(1, values[1]);
      fx->FixParameter(2, values[2]);
      fx->FixParameter(3, values[3]);
      double fit_integral = fx->Integral(photopeak_range[0][0],photopeak_range[0][1]);
      double counts_photopeak_newfit = sum_integral[0] - fit_integral;
      if(debugout2==true){
	std::cout<<"Cov "<<loop<<": "<<values[0]<<" "<<values[1]<<" "<<values[2]<<" "<<values[3]<<std::endl;
	std::cout<<"Real number of counts in photopeak: "<<counts_photopeak_newfit<<std::endl;}
      //Fill histogram with counts in the photopeak
      hsigmaE->Fill(counts_photopeak_newfit);

      //Draw fits
      if((drawplot==true)&&(draw_just_signal==false)){fx->Draw("same");}
    }

    sigma_counts = hsigmaE->GetRMS();
    mean_counts = hsigmaE->GetMean();
    accuracy = sigma_counts/mean_counts;
    std::cout<<"RESULT: sigma_counts: "<<sigma_counts<<" mean_counts: "<<mean_counts<<" accuracy: "<<accuracy<<std::endl;
  }//if meanE


  double sigmakeV = sigma*1000;
  std::stringstream sigmastream;
  sigmastream << std::fixed << std::setprecision(1) << sigmakeV;

  //Draw functions
  if(drawplot==true){
    if(draw_just_signal==true){
      fsum->Draw("same,LF2");
      for(int i =0; i < dimpeaks; i++){
        fsignal[i]->Draw("same,LF2");
      }
      string Signal_sigma1_str = "#splitline{{}^{27}Al Energy Spectrum}{#sigma_{HPGe}= "+sigmastream.str()+" keV}";
      char*  Signal_sigma1_char = const_cast<char*>(Signal_sigma1_str.c_str());

      string Bremsrate1_kHz_str = "#splitline{Signal + }{\n"+std::to_string(int(Bremsrate_kHz))+" kHz Bremsstrahlung}";
      char*  Bremsrate1_kHz_char = const_cast<char*>(Bremsrate1_kHz_str.c_str());

      auto legend = new TLegend(0.35,0.6,0.7,0.8);
      legend->AddEntry(fsignal[0],Signal_sigma1_char,"f");
      legend->AddEntry(fsum,Bremsrate1_kHz_char,"f");
      legend->Draw("same");
    }
    else{
      fsum->Draw("same,LF2");
      fbrems->Draw("same,l");
      //Draw Tlines of sigma cuts
      TLine *line1=new TLine(meanE-sigmacut*sigma,0,meanE-sigmacut*sigma,ymax);
      line1->SetLineColor(kBlack);
      line1->SetLineStyle(2);
      line1->SetLineWidth(2);
      line1->Draw("same,l");

      TLine *line2=new TLine(meanE+sigmacut*sigma,0,meanE+sigmacut*sigma,ymax);
      line2->SetLineColor(kBlack);
      line2->SetLineStyle(2);
      line2->SetLineWidth(2);
      line2->Draw("same,l");

      double mean_keV = meanE*1000;

      string Bremsrate_kHz_str = "#splitline{"+std::to_string(int(mean_keV))+" keV peak +}{\n"+std::to_string(int(Bremsrate_kHz))+" kHz Bremsstrahlung}";
      char*  Bremsrate_kHz_char = const_cast<char*>(Bremsrate_kHz_str.c_str());

      string Signal_sigma_str = "Signal Region (#sigma="+sigmastream.str()+" keV)";
      char*  Signal_sigma_char = const_cast<char*>(Signal_sigma_str.c_str());

      auto legend = new TLegend(0.3,0.5,0.65,0.9);
      legend->AddEntry(fsum,Bremsrate_kHz_char,"f");
      legend->AddEntry(fx,"#splitline{\nEstimated Bremsstrahlung}{Background}","l");
      legend->AddEntry(fbrems,"Best Background Fit","l");
      legend->AddEntry(line1,Signal_sigma_char,"l");
      legend->Draw("same");

      gPad->RedrawAxis();
      string namepng = "SignalBackground_"+std::to_string(int(Bremsrate_kHz))+"kHz_sigma"+sigmastream.str()+"keV_runtime"+std::to_string(int(TimeSim))+"s.png";
      string namepdf = "SignalBackground_"+std::to_string(int(Bremsrate_kHz))+"kHz_sigma"+sigmastream.str()+"keV_runtime"+std::to_string(int(TimeSim))+"s.pdf";
      char* imagenamepng = const_cast<char*>(namepng.c_str());
      char* imagenamepdf = const_cast<char*>(namepdf.c_str());
      //c1->Print(imagenamepng);
      //c1->Print(imagenamepdf);
    }//else
  }//if drawplot

  if(debugoutlog==true){
    std::cout<<"Rate(kHz): "<<Bremsrate_kHz<<std::endl;
    std::cout<<"Time(s): "<<TimeSim<<std::endl;
    std::cout<<"Resol(MeV): "<<sigma<<std::endl;
    std::cout<<"sigmacut: "<<sigmacut<<std::endl;
    std::cout<<"Energy: "<<meanE<<std::endl;
    std::cout<<"mean(counts): "<<mean_counts<<std::endl;
    std::cout<<"RMSsignal(counts): "<<sigma_counts<<std::endl;
    std::cout<<"UncertaintyRMS/meansignal: "<<accuracy<<std::endl;
    std::cout<<"Real mean Xrays simulated in the peak: "<<pulseNumXrays[0]<<std::endl;
  }


  hsigmaE->Reset();
  //Delete objects
  if(drawplot==false){
    delete gROOT->FindObject("c1");
    delete gROOT->FindObject("fbrems");
    delete gROOT->FindObject("fsignal");
    delete gROOT->FindObject("fsum");
    delete gROOT->FindObject("graph1");
    delete gROOT->FindObject("hsigmaE");
    delete gROOT->FindObject("fx");
    delete gROOT->FindObject("line1");
    delete gROOT->FindObject("line2");
    delete gROOT->FindObject("legend");
  }
  return accuracy;
}

void BremsstrahlungCov(double Bremsrate_kHz, double TimeSim, double meanE, double sigma, double sigmacut){

if(TimeSim==0){
  TimeSim = 10;
  double accuracy = 10;
  double acc_limit = 0.01;
  while(accuracy > acc_limit){
  accuracy = Signal_over_Background(Bremsrate_kHz,TimeSim,meanE,sigma,sigmacut);
  std::cout<<"Check accuracy: "<<accuracy<<" time: "<<TimeSim<<std::endl;
  std::cout<<"-----------------------------------------------"<<std::endl;
  TimeSim = TimeSim+1000;

 }
}
else{
  //double seed = 1;
  double accuracy = Signal_over_Background(Bremsrate_kHz,TimeSim,meanE,sigma,sigmacut);
  std::cout<<"Check accuracy: "<<accuracy<<std::endl;
  }
}
