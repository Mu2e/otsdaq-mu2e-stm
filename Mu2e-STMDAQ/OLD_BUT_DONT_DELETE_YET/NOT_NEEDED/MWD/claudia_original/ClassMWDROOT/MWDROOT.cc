#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"

#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "STMDAQ-TestBeam/MWD/claudia_original/MWD.h"
#include "STMDAQ-TestBeam/MWD/claudia_original/data.hh"
#include "STMDAQ-TestBeam/MWD/claudia_original/peaks.hh"

//Read FNAL data headers K
#include "STMDAQ-TestBeam/MWD/claudia_original/FNALK/calcFuncs.h"

//using namespace std;

void MWDROOT(int argc, char *argv[],std::string  filename, int M, int L, double peakssim, double tau){

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  //************************************************************
  //******************CONFIGURATION VARIABLES******************
  //************************************************************
  // ------------- Options and parameters to change -------------  
  bool simulation = true; //if this is true: use simulation configuration parameters, if this is false: use data (Run109 and 110) configuration parameters
  bool fnalheaders = false; //if this is true, we need to skip headers from the data in the binary files 

  bool calculatemean_and_rms; //if this is true mean_baseline and rms_baseline are calculated using MWD::calculate_baseline(), if this is false, mean_baseline and rms_baseline are inputs
  bool savepeaks_toROOTfile;
  bool readpeaks_fromROOTfile;

  int Draw_option; // 1 for drawing peak histograms and fits, 2 for drawing the signal (adc values) and mwd outputt (l values), 3 for drawing a histogram with noise in ADC values
  bool printtime_peaks; // true for printing in the screen time and energy of peaks, false not printing it
  // MWD::MWD
  //double tau; //ns
  double nsigma_cut; //Used if cutmode = 1
  double thresholdgrad; //Used for calculating the baseline
  double fADC; //MHz
  int cut_mode; //Cutmode (1 using sigma, 2 using a fixed cut)
  double fixed_cut_parameter;

  // MWD::find_peaks
  double mean_baseline;
  double rms_baseline; //This is for cutmode 1
  double time_offset; //time of the ADC0 value for each trigger, 0 in our case  

  //Calibration
  double calibration; //1 for 1ADC=0.57keV, 2 for Liverpool Calibration M=400, L=200, 3 for ELBE calibration M=400, L=200
  // ************************************************************  




  if(simulation==true){
  //************************************************************
  //******************SIMULATION CONFIGURATION******************
  //************************************************************ 
  // ------------- Options and parameters to change -------------
  //Real number of peaks simulated
  //double peakssim = 29978; // Depends on the pulse rate we are analyzing

  Draw_option = 2;
  printtime_peaks = true;
  savepeaks_toROOTfile = true;
  readpeaks_fromROOTfile = false;

  // MWD::MWD
  //tau = 70000; //ns
  nsigma_cut = 1; //Used if cutmode = 1
  thresholdgrad = -0.5; //Used for calculating the baseline
  //fADC = 320.0520833313; //MHz
  fADC = 370; //MHz
  std::cout<<"fadc: "<< fADC<<" MHz, tau: "<<tau<<" ns"<<std::endl;
  cut_mode = 2; //Cutmode (1 using sigma, 2 using a fixed cut)
  //fixed_cut_parameter = -500; //Used if cutmode = 2 
  //fixed_cut_parameter = -68;
  fixed_cut_parameter = -100;
  
  // MWD::find_peaks
  calculatemean_and_rms = false;
  mean_baseline = 0;
  //mean_baseline = 432; 
  //mean_baseline = 500;
  rms_baseline = 0; //we dont use this, this is for cutmode 1
  time_offset = 0; //time of the ADC0 value for each trigger, 0 in our case

  calibration=1;
  //Calculate the number of counts (peaks) between these values
  //double countx = 660;
  //double county = 686;
  // ************************************************************ 
  }



  if((simulation==false)&&(fnalheaders==false)){
    //************************************************************
    //*****************RUN109 RUN110 CONFIGURATION****************
    //************************************************************
    // ------------- Options and parameters to change -------------   
    /*Draw_option = 2;
    printtime_peaks = true;
    savepeaks_toROOTfile = false;
    readpeaks_fromROOTfile=false;
    // MWD::MWD
    tau = 55748.2; //ns
    nsigma_cut = 4; //Used if cutmode = 1
    thresholdgrad = -0.5; //Used for calculating the baseline
    fADC = 370; //MHz
    std::cout<<"fadc: "<< fADC<<" MHz, tau: "<<tau<<" ns"<<std::endl;
    cut_mode = 1; //Cutmode (1 using sigma, 2 using a fixed cut)
    fixed_cut_parameter = 0.0; //Used if cutmode = 2

    // MWD::find_peaks
    calculatemean_and_rms = false;
    mean_baseline = 17.7; 
    rms_baseline = 4.3; //this is for cutmode 1
    time_offset = 0; //time of the ADC0 value for each trigger, 0 in our case
   
    calibration=1;*/
    //************************************************************
    //******NEW FNAL DATA_NO HEADERS CONFIGURATION****************
    //************************************************************
    // ------------- Options and parameters to change -------------   
    Draw_option = 2;
    printtime_peaks = true;
    savepeaks_toROOTfile = false;
    readpeaks_fromROOTfile=false;
    // MWD::MWD
    tau = 55748.2; //ns
    nsigma_cut = 4; //Used if cutmode = 1
    thresholdgrad = -0.5; //Used for calculating the baseline
    fADC = 300; //MHz
    std::cout<<"fadc: "<< fADC<<" MHz, tau: "<<tau<<" ns"<<std::endl;
    cut_mode = 2; //Cutmode (1 using sigma, 2 using a fixed cut)
    fixed_cut_parameter = -350.0; //Used if cutmode = 2

    // MWD::find_peaks
    calculatemean_and_rms = false;
    mean_baseline = -303.468;
    rms_baseline = 4.3; //this is for cutmode 1
    time_offset = 0; //time of the ADC0 value for each trigger, 0 in our case
   
    calibration=1;
    // ************************************************************
  }



  if((simulation==false)&&(fnalheaders==true)){
    //************************************************************
    //**********POTASIUM AND COSMICS FNAL CONFIGURATION***********
    //************************************************************
    // ------------- Options and parameters to change -------------   
    Draw_option = 2;
    printtime_peaks = true;
    savepeaks_toROOTfile = false;
    readpeaks_fromROOTfile = false;
    // MWD::MWD
    tau = 55748.2; //ns
    nsigma_cut = 4; //Used if cutmode = 1
    thresholdgrad = -0.5; //Used for calculating the baseline
    fADC = 300; //MHz
    std::cout<<"fadc: "<< fADC<<" MHz, tau: "<<tau<<" ns"<<std::endl;
    cut_mode = 2; //Cutmode (1 using sigma, 2 using a fixed cut)
    fixed_cut_parameter = -200.0; //Used if cutmode = 2

    // MWD::find_peaks
    calculatemean_and_rms = false;
    mean_baseline = 0.369; 
    rms_baseline = 4.3; //this is for cutmode 1
    time_offset = 0; //time of the ADC0 value for each trigger, 0 in our case
   
    calibration=1;
    // ************************************************************
  }



  gROOT->SetStyle("ATLAS");
  //TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);
  TCanvas* c = new TCanvas("c"); 

  /////////////////////////////////////////////////////////////////////////-------------------------------- CALL MWD.cc/MWD.h CLASS --------------------------------///////////////////////////////////////////////////////////////////////// 

  int n; 
  struct stat st;
  stat(filename.c_str(), &st);
  data* d = new data();
  d->t0 = 0xdeadbeef; //whatever


  //Read the file, need to subtract the header from FNAL potasium data (readFile function) 

  if(fnalheaders==false){
    n = st.st_size/2;
    d->nadc = n;
    d->adc = new int16_t[d->nadc];

    std::ifstream myFile;
    myFile.open(filename, std::ios::in | std::ios::binary);
    myFile.read( (char*) d->adc, n*sizeof(d->adc[0]));
    myFile.close();

  }
  else{
    std::vector<int16_t> DataADC;
    char* fname = const_cast<char*>(filename.c_str());
    calcFuncs fcalcFuncs;
    //Reads the file, subtract the headers and fill the data into a vector
    DataADC = fcalcFuncs.readFile(fname);
    n = DataADC.size();
    d->nadc = n;
    d->adc = new int16_t[d->nadc];

    /*    for(unsigned long int i=0;i<n;i++){
      d->adc[i]=DataADC[i];
      if(i<100){std::cout<<d->adc[i]<<std::endl;}
      }*/

    memcpy(d->adc,&(DataADC[0]),n*sizeof(int16_t));
    }


  //hack to move the baseline of the signal for high energies because it exceed the range of an int16 in simulation 
  /*for(int i=0; i < n ;i++){
    //if ADC values are above the baseline
    if(d->adc[i]>10000){d->adc[i]=d->adc[i]-65536+30000;}
    //else just move the ADC values
    else{d->adc[i]=d->adc[i]+30000;}
    }
  */
  const double T0 = (1./(fADC)); //us
  double* timevalue = new double[d->nadc];

  //----------------MWD----------------
  t1 = boost::chrono::high_resolution_clock::now();

  //M, L, tau, nsigma_cut (we dont use this, it is for cutmode 1), thresholdgrad (we dont use this, it is just for calculating the baseline), fadc, cutmode (1 using sigma, 2 using a fixed cut), fixed_cut_parameter
  MWD *mwd = new MWD(M,L,tau,nsigma_cut,thresholdgrad,fADC,cut_mode,fixed_cut_parameter);
  mwd->mwd_algorithm(d);
  //l value
  double* lvalue = mwd->return_lvalue();
  
  //----------------CALCULATE MEAN AND RMS---------------- 
  if(calculatemean_and_rms==true){
    std::vector<double> baseline_values = mwd->calculate_baseline();
    mean_baseline=baseline_values.at(0);
    rms_baseline=baseline_values.at(1);
    std::cout<<"Calculated mean: "<<mean_baseline<<" Calculated rms: "<<rms_baseline<<std::endl;
  }

  //----------------FIND PEAKS----------------  
  //mean_baseline, rms_baseline, time_offset
  peaks* peak_data = mwd->find_peaks(mean_baseline, rms_baseline, time_offset);

  //we have to fill the histogram with the peak heights in adc counts and convert them to energy
  //All the peaks found by the algorithm
  int npeaks; 
    if(readpeaks_fromROOTfile==false){
      npeaks= peak_data->npeaks;
    }

    t2 = boost::chrono::high_resolution_clock::now();
    std::cout << "MWD Algorithm computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;
  //----------------SAVE PEAKS TO ROOT FILE FOR SEPARATE ANALYSIS---------------- 
  std::string rootname;
  if((savepeaks_toROOTfile==true)||(readpeaks_fromROOTfile==true)){
    //Old Simulation
    //rootname="/data1/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/Resolution_Efficiency_ML/1kHz/MWD"+filename.substr(filename.find("GendataNoise_"), 18)+"_M"+std::to_string(M)+"L"+std::to_string(L)+"_Peaks.root";
    
    //RUN109 Raw 
    //rootname="/data1/cgarcia/DATA/MWD_Analysis/RUN109/M"+std::to_string(M)+"L"+std::to_string(L)+"/"+filename.substr(filename.find("run"), 12)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
    //RUN109 Suppressed
    //rootname="/data1/cgarcia/DATA/MWD_Analysis/RUN109/SuppresedFiles_tbefore2us_tafter10us_v3/M400L200/"+filename.substr(filename.find("Suppressed_"), 10)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
    //rootname="/data1/cgarcia/DATA/MWD_Analysis/RUN109/SuppresedFiles_PRE5us_POST10us_HZDR/M1000L500/"+filename.substr(filename.find("Suppressed_"), 10)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
    //rootname="/data1/cgarcia/DATA/Claudia/HLSData/SoftwareClass_2048ADCperRAM/Spill/M400L200/Suppressed_energypeaks_bin_00.root";
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV_testHLS/M400L200/Raw_energypeaks_bin_30kHz.root";
    
    //RUN110
    //rootname="/data1/cgarcia/DATA/MWD_Analysis/RUN110/M400L200/"+filename.substr(filename.find("run"), 12)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
    

    //New Simulation
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/Resolution_Efficiency_ML/10kHz/MWD"+filename.substr(filename.find("Data662"), 22)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //jobs
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/1809keV_0.32mV/M400L200/MWD"+filename.substr(filename.find("Data1809"), 23)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //no jobs
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_1mV/M400L200/MWD"+filename.substr(filename.find("Data662"), 15)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/Resolution_Efficiency_ML/40kHz/MWDData662keV_40kHz_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //rootname="/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/M400L200Tau/20kHz/MWDData662keV_20kHz_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks_tau"+std::to_string(int(tau))+"ns.root"; 

    //Potassium FNAL data
    //rootname="/data1/STM_VST_DATA/MWD_Cosmics_Data/M400L200_notrigcut/MWD"+filename.substr(filename.find("stm-daq"), 36)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root"; 

    //Simulation
    //rootname="MWD_"+filename.substr(filename.find("genData347keV_0.012kHz_job"), 28)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";

    //rootname="MWD_"+filename.substr(filename.find("5kHz_FlashANDXrays_job"), 24)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"Noise_energypeaks.root";
    //rootname="MWD_"+filename.substr(filename.find("30kHz_FlashANDXrays_job"), 25)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"Noise_energypeaks.root";
    //rootname="MWD_"+filename.substr(filename.find("100kHz_FlashANDXrays_job"), 26)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"Noise_energypeaks.root";
    
    //rootname="MWD_"+filename.substr(filename.find("1kHz_FlashANDXrays_job"), 24)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //rootname="MWD_"+filename.substr(filename.find("50kHz_FlashANDXrays_job"), 25)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //rootname="MWD_"+filename.substr(filename.find("100kHz_FlashANDXrays_job"), 26)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";

    //rootname="MWD_"+filename.substr(filename.find("1kHz_Flash_job"), 16)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    //rootname="MWD_"+filename.substr(filename.find("75kHz_Flash_job"), 17)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root";
    rootname="MWD_"+filename.substr(filename.find("100kHz_Flash_job"), 18)+"_M"+std::to_string(M)+"_L"+std::to_string(L)+"_energypeaks.root"; 
    
    //rootname="hi";
    std::cout<<rootname<<std::endl;
  }
  if(savepeaks_toROOTfile==true){
    //Store ADC heights in .root file                                                               
    TFile *rootfile=new TFile(rootname.c_str(),"recreate");
    TTree*tree=new TTree("treeADC","treeADC");
    double peaks;
    tree->Branch("peaks",&peaks);
    for(int i=0;i<npeaks;i++){
      peaks=peak_data->peak_heights.at(i);
      tree->Fill();
    }

    rootfile->Write();
    rootfile->Close();

  }
 

  //MWD mean baseline
  //MEAN THRESHOLD MWD l[i]
  /*  double t_1,t_2;
  t_1=100;//us
  t_2=1000;//us
  TH1F *hav = new TH1F("hav", "", 100, -100000, 100000);
  for(int i=(t_1*fADC);i<(t_2*fADC);i++){
    hav->Fill(lvalue[i]);
    }
  std::cout<<"Mean of MWD output between: "<<t_1<<" and "<<t_2<<" us: "<<hav->GetMean()<<" ADC Counts (cross-check)"<<std::endl;
  */

  double peakADC;
  std::vector<double> peakROOT;


  TFile *input;
  TTree* treeread;

  if(readpeaks_fromROOTfile==true){
    input=new TFile(rootname.c_str());
    treeread=(TTree*)input->Get("treeADC");
    treeread->SetBranchAddress("peaks",&peakADC);
    npeaks=treeread->GetEntries();
  }

  if(readpeaks_fromROOTfile==true){
    for(int i=0 ; i< npeaks; i++){
      treeread->GetEntry(i);
      peakROOT.push_back(peakADC);
      //peak stored in peakADC 
    }
  }


  /////////////////////////////////////////////////////////////////////////-------------------------------- DRAW OPTIONS --------------------------------/////////////////////////////////////////////////////////////////////////


  //Draw histogram with peaks and fit
   if(Draw_option==1){
    double xhist = 0;
    double yhist = 5000;

    //Fill a histogram with a wide range of energies
    TH1F*henergy = new TH1F("henergy","",100 , xhist, yhist);

    std::cout<<"Check: n peaks found: "<<npeaks<<std::endl;
    for(int i=0 ; i< npeaks; i++){
    
      if(readpeaks_fromROOTfile==true){peakADC=peakROOT.at(i);}
      if(readpeaks_fromROOTfile==false){peakADC=peak_data->peak_heights.at(i);}
      if(calibration==1){
	//henergy->Fill((peak_data->peak_heights.at(i)-0.354947)/(-1.75514));}
	henergy->Fill(peakADC*(-0.57));} 
      if(calibration==2){
        henergy->Fill((peakADC-1.738)/(-1.767));}

      if(calibration==3){
	henergy->Fill((peakADC+1.76)/(-1.785));}
    
    }

   
    henergy->GetXaxis()->SetTitle("E[keV]");
    
    //take the mean of the histogram
    Double_t fitmean = henergy->GetMean();
    Double_t rms = henergy->GetRMS();
    //fitmean = 675;
    //fitmean = 1836;
    fitmean = 665;
    //fitmean =1400;

    if(tau<40000){ fitmean = ((50000-tau)/1000) + fitmean;}
    else if(tau>59000){fitmean = fitmean - (tau-50000)/4000;}
    else{fitmean = fitmean;}

    //Create another histogram with a lower range based on fitmean to fit it    
    double fitx, fity;
    //Ranges for 1kHz to 20kHz
    //if(rms < 0.9){fitx = fitmean-5;fity = fitmean+5;}
    //else{fitx = fitmean-10;fity = fitmean+10;}
    fitx = fitmean-10; 
    fity = fitmean+10;

    //Ranges 30kHz-50kHz
    //fitx = fitmean-20;
    //fity = fitmean+20;

    //Ranges 80kHz-200kHz
    //fitx = fitmean-40;
    //fity = fitmean+40;

    //fitx = xhist;
    //fity = yhist;

    TH1F*hraw = new TH1F("hraw","", 100, fitx, fity);
    TH1F*hADC = new TH1F("hADC","", 100, -100000, 100000);
    double peak_energy;
    int countnumber=0;

    double countx = fitx;
    double county = fity;



    for(int i=0 ; i< npeaks; i++){
      if(readpeaks_fromROOTfile==true){peakADC=peakROOT.at(i);}
      if(readpeaks_fromROOTfile==false){peakADC=peak_data->peak_heights.at(i);}
      if(calibration==1){
	// peak_energy=(peak_data->peak_heights.at(i)-0.354947)/(-1.75514);}
	peak_energy=(peakADC*(-0.57));}

      if(calibration==2){
        peak_energy=(peakADC-1.738)/(-1.767);}

      if(calibration==3){
        peak_energy=(peakADC+1.76)/(-1.785);}


    
      hraw->Fill(peak_energy);
      hADC->Fill(peakADC);
      if((peak_energy>=countx)&&(peak_energy<=county)){
	countnumber++;
      }
    
    }


    std::cout<<"Fit using range: " <<fitx <<" and "<<fity<<" and mean: "<<fitmean<<std::endl;
    //Here do the fit to get the resolution
    TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", fitx, fity);
    Fit->SetParameters(1,fitmean,1);
    hraw->Fit(Fit,"0","",fitx, fity);
    Fit->SetLineColor(kRed);
    Fit->SetLineStyle(2);


    hraw->Draw("");
    hraw->GetXaxis()->SetTitle("E [keV]");
    Fit->Draw("same");

    std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L);
    char* MLchar = const_cast<char*>(ML.c_str());
    TLatex latex1;
    latex1.SetTextSize(0.05);
    //latex1.DrawLatex(fitx,1,MLchar);
    //latex1.DrawLatex(fitx,1,"Old Simulation: 1 kHz, #sigma_{noise}=0.32mV");


    //Mean
    std::cout<<"Mean energy (keV) and in ADC Counts from original histogram: "<<std::endl;
    std::cout<<fitmean<<" keV, "<<hADC->GetMean()<<"+-"<<hADC->GetMeanError()<<" ADC Counts"<<std::endl;
    //Standard deviation
    std::cout<<"StDev (keV) from original histogram: "<<std::endl;
    std::cout<<rms<<std::endl;
    //write resolution and the error from fit
    std::cout<<"Resolution and error from fit: "<<std::endl;
    std::cout<<Fit->GetParameter(2)<<std::endl;
    std::cout<<Fit->GetParError(2)<<std::endl;

    //write the eficiency
    //double countsRaw = hraw->Integral(hraw->FindFixBin(fitx), hraw->FindFixBin(fity), "");
    double countsRaw = hraw->Integral(hraw->FindFixBin(countx), hraw->FindFixBin(county), "");
    double efficiency = countnumber/peakssim;
    double efficiency_error = efficiency/sqrt(countnumber);
    std::cout<<"Efficiency npeaks in photopeak/npeaks simulated and error: "<<std::endl;
    std::cout<<efficiency<<std::endl;
    std::cout<<efficiency_error<<std::endl;

    std::cout<<"Npeaks found by algorithm: "<<npeaks<<std::endl;
    std::cout<<"Npeaks simulated: "<<peakssim<<std::endl;
    std::cout<<"Number of peaks in the photopeak (using integral, depends on binning), between "<<countx <<" and "<<county<<": "<<countsRaw<<std::endl;
    std::cout<<"Number of peaks in the photopeak (using number of entries): between "<<countx <<" and "<<county<<": "<<countnumber<<std::endl;

    std::cout<<"-------Original parameters of peak histo-------"<<std::endl;
    Double_t meanerror = henergy->GetMeanError();

    Double_t rmserror = henergy->GetRMSError();
    std::cout<<"Mean: "<<fitmean<<" +- "<<meanerror<<std::endl;
    std::cout<<"St.dev: "<<rms<<" +- "<<rmserror<<std::endl;


    std::cout<<"-------Fit parameters-------"<<std::endl;
    Double_t p1 = Fit->GetParameter(1);
    Double_t e1 = Fit->GetParError(1);

    Double_t p2 = Fit->GetParameter(2);
    Double_t e2 = Fit->GetParError(2);
    std::cout<<"Mean: "<<p1<<" +- "<<e1<<std::endl;
    std::cout<<"St.dev: "<<p2<<" +- "<<e2<<" Error using sigma/sqrt(2Ncounts)= "<<p2/sqrt(2*countnumber)<<std::endl;

    //Return the chi2 to see if it's right
    double Chi2errors;
    Chi2errors=hraw->GetFunction("Fit")->GetChisquare();
    std::cout<<"Chi2= "<<Chi2errors<<std::endl;

    //Mean in ADC Counts recovered
    std::cout<<"Mean in ADC Counts recovered: "<<hADC->GetMean()<<std::endl;
    
    //File size in ADC counts to calculate the fraction of raw data kept
    std::cout<<"Number of ADCs in file: "<<std::endl;
    std::cout<<n<<std::endl;
    std::cout<<"Number of bytes in file: "<<std::endl;
    std::cout<<st.st_size<<std::endl;

    if(savepeaks_toROOTfile==true){ std::cout<<"new ROOT file created: "<<rootname<<std::endl;}
    if(savepeaks_toROOTfile==false){ std::cout<<"ROOT file not created"<<std::endl;}
    if(readpeaks_fromROOTfile==true){std::cout<<"Reading peaks from ROOT file: "<<rootname<<std::endl;}
    if(readpeaks_fromROOTfile==false){std::cout<<"Not reading peaks from ROOT file, read them from MWD after reading binary file"<<std::endl;}
   
}




  //Draw signal and MWD output
  if(Draw_option==2){

    std::cout<<"Check: n peaks found: "<<npeaks<<std::endl;
    //Fill graph with adc values and l values                                          
    TGraph* gr_adc = new TGraph();
    TGraph* gr_lvalue = new TGraph();
    //int limit = 100000000; 
    int limit = 1110000; 
    //int limit =n;

    int starttime= 0; //us
    int start_index= int(starttime/T0);
    int endtime= n*T0; //us
    int end_index= int(endtime/T0);
    //int endfile=320052083;
    int k=0;
    //for(int i=start_index;i<end_index;i++){  
      for(int i=0;i<limit;i++){   
      timevalue[k] = T0*i;
      gr_adc->SetPoint(k,timevalue[k],d->adc[i]);
      gr_lvalue->SetPoint(k,timevalue[k],lvalue[i]);
      k++;
    }

      //double xmin =starttime;//us
   //double xmax = endtime;//us     

    double xmin = 0;//us
    double xmax = 2000;//us
    double ymin = -200;
    double ymax = 500;

    double xx1[2]={xmin , xmax};
    double yy1[2]={ymin, ymax};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xmin, xmax);
    graph1->GetYaxis()->SetRangeUser(ymin,ymax);
    graph1->GetXaxis()->SetTitle("Time [#mus]");
    graph1->GetYaxis()->SetTitle("ADC Counts");
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");


    //gr_adc->SetMarkerColor(kPink);
    //gr_adc->SetLineColor(kPink); 
    gr_adc->SetMarkerColor(kBlack); 
    gr_adc->SetLineColor(kBlack);
    gr_adc->SetMarkerStyle(6);
    gr_adc->Draw("same, p");

    gr_lvalue->SetMarkerColor(kBlue);
    gr_lvalue->SetLineColor(kBlue);
    gr_lvalue->SetMarkerStyle(6);
    gr_lvalue->Draw("same, l");
    
    //Tresholdline
    if(cut_mode==2){
      TLine *line=new TLine(xx1[0],fixed_cut_parameter,xx1[1],fixed_cut_parameter);
      line->SetLineColor(kRed);
      line->Draw("same");

      auto legend = new TLegend(0.6,0.2,0.88,0.4);
      //legend->AddEntry(gr_adc,"Simulated Data","l");
      legend->AddEntry(gr_adc,"Data","l"); 
      //legend->AddEntry(gr_adc,"Suppressed Data","l");
      legend->AddEntry(gr_lvalue,"MWD Output","l");
      legend->AddEntry(line,"Threshold","l");
      legend->Draw("same");
    }
    if(cut_mode==1){
      double line_threshold= mean_baseline-nsigma_cut*rms_baseline;
      TLine *line=new TLine(xx1[0],line_threshold,xx1[1],line_threshold);
      line->SetLineColor(kRed);
      line->Draw("same");
      
      auto legend = new TLegend(0.6,0.2,0.88,0.4);
      legend->AddEntry(gr_adc,"Raw Data","l");
      //legend->AddEntry(gr_adc,"Suppressed Data","l");
      legend->AddEntry(gr_lvalue,"MWD Output","l");
      legend->AddEntry(line,"Threshold","l");
      legend->Draw("same");
    }

    int tau_mus =tau/1000;
    //std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L);
    std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L)+" #tau="+std::to_string(tau_mus)+"#mus";
    char* MLchar = const_cast<char*>(ML.c_str());    
    TLatex latex1;
    latex1.SetTextSize(0.05);
    latex1.DrawLatex((xmin+xmax)/2,(ymin+ymax)/2,MLchar);
    //latex1.DrawLatex((xmin+xmax)/2,(ymin+ymax)/2,"1 kHz");
    //c->Print("MWD_M400L200_noise_Xrays_Flash_10kHz.png");
    //c->Print("MWD_M1000L500_ZoomSup109.pdf");
  
    std::cout<<"Number of ADCs in file: "<<std::endl;
    std::cout<<n<<std::endl;
    std::cout<<"Number of bytes in file: "<<std::endl;
    std::cout<<st.st_size<<std::endl;

  }




  //Plot noise of the signal in a histogram
  if(Draw_option==3){
    gROOT->SetStyle("ATLAS");
    int nbins = 100;
    double xmin = -20;
    double xmax = 20;
    TH1F *hnoise= new TH1F("", "", nbins, xmin, xmax);

    double timemin_plot = 100; //us
    double timemax_plot = 1000; //us
    int timetoindex_min = int(timemin_plot*fADC);
    int timetoindex_max = int(timemax_plot*fADC);

    for(int i=timetoindex_min;i<timetoindex_max;i++){
      //std::cout<<d->adc[i]<<" ADC Counts "<<(d->adc[i]-0.354947)/(-1.75514)<<" keV "<<d->adc[i]/(38.5)<<" mV"<<std::endl;
      //When we plot the energy spectrum the height of the peaks are negative so we have to use this convertion:
      //hnoise->Fill((d->adc[i]-0.354947)/(-1.75514));
      //Now, the ADC values from the signal are positive so we use that 1ADC value is 0.57 keV
      //Or in mVolts 1mV is equal to 38.5 ADC count
      hnoise->Fill(d->adc[i]/(38.5)); //this is in mV
      //This is just to compare with the lvalue mean and sigma in mV
      //hnoise->Fill(lvalue[i]/(38.5));
      //hnoise->Fill(lvalue[i]);
    }


    //Normalize the y axis to the number of entries
    TH1*hnorm = (TH1*)(hnoise->Clone("hnorm"));
    hnorm->Scale(1./hnorm->Integral());
    hnorm->SetFillColor(kOrange-3);
    hnorm->SetLineColor(kOrange-3);
    //hnorm->SetFillColor(kBlue);
    //hnorm->SetLineColor(kBlue);
    //hnorm->GetYaxis()->SetRangeUser(0,0.08);
    hnorm->GetXaxis()->SetTitle("Noise [mV]");
    //hnorm->GetXaxis()->SetTitle("MWD Output [mV]");  
    //hnorm->GetXaxis()->SetTitle("MWD Output [ADC Counts]");
    //DRAW NORMALIZED HISTOGRAM
    hnorm->Draw("HIST");
    
    TF1*Fit2 = new TF1("Fit2", "[0]*TMath::Gaus(x,[1],[2])", -20, 20);
    Fit2->SetParameters(0.2,0,3);
    hnorm->Fit(Fit2,"0","",-20,20);
    Fit2->SetLineColor(kRed);
    Fit2->SetLineStyle(2);
    Fit2->Draw("same");

    std::cout<<"Resolution and meanfrom fit: "<<std::endl;
    std::cout<<Fit2->GetParameter(2)<<std::endl;
    //std::cout<<Fit2->GetParError(2)<<std::endl;
    std::cout<<Fit2->GetParameter(1)<<std::endl;

    std::string fitsigma_string =std::to_string(Fit2->GetParameter(2));
    std::string fitmean_string =std::to_string(Fit2->GetParameter(1));
    
    std::string roundedsigma = fitsigma_string.substr(0, fitsigma_string.find(".")+3);
    std::string roundedmean = fitmean_string.substr(0, fitmean_string.find(".")+4);


    std::string sigmanoise = "#sigma= "+roundedsigma+" mV";
    std::string meannoise = "#mu= "+roundedmean+" mV";
 
    //std::string sigmanoise = "#sigma= "+roundedsigma;
    //std::string meannoise = "#mu= "+roundedmean;

    char* sigmachar = const_cast<char*>(sigmanoise.c_str());
    char* meanchar = const_cast<char*>(meannoise.c_str());
    

    TLatex latex;
    latex.SetTextSize(0.06);
    latex.SetTextColor(kBlue);
    latex.DrawLatex(-17,0.037,sigmachar);
    latex.DrawLatex(-17,0.034,meanchar);
    TLatex latex1;
    latex1.SetTextSize(0.04);
    //latex1.DrawLatex(0,0.05,"Simulation 1kHz");
    latex1.DrawLatex(27,0.05,"Eu-152 (Run-110)");
    //latex1.DrawLatex(-17,0.03,"K-40");
    //c->Print("SignalLvalueFit_inADCnotmV.png");   
  }



  c->Modified();
  c->Update();
 

 
  if(printtime_peaks){
    for(int i = 0 ; i < npeaks ; i++){
    
      if(readpeaks_fromROOTfile==false){peakADC=peak_data->peak_heights.at(i);
	
	if(calibration==1){
	  std::cout<<"Peak: "<< i <<", Time: "<<peak_data->peak_times.at(i)<<" us,"<<" ADC Counts: "<<peakADC<<", Energy: "<< peakADC*(-0.57) <<" keV"<<std::endl;}
	if(calibration==2){
	  std::cout<<"Peak: "<< i <<", Time: "<<peak_data->peak_times.at(i)<<" us,"<<" ADC Counts: "<<peakADC<<", Energy: "<< (peakADC-1.738)/(-1.767) <<" keV"<<std::endl;}
	if(calibration==3){
	  std::cout<<"Peak: "<< i <<", Time: "<<peak_data->peak_times.at(i)<<" us,"<<" ADC Counts: "<<peakADC<<", Energy: "<< (peakADC+1.76)/(-1.785) <<" keV"<<std::endl;}
      }
      if(readpeaks_fromROOTfile==true){peakADC=peakROOT.at(i);
	if(calibration==1){
	  std::cout<<"Peak: "<< i <<", ADC Counts: "<<peakADC<<", Energy: "<< peakADC*(-0.57) <<" keV"<<std::endl;}
        if(calibration==2){
	  std::cout<<"Peak: "<< i <<", ADC Counts: "<<peakADC<<", Energy: "<< (peakADC-1.738)/(-1.767) <<" keV"<<std::endl;}
        if(calibration==3){
	  std::cout<<"Peak: "<< i <<", ADC Counts: "<<peakADC<<", Energy: "<< (peakADC+1.76)/(-1.785) <<" keV"<<std::endl;}
      }
    }
  }


 
  
  std::cout<<"Input file to do checks: "<<filename<<std::endl;
  std::cout<<"Ended successfully"<<std::endl;



#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
}








int main(int argc, char *argv[]){

  //argv[0]=program, argv[1]=M, argv[2]=L
  std::string  input_filename = std::string(argv[1]);
  int M = atoi(argv[2]);
  int L = atoi(argv[3]);
  double peakssim = atoi(argv[4]);
  double tau = std::stod(argv[5]);
 
  MWDROOT(argc,argv,input_filename,M,L,peakssim,tau);


  return 0;
}
