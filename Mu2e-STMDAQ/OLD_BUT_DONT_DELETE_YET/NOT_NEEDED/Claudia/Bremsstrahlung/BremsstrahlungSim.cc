#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <iomanip>
#include <boost/chrono.hpp>
#include <chrono>

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

#include "TApplication.h"
#include "TRootCanvas.h"

//brems rate [kHz], simulation time [s], nbins, mean gaussian [MeV], MeV
std::vector<double> BremsstrahlungSim(int argc, char *argv[], double Bremsrate_kHz, double TimeSim, int nbins, double mean, double sigma, double sigmacut, double ymax, unsigned long int seed, double useweight){
  
#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  
  delete gROOT->FindObject("c");  
  delete gROOT->FindObject("hXray");
  delete gROOT->FindObject("hBrems");
  delete gROOT->FindObject("hcBrems");
  delete gROOT->FindObject("d");
  
  //If this is not commented - new seed everytime we run this, so different events for different runs, different seed sequence for each run
  //Default constructor If seed is 0, the seed is automatically computed via a TUUID object
  //gRandom = new TRandom(0);
  //If this is uncommented - same seed everytime we call the function - generates same events for same run
  //gRandom = new TRandom3();
  //gRandom = new TRandom(2); //all the events look the same with the same seed
  //If everything is commented ie default, generate same seed sequence every time we run it (generates different events for same run but if we run it twice it generates the same seed sequence)

  //This seed is for fill random
  gRandom->SetSeed(seed);  
  
  bool debugout = false;
  bool ART_geom = true;
  bool write_weights_file=true;
  
  std::fstream readfile;
  std::fstream writefile;
  std::string input_path = "Histogram_Weights_1kHz_100000s.txt";
  std::string output_path = "Histogram_Weights_1kHz_100000s.txt";
 


  gROOT->SetStyle("ATLAS");
  TCanvas* canvas = new TCanvas("c");
  
  double xmin_histo = 0;
  double xmax_histo = 2;
  double binning = (xmax_histo-xmin_histo)/nbins;

  //Rate
  double rateHz = Bremsrate_kHz*1000;
  //Rate in MHz
  double rate = rateHz * 1e-6;

  //Number of bremsstrahlung photons in the sample 
  unsigned long int pulseNumBrems = rateHz*TimeSim;

  //Model for bremsstrahlung
  double value_0 = 3.043 * 1e4;
  double value_1 = -6.85;
  double value_2 = 594.6;
  if(debugout==true){
    std::cout<<"Bremsstrahlung function: value_0: "<< value_0 << ", value_1: " << value_1 <<", value_2: "<< value_2 <<std::endl;
  }
  TF1 *expBrems = new TF1("expBrems","[0]*exp([1]*x)+[2]",xmin_histo,xmax_histo);
  expBrems->SetParameter(0,value_0);
  expBrems->SetParameter(1,value_1);
  expBrems->SetParameter(2,value_2);
 

  if(debugout==true){
    std::cout<<"Theoretical pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;
    std::cout<<"--Sample number of pulses with a Poisson distribution---"<<std::endl;}

  pulseNumBrems = gRandom->Poisson(pulseNumBrems);

  //Number of pulses in X-Ray peak
  if(debugout==true){std::cout<<"Number of pulses in Bremsstrahlung background: "<<pulseNumBrems<<std::endl;}


  TH1F *hBrems = new TH1F("hBrems", "", nbins,xmin_histo,xmax_histo);  
 
  double pulseNumBrems_aux, weight, weight_aux;
  //number of pulses for a rate of 1kHz during 100,000s bremsstrahlung
  pulseNumBrems_aux = 1000*100000;

  if(useweight==0){
    hBrems->FillRandom("expBrems",pulseNumBrems);
  }
  
  //Read the file with the weights
  else if(useweight==1){
    readfile.open(input_path,std::ios::in);
    if(debugout==true){std::cout<<"Reading input file with weights: "<<input_path<<std::endl;}
    //Read the file with pulseNumBrems_aux and all the weight_aux
    readfile>>pulseNumBrems_aux;
    //Get the bins and the content for the bin
    for(int i=0; i<nbins;i++){
      readfile>>weight_aux;
      weight = pulseNumBrems * weight_aux / pulseNumBrems_aux;
      //bin number and weight to increment the bin height
      hBrems->SetBinContent(i,weight);
      
    }
    readfile.close();
  } 
   //Generate the auxiliary histogram everytime to get the weights and write weights to a txt file
  else {
    if(write_weights_file==true){
      writefile.open(output_path,std::ios::out);
      writefile<<pulseNumBrems_aux<<std::endl;
    }
    //fill the histogram for 1kHz
    hBrems->FillRandom("expBrems",pulseNumBrems_aux);
    //Get the bins and the content for the bin
    for(int i=0; i<nbins;i++){
      weight_aux = hBrems->GetBinContent(i);
      weight = pulseNumBrems * weight_aux / pulseNumBrems_aux;
      if(write_weights_file==true){
	writefile<<weight_aux<<std::endl;
	  }
      //bin number and weight to increment the bin height
      hBrems->SetBinContent(i,weight);
    }
    if(debugout==true){std::cout<<"File generated with pulseNumBrems_aux (number of pulses used to fill the histogram) and "<<nbins<<" weights: "<<output_path<<std::endl;}
    writefile.close();
  }  
 
    
  
  double POT_microbunch = 30000000;
  double microbunch_spill = 32000;
  double spills_maininjector = 8;
  double maininjectortime = 1.4;
  double stoppedmuons_proton = 0.0016;
  double percentage_66keV = 0.62;
  double percentage_347keV = 0.8;
  double percentage_844keV = 0.05704;
  double percentage_1809keV = 0.3162;
  double r_STM = 45;
  double z_STM = 40750;
  double z_ST = 5800;
  double r = z_STM-z_ST;
  double STM_accept,prot_per_maininjectorcycle;

  if(ART_geom == false){
    prot_per_maininjectorcycle = spills_maininjector*microbunch_spill*POT_microbunch;
    STM_accept = (2*r_STM*r_STM)/(4*r*r);
  }
  else{
    prot_per_maininjectorcycle = 8e12;
    STM_accept = 8.03e-10;
  }

  double rate_prot_s = prot_per_maininjectorcycle/maininjectortime;
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

  unsigned long int pulseNumXrays=0;
  unsigned long int pulseNumXrays66=0, pulseNumXrays347=0, pulseNumXrays844=0, pulseNumXrays1809=0;

  
  if(debugout==true){
    std::cout<<"Simulation time: "<<TimeSim<<" s"<<std::endl;
    std::cout<<"Mean energy: "<<mean<<" MeV"<<std::endl;
    std::cout<<"Detector resolution: "<<sigma<<" MeV"<<std::endl;
    std::cout<<"Bremsstrahlung Rate: "<<rateHz<<" Hz"<<std::endl;
    std::cout<<"Bremsstrahlung pulses: "<<pulseNumBrems<<std::endl;
    std::cout<<"Histogram from: "<<xmin_histo<<" to: "<<xmax_histo<<" MeV, nbins: "<<nbins<<", binning: "<<binning<<" MeV"<<std::endl;

    std::cout<<"Protons per main injector cycle: "<<prot_per_maininjectorcycle<<std::endl; 
    //std::cout<<""<<std::endl;
    std::cout<<"STM acceptance: "<<STM_accept<<" prot per s: "<<rate_prot_s<<" stopped muons per second: "<<rate_stoppedmuons_s<<std::endl;
    //std::cout<<""<<std::endl;
    std::cout<<"rate_66keV_s: "<<rate_66keV_s<<" rate_347keV_s: "<<rate_347keV_s<<" rate_844keV_s: "<<rate_844keV_s<<" rate_1809keV_s: "<<rate_1809keV_s<<std::endl;
    //std::cout<<""<<std::endl;
    std::cout<<"rate_66keV_s_STM: "<<rate_66keV_s_STM<<" rate_347keV_s_STM: "<<rate_347keV_s_STM<<" rate_844keV_s_STM: "<<rate_844keV_s_STM<<" rate_1809keV_s_STM: "<<rate_1809keV_s_STM<<std::endl;
  }



  
  if(mean==0.066){pulseNumXrays = rate_66keV_s_STM*TimeSim; 
    if(debugout==true){std::cout<<"66keV XRay Rate in theory: "<<pulseNumXrays<<" pulses"<<std::endl;}
  }
  else if(mean==0.347){pulseNumXrays = rate_347keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"347keV XRay Rate in theory: "<<pulseNumXrays<<" pulses"<<std::endl;}
  }
  else if(mean==0.844){pulseNumXrays = rate_844keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"844keV XRay Rate in theory: "<<pulseNumXrays<<" pulses"<<std::endl;}
  }
  else if(mean==0.1809){pulseNumXrays = rate_1809keV_s_STM*TimeSim;
    if(debugout==true){std::cout<<"1809keV XRay Rate in theory: "<<pulseNumXrays<<" pulses"<<std::endl;}
  }
  else if(mean==0){
    pulseNumXrays66=rate_66keV_s_STM*TimeSim;
    pulseNumXrays347=rate_347keV_s_STM*TimeSim;
    pulseNumXrays844=rate_844keV_s_STM*TimeSim;
    pulseNumXrays1809=rate_1809keV_s_STM*TimeSim;
  }
  else{std::cout<<"XRay energy not valid"<<std::endl; exit(0);}

  //Model for Xray lines
  TH1F *hXray66, *hXray347, *hXray844, *hXray1809;
  TF1 *gausXray,  *gausXray66, *gausXray347, *gausXray844, *gausXray1809;

  if(debugout==true){std::cout<<"--Sample number of pulses with a Poisson distribution---"<<std::endl;}

  pulseNumXrays = gRandom->Poisson(pulseNumXrays); 

  //Number of pulses in X-Ray peak
  if(debugout==true){std::cout<<"Number of pulses in X-Ray peak: "<<pulseNumXrays<<std::endl;}

  TH1F *hXray = new TH1F("hXray", "", nbins,xmin_histo,xmax_histo);

  if(mean!=0){
    gausXray= new TF1("gausXray","gaus",xmin_histo,xmax_histo);
    gausXray->SetParameters(1,mean,sigma);

    hXray->FillRandom("gausXray",pulseNumXrays);
  }

  if(mean==0){
    gausXray66= new TF1("gausXray66","gaus",xmin_histo,xmax_histo);
    gausXray66->SetParameters(1,0.066,sigma);
    gausXray347= new TF1("gausXray347","gaus",xmin_histo,xmax_histo);
    gausXray347->SetParameters(1,0.347,sigma);
    gausXray844= new TF1("gausXray844","gaus",xmin_histo,xmax_histo);
    gausXray844->SetParameters(1,0.844,sigma);
    gausXray1809= new TF1("gausXray1809","gaus",xmin_histo,xmax_histo);
    gausXray1809->SetParameters(1,1.809,sigma);
 
    hXray66 = new TH1F("hXray66", "", nbins,xmin_histo,xmax_histo);
    hXray66->FillRandom("gausXray66",pulseNumXrays66);
    hXray347 = new TH1F("hXray347", "", nbins,xmin_histo,xmax_histo);
    hXray347->FillRandom("gausXray347",pulseNumXrays347);
    hXray844 = new TH1F("hXray844", "", nbins,xmin_histo,xmax_histo);
    hXray844->FillRandom("gausXray844",pulseNumXrays844);
    hXray1809 = new TH1F("hXray1809", "", nbins,xmin_histo,xmax_histo);
    hXray1809->FillRandom("gausXray1809",pulseNumXrays1809);
    
    hXray->Add(hXray66);
    hXray->Add(hXray347);
    hXray->Add(hXray844);
    hXray->Add(hXray1809);
  }


  //Generated histos
  double numberCountsXray = hXray->Integral(hXray->FindFixBin(xmin_histo), hXray->FindFixBin(xmax_histo), "");
  //Before adding Xrays to bremsstrahlung
  double numberCountsBrems = hBrems->Integral(hBrems->FindFixBin(xmin_histo), hBrems->FindFixBin(xmax_histo), "");


  //****************************
  //-----PLOTTING---------------
  //****************************
  double xx1[2]={0.25,0.4};
  double yy1[2]={0,ymax};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  
  //Function that generates the background
  expBrems->SetLineColor(kRed+1);
  expBrems->SetMarkerColor(kRed+1);
  expBrems->SetMarkerStyle(6);
  //expBrems->Draw("");

  //Histogram bremsstrahlung background
  hBrems->SetFillColor(kOrange-3);
  hBrems->SetLineColor(kOrange-3);
  //hBrems->Draw("same");

  //Histogram Xray peaks
  hXray->SetLineColor(kOrange-3);
  //hXray->Draw("same");

  //Histogram Background+X-ray
  //Add histograms together
  hBrems->Add(hXray);
  hBrems->SetFillStyle(3001);
  hBrems->SetLineColor(kOrange-3);
  hBrems->SetFillColor(kOrange-3);
  hBrems->Draw("same");

  TH1* hcBrems = (TH1*)hBrems->Clone();
  hcBrems->SetName("hcBrems");

  //Remove Background
  Double_t source[nbins];
  TH1F *d    = new TH1F("d","",nbins,xmin_histo,xmax_histo);

  TSpectrum *s = new TSpectrum();
  for (int i = 0; i < nbins; i++) {source[i] = hcBrems->GetBinContent(i + 1);}
  s->Background(source,nbins,10,TSpectrum::kBackIncreasingWindow,
                TSpectrum::kBackOrder2,kFALSE,
                TSpectrum::kBackSmoothing3,kFALSE);
  // Draw the estimated background
  for (int i = 0; i < nbins; i++) {d->SetBinContent(i + 1,source[i]);}
  d->SetLineColor(kRed);
  d->Draw("SAME L");

  //Peak after background subtraction
  hcBrems->Add(d,-1);
  hcBrems->SetLineColor(kCyan-3);
  hcBrems->SetFillStyle(3001);
  hcBrems->SetFillColor(kCyan-3);
  hcBrems->Draw("same");

  //Draw Tlines of sigma cuts
  TLine *line1=new TLine(mean-sigmacut*sigma,0,mean-sigmacut*sigma,ymax);
  line1->SetLineColor(kBlack);
  line1->SetLineStyle(2);
  line1->SetLineWidth(2);
  line1->Draw("same");

  TLine *line2=new TLine(mean+sigmacut*sigma,0,mean+sigmacut*sigma,ymax);
  line2->SetLineColor(kBlack);
  line2->SetLineStyle(2);
  line2->SetLineWidth(2);
  line2->Draw("same");
  
  //Output histos
  double numberCountsBackSignal = hBrems->Integral(hBrems->FindFixBin(mean-sigmacut*sigma), hBrems->FindFixBin(mean+sigmacut*sigma), "");
  double numberCountsSignal = hcBrems->Integral(hcBrems->FindFixBin(mean-sigmacut*sigma), hcBrems->FindFixBin(mean+sigmacut*sigma), "");
  double numberCountsBack = d->Integral(d->FindFixBin(mean-sigmacut*sigma), d->FindFixBin(mean+sigmacut*sigma), "");
  double Brems = numberCountsBackSignal-numberCountsSignal;
  double signal_to_total = numberCountsSignal/numberCountsBackSignal;
  double signal_to_background = numberCountsSignal/Brems;

  if(debugout==true){
  std::cout<<"Number of counts in Full generated Xray peak histogram: "<<numberCountsXray<<std::endl;
  std::cout<<"Number of counts in Full generated Bremsstrahlung histogram: "<<numberCountsBrems<<std::endl;
  std::cout<<"Using: mean: "<<mean<<" sigma: "<<sigma<<" mean-"<<sigmacut<<"sigmas: "<<mean-sigmacut*sigma<<" mean+"<<sigmacut<<"sigmas: "<<mean+sigmacut*sigma<<std::endl;
  std::cout<<"Number of counts in Xray+brems +-"<<sigmacut<<"sigmas (using fitted histogram): "<<numberCountsBackSignal<<std::endl;
  std::cout<<"Number of counts in Xray peak after background substraction +-"<<sigmacut<<"sigmas: "<<numberCountsSignal<<std::endl;
  std::cout<<"Number of counts in background: "<<Brems<<" = "<<numberCountsBack<<std::endl;
}

  /*
  std::cout<<"-----OUTPUT-----"<<std::endl;
  std::cout<<"Time(s): "<<TimeSim<<std::endl;
  std::cout<<"Resol(MeV): "<<sigma<<std::endl;
  std::cout<<"sigmacut: "<<sigmacut<<std::endl;
  std::cout<<"Energy: "<<mean<<std::endl;
  std::cout<<"CountsSignalTheory: "<<numberCountsXray<<std::endl;
  std::cout<<"CountsSignalBrems(+-sigmacut): "<<numberCountsBackSignal<<std::endl;
  std::cout<<"CountsSignalsubtracted: "<<numberCountsSignal<<std::endl;
  std::cout<<"SignalBrems-Signal=Brems: "<<numberCountsBack<<std::endl;
  std::cout<<"Signal/(Signal+Background): "<<signal_to_total<<std::endl;
  std::cout<<"Signal/Background: "<<numberCountsSignal/Brems<<std::endl;
  std::cout<<""<<std::endl;
  */

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(hBrems,"#splitline{^{27}Al energy spectrum +}{\nBremsstrahlung}","f");
  legend->AddEntry(d,"#splitline{Estimated Bremsstrahlung}{Background}","l");
  legend->AddEntry(hcBrems,"347keV peak","f");
  //legend->AddEntry(hXray,"^{27}Al energy spectrum","f");
  legend->Draw("same");

 
  canvas->Modified();
  canvas->Update();
  //canvas->Print(".png");                                                                                             


#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)canvas->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif  


  //return a vector with counts in the signal and counts in background
  std::vector<double> SignalBackground;
  SignalBackground.push_back(numberCountsSignal);
  SignalBackground.push_back(numberCountsBack);
  SignalBackground.push_back(numberCountsXray);
 
  return SignalBackground;
}//end Sim Function





int main(int argc, char *argv[]){

  //argv[0]=program, argv[1]=Bremsrate_kHz(kHz), argv[2]=TimeSim(s), argv[3]=nbins, argv[4]=meanE(MeV), argv[5]=sigma(MeV), argv[6]=sigmacut, argv[7]=ymax, argv[8]=use weight, 0no, 1yes
  double Bremsrate_kHz = std::stod(argv[1]);
  double TimeSim = std::stod(argv[2]);
  int nbins = int(std::stod(argv[3]));
  double meanE = std::stod(argv[4]);
  double sigma = std::stod(argv[5]);
  double sigmacut = std::stod(argv[6]);
  double ymax = std::stod(argv[7]);
  double useweight = std::stod(argv[8]);

  std::vector<double> Signal, TrueXrayCounts;
  double sum_Xraysreco = 0;
  double sum_Xraystrue = 0;
  unsigned long int seed = 0;
  std::vector<unsigned long int> seedarray;
  unsigned long int num_experiments = 1000;

#if defined(USE_GRAPHICS)
  num_experiments = 1;
#endif

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;


  t1 = boost::chrono::high_resolution_clock::now();
  
  for(unsigned long int i=1; i<=num_experiments; i++){
    //If seed = 0 get a random event every time (like chrono)  
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    seedarray.push_back(seed);
    //std::cout<<""<<std::endl;
    //std::cout<<"NEW experiment, seed: "<<seed<<std::endl;
    //Generates signal and background
    std::vector<double> SignalBackground = BremsstrahlungSim(argc,argv,Bremsrate_kHz,TimeSim,nbins,meanE,sigma,sigmacut,ymax,seed,useweight);
    
    //std::cout<<"Signal: "<<SignalBackground.at(0)<<" Background: "<<SignalBackground.at(1)<<std::endl;
    Signal.push_back(SignalBackground.at(0));
    TrueXrayCounts.push_back(SignalBackground.at(2));
    
    sum_Xraysreco = sum_Xraysreco+SignalBackground.at(0);
    sum_Xraystrue = sum_Xraystrue+SignalBackground.at(2);
    double mean_Xraysreco = 0;
    double mean_Xraystrue = 0;
    if(i==num_experiments){
      //Calculate rms
      mean_Xraysreco = sum_Xraysreco/Signal.size();
      mean_Xraystrue = sum_Xraystrue/TrueXrayCounts.size();
      double sumsquared_Xraysreco = 0;
      double sumsquared_Xraystrue = 0;
      for(unsigned long int j=0; j<Signal.size(); j++){ 
	sumsquared_Xraysreco = sumsquared_Xraysreco+((Signal.at(j)-mean_Xraysreco)*(Signal.at(j)-mean_Xraysreco));
	sumsquared_Xraystrue = sumsquared_Xraystrue+((TrueXrayCounts.at(j)-mean_Xraystrue)*(TrueXrayCounts.at(j)-mean_Xraystrue));
	//double test = (Signal.at(j)-mean);
      }
      double signal_rms = sqrt(sumsquared_Xraysreco/Signal.size());
      double uncertainty = signal_rms/mean_Xraysreco;

      double Xraystrue_rms = sqrt(sumsquared_Xraystrue/TrueXrayCounts.size());
      //std::cout<<""<<std::endl;

      std::cout<<"Rate(kHz): "<<Bremsrate_kHz<<std::endl;
      std::cout<<"Time(s): "<<TimeSim<<std::endl;
      std::cout<<"Resol(MeV): "<<sigma<<std::endl;
      std::cout<<"sigmacut: "<<sigmacut<<std::endl;
      std::cout<<"Energy: "<<meanE<<std::endl;
      std::cout<<"mean(counts): "<<mean_Xraysreco<<std::endl;
      std::cout<<"RMSsignal(counts): "<<signal_rms<<std::endl;
      std::cout<<"UncertaintyRMS/meansignal: "<<uncertainty<<std::endl;
      std::cout<<"Real mean Xrays simulated in the peak+-rms: "<<mean_Xraystrue<<std::endl;
      std::cout<<"+- "<<Xraystrue_rms<<std::endl;

      std::cout<<"SEEDARRAY: "<<std::endl;
      for(unsigned long int s=0; s<seedarray.size(); s++){
std::cout<<s<<"- "<<seedarray.at(s)<<std::endl;
      }

      t2 = boost::chrono::high_resolution_clock::now();
      std::cout << "Brems computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) <<" ms"<<std::endl;
  
      /*
      if(uncertainty<=0.1){
      //write to file energy res brems rate and timesim
      exit(0);
      }
      else{ 
      //Add 1s to the simulation
      TimeSim=TimeSim+1;
      std::cout<<"New simulation time: "<<TimeSim<<" s"<<std::endl;
      i=1;
      sum=0;
      }
      */
    }//if i=num experiments
  }//for num experiments

  return 0;
}
