#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <boost/chrono.hpp>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"

#include "TApplication.h"
#include "TRootCanvas.h"

#include "STMDAQ-TestBeam/utils/Logger.hh"
#include "STMDAQ-TestBeam/utils/Random.hh"
#include "STMDAQ-TestBeam/Claudia/New_eh_Simulation/HPGe_ehSim.hh"


void HPGe_ehModelSim(int argc, char *argv[], std::string rate_stringkHz, double Energy, double noiseSD, double timeSample, std::string  output_path){

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  TCanvas* canvas = new TCanvas("c");

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  ehDriftFunctions fehDriftFunctions;

  double tadc = 1.0/(fehDriftFunctions.fADC); //us

  //ADC Counts simulated data
  unsigned long int sampleNum = floor(timeSample/tadc);
  std::cout<<"Number of ADC values in the bin file: "<<sampleNum<<std::endl;
  std::cout<<"fADC: "<<fehDriftFunctions.fADC<<" MHz"<<std::endl;
  std::cout<<"Real time of the sample: "<<sampleNum*tadc<<" us"<<std::endl;

  //Rate
  double ratekHz = std::stod(rate_stringkHz);
  double rateHz = ratekHz*1000;
  std::cout<<"Rate: "<<rateHz<<" Hz"<<std::endl;
  //Rate in MHz
  double rate = rateHz * 1e-6;

  //Average number of pulses in the sample
  int pulseNum = rate*timeSample;
  std::cout<<"Number of pulses generated (real number of simulated pulses at the end of the file): "<<pulseNum<<std::endl;
  std::cout<<"Simulated Pulse Energy: "<<Energy<<" keV = "<<Energy/fehDriftFunctions.ADC_toE<<" ADC Counts"<<std::endl;

  t1 = boost::chrono::high_resolution_clock::now();

  //Initialise array with induced charge, voltage in preamp, ADC Counts from ADC and time
  double* Qpulse = new double[sampleNum];
  double* Voltage = new double[sampleNum];
  int16_t* ADC = new int16_t[sampleNum];
  double* timeADC = new double[sampleNum];


  //Initialise Qpulse array to 0
  for(unsigned long int i=0; i<sampleNum; i++){
    Qpulse[i] = 0;
    Voltage[i] = 0;
  }

  //Intialise data array of ADC values  with random numbers following a gaussian, the input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC=noiseSD*38.5;
  std::cout<<"NoiseSD: "<<noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts"<<std::endl;
  
  for(unsigned long int i=0; i<sampleNum; i++){
    double noise = Random::Instance()->GaussValue(0, sigma_noise_ADC);
    ADC[i] = noise;
  }						

  // Now, calculate and add pulses to Qpulse array, loop over average number of pulses       
  unsigned long int timeIndex = 0;
  int realpulses_binary = 0;
  for (int i = 0; i < pulseNum; i++){

    //Randomly generate pulse times in clock ticks for a given rate within sample time
    timeIndex += int(Random::Instance()->PoissValue(rate)/tadc); //mean of timings distribution in us, gives the time separation
    //Need to add this
    if(timeIndex>=sampleNum){std::cout<<""<<std::endl; std::cout<<"-----Peak generated at: "<<timeIndex*tadc<<" us "<<" outside sample lenght of: "<<timeSample<<" us -----END LOOP-----"<<std::endl; continue;}
   
    //Generate the number of comptons and photoelectric effects according to geant4 simulation distributions
    int n_compt_phot= fehDriftFunctions.Number_compt_phot(Energy);
   
    std::cout<<"***********************NEW EVENT: "<<i<<" with "<< n_compt_phot<<" comptons***********************"<<std::endl;

    //total number of eh pairs in the whole event
    double Nehevent=0;
    //Get the distribution of the energy for each compton
    std::vector<double> energiescomptons = fehDriftFunctions.Distribute_energies(Energy, n_compt_phot);
    double r0;

    //Generate initial position
    std::vector<double> InitPos=fehDriftFunctions.PosActive();

    //Get the index value of the last element in the pulse 
    unsigned long int max = timeIndex + fehDriftFunctions.pulseLength/tadc;
    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + (fehDriftFunctions.pulseLength/tadc) > sampleNum) {max = sampleNum;}
    
    std::cout<<"Peak "<<i<<" at "<<double(timeIndex*tadc)<<" us"<<std::endl;

    //Loop over the number of Comptons in the event
    for(int c=0;c<n_compt_phot;c++){

      //Get the energy and the number of eh pairs created in the Compton
      double Ei=energiescomptons.at(c);
      int Nehi=fehDriftFunctions.Neh(Ei);
      std::cout<<"Nehi: "<<Nehi<<std::endl;
      Nehevent +=Nehi;

      //Calculate the positions for following comptons              
      if(c==0){r0=InitPos.at(0);}
      else{r0=fehDriftFunctions.PosCompPhot(InitPos,Ei);}
      std::cout<<"Ei: "<<Ei<<std::endl; 
      std::cout<<"---------New compton at: "<<r0<<" cm, energy: "<<Ei<<" keV, Neh: "<<Nehi<<" ---------"<<std::endl;

      int plot_comptcounter=0;
     //Loop over the number of values in a pulse, starting from the randomly generated pulse time
      for (unsigned long int j = timeIndex; j < max; j++){
	//Find the time starting from one
	double time = (j+1-timeIndex)*tadc; //starting in tadc us not in 0 us 
	Qpulse[j] += Nehi*fehDriftFunctions.Q_tNtype(time, r0);
	//std::cout<<Qpulse[j]/fehDriftFunctions.q0<<std::endl; 
      }//for timeIndex

    }//for n_compt_phot   

    std::cout<<"Neh event: "<<Nehevent<<std::endl;

    //shaping time is the first time where the induced charge is Nehxq0
    double ts=0; //in us
    
    //we have the charge induced in the detector (Qpulse) and we need to add the preamp shaping
    //Preamplifier effect
    for (unsigned long int j = timeIndex; j < max; j++){
      double timeV=(j+1-timeIndex)*tadc;
	//std::cout<<"shaping time: "<<ts<<" Q(t): "<<Qpulse[j]/fehDriftFunctions.q0<<" (-1)*Nehevent: "<<(-1)*Nehevent<<std::endl;}
      //When Qpulse/q0 is equal to  Neh, then shape it
      int var1=Qpulse[j]/fehDriftFunctions.q0;
      int var2=(-1)*Nehevent;
      //std::cout<<"var1: "<<var1<<" var2: "<<var2<<std::endl;
      if(var1==var2||(var1+1)==var2||(var1-1)==var2){ //Need to add the +1 or -1 because when converting to int we can get a difference of +-1 between var1 and var2

	if(ts==0){ts=timeV;std::cout<<"------Shaping time: "<<ts<<" us"<<std::endl;
	  Voltage[j] += fehDriftFunctions.HighPassFilter(timeV,ts,Nehevent);}
	else{Voltage[j] += fehDriftFunctions.HighPassFilter(timeV,ts,Nehevent);}
      }// if var1==var2
      
      else{Voltage[j] += Qpulse[j];}
    }//for timeIndex 


    //Qpulse=0 again 
    for(unsigned long int j=0;j<sampleNum;j++){
      Qpulse[j]=0;
    }

    //Number of simulated pulses
    realpulses_binary++;

  }//for pulseNum

  //Voltage output from preamp should be converted to ADC counts plus noise in ADC
  for(long unsigned int i=0;i<sampleNum;i++){
    ADC[i] += fehDriftFunctions.preamptoADC(Voltage[i]/fehDriftFunctions.q0);
    //std::cout<<ADC[i]<<" time: "<<i*tadc<<std::endl;
  }  

  std::cout<<"Real number of pulses stored in binary file: "<<realpulses_binary<<std::endl;

  //Store in a binary file
  //std::string stringfile = output_path+"/genData"+std::to_string(int(Energy))+"keV_"+rate_stringkHz+"kHz.bin"; 
  std::string stringfile = output_path;
  //std::string stringfile = "hi.bin";
  char file_name[stringfile.size()+1];//as 1 char space for null is also required
  strcpy(file_name, stringfile.c_str());

  FILE * fp = fopen(file_name, "wb");
  fwrite(&ADC[0], sizeof(int16_t), sampleNum, fp );
  fclose(fp);
  std::cout<<"Binary file: "<<stringfile<<" created."<<std::endl;

  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "HPGe Simulation computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  //****************************
  //-----PLOTTING---------------
  //****************************
  double xx1[2]={0,5000};
  //double yy1[2]={-800000,1.5};
  double yy1[2]={-2000,100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  //graph1->GetYaxis()->SetTitle("N_{eh}#times(Q(t)/q0)");
  //graph1->GetYaxis()->SetTitle("V_{preamp}/q0");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  
  //Plot the voltage from preamp in green
  TGraph* grvoltage = new TGraph();
  //ADC counts from ADC in red
  TGraph* grADC = new TGraph();

  double timerangeplot=5000; //us
  int ADCrangeplot=int(timerangeplot/tadc); //ADC Counts
  
  for(long unsigned int i=0;i<sampleNum;i++){
    grADC->SetPoint(i,i*tadc,ADC[i]);
  }


  for(long unsigned int i=0;i<sampleNum;i++){
    grvoltage->SetPoint(i,i*tadc,Voltage[i]/fehDriftFunctions.q0);
  }


  std::string ratelatex = rate_stringkHz+" kHz";
  char* ratechar = const_cast<char*>(ratelatex.c_str());

  TLatex latex1;
  latex1.SetTextSize(0.05);
  latex1.DrawLatex((xx1[0]+xx1[1])/2,-1000,ratechar);

  grvoltage->SetLineColor(kGreen+1);
  grvoltage->SetMarkerColor(kGreen+1);
  grvoltage->SetMarkerStyle(6);
  //grvoltage->Draw("same,p");

  grADC->SetLineColor(kRed+1);
  grADC->SetMarkerColor(kRed+1);
  grADC->SetMarkerStyle(6);
  grADC->Draw("same,p");



  canvas->Modified();
  canvas->Update();
  //canvas->Print(".png");
#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)canvas->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif


}//end Sim Function











int main(int argc, char *argv[]){

  // argv[0]=program, argv[1]=rate(kHz), argv[2]=Energy(keV), argv[3]=noiseSD(mV), argv[4]=timeSample(us), argv[5]=seed (0 for random numbers using chrono, rest the value of the seed to use), argv[6]=output path
  std::string rate_stringkHz = std::string(argv[1]);
  double Energy = std::stod(argv[2]);
  double noiseSD = std::stod(argv[3]);
  double timeSample = std::stod(argv[4]);
  unsigned int seed_value = std::stod(argv[5]);
  std::string  output_path = std::string(argv[6]);

  // Initialise the Logger (set threshold for messages to be DEBUG (other threshold are INFO, WARNING, ERROR)
  Logger::Instance(Logger::DEBUG);
  Logger::Instance()->setStylePlain();
  
  if (seed_value == 0)
    Random::Init(); // initialise with random seed
  else
    Random::Init(seed_value); // initialise with this seed


  HPGe_ehModelSim(argc,argv,rate_stringkHz,Energy,noiseSD,timeSample,output_path);

  return 0;
}
