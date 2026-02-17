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

// Include drift functions header
#include "ehDriftFunctions_setseed.hh"
//#include "STMDAQ-TestBeam/HPGeSim/ehDriftFunctions_setseed.hh"

// Main pulse simulation code
void HPGe_ehModelSim(int argc, char *argv[], std::string rate_stringkHz, double Energy, double noiseSD, double timeSample,  std::default_random_engine gen, std::string  output_path){

  // If USE_GRAPHICS is defined... 
#if defined(USE_GRAPHICS)
  // Call app
  TApplication app("app", &argc, argv);
#endif
  // Set the ROOT plot style to ATLAS
  gROOT->SetStyle("ATLAS");
  // Create a TCanvas
  TCanvas* canvas = new TCanvas("c");

  //Plot Qpulse                                                                                        
  //TGraph* grQpulse = new TGraph(); 

  // Timers
  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  // Create instance of ehDriftFunctions
  ehDriftFunctions fehDriftFunctions;
  
  // Get the adc clock time
  double tadc = 1.0/(fehDriftFunctions.fADC); //us
  
  // Get the size of the full ADC sample
  unsigned long int sampleNum = floor(timeSample/tadc);
  
  // Print to screen
  std::cout << "Number of ADC values in the bin file: " << sampleNum << std::endl;
  std::cout << "fADC: " << fehDriftFunctions.fADC << " MHz" << std::endl;
  std::cout << "Real time of the sample: " << sampleNum*tadc<<" us" << std::endl;

  // Get the rate as a double in MHz, kHz and Hz
  double ratekHz = std::stod(rate_stringkHz); // kHz
  double rateHz = ratekHz*1000; // Hz
  double rate = rateHz * 1e-6; // MHz
  std::cout<<"Rate: "<<rateHz<<" Hz"<<std::endl;

  // Get the average number of pulses in the sample
  int pulseNum = rate*timeSample;
  std::cout << "Number of pulses generated (real number of simulated pulses at the end of the file): " 
	    << pulseNum << std::endl;
  std::cout << "Simulated Pulse Energy: " << Energy << " keV = " << 
    Energy/fehDriftFunctions.ADC_toE << " ADC Counts" << std::endl;

  // Get the time at the start as t1
  t1 = boost::chrono::high_resolution_clock::now();

  // Initialise arrays: induced charge, voltage in preamp, ADC Counts, time
  double* Qpulse = new double[sampleNum];
  double* Voltage = new double[sampleNum];
  int16_t* ADC = new int16_t[sampleNum];
  double* timeADC = new double[sampleNum];

  // Initialise charge and voltage arrays to 0
  for(unsigned long int i=0; i<sampleNum;i++){
    // Charge induced in the detector (Qpulse) 
    Qpulse[i]=0;
    Voltage[i]=0;
  }

  // Intialise data array of ADC values with Gaussian noise
  // The input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC = noiseSD*38.5;
  std::cout << "NoiseSD: " << noiseSD << " mV = " 
	    << sigma_noise_ADC << " ADC Counts" << std::endl;
  // Normal distribution generator for noise
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);

  // Loop over all samples
  for(unsigned long int i=0; i<sampleNum;i++){
    // Add noise from Gaussian
    double noise = distribution(gen);
    ADC[i] = noise;
  }

  // Poisson generate pulse times for a given rate within sample time
  std::poisson_distribution<int> pulseTime (1/rate); 

  // Time index
  unsigned long int timeIndex = 0;
  // Counter for real number of generated pulses 
  int realpulses_binary = 0;

  // Loop over number of pulses
  for (int i = 0; i < pulseNum; i++){

    // Generate the number of comptons and photoelectric effects 
    // according to geant4 simulation distributions
    int n_compt_phot = fehDriftFunctions.Number_compt_phot(Energy,gen);
    std::cout << "***********************NEW EVENT: " << i << 
      " with " <<  n_compt_phot << " comptons***********************" << std::endl;
    
    // Total number of eh pairs 
    double Nehevent=0;
    
    // Get the distribution of the energy for each compton
    std::vector<double> energiescomptons = fehDriftFunctions.Distribute_energies(Energy,n_compt_phot,gen);
    // Generate initial position
    std::vector<double> InitPos=fehDriftFunctions.PosActive(gen);

    // Positions of compton events
    double r0;

    // Randomly generate pulse time in clock ticks
    timeIndex += int(pulseTime(gen)/tadc);
    
    // Need to add this
    if (timeIndex >= sampleNum){
      std::cout << "" << std::endl; 
      std::cout << "-----Peak generated at: " << timeIndex*tadc << " us " 
		<< " outside sample lenght of: " << timeSample << " us -----END LOOP-----" 
		<< std::endl; 
      continue;
    }

    // Get the index value of the last element in the pulse 
    unsigned long int max = timeIndex + fehDriftFunctions.pulseLength/tadc;

    // If the pulse exceeds the sample length, max out at the sample length
    if (timeIndex + (fehDriftFunctions.pulseLength/tadc) > sampleNum){
      max = sampleNum;
    }

    // Print peak time
    std::cout << "Peak " << i << " at " << double(timeIndex*tadc) << " us" << std::endl;

    // Loop over the number of Comptons in the event
    for(int c = 0; c < n_compt_phot; c++){
    
      // Get the energy and the number of eh pairs created in the Compton
      double Ei = energiescomptons.at(c); // Energy
      int Nehi = fehDriftFunctions.Neh(Ei); // Number
      std::cout << "Nehi: " << Nehi << std::endl;

      // Add to total number of eh pairs
      Nehevent += Nehi;
      
      // Calculate the positions for following comptons              
      if (c == 0){ 
	// Initial compton position
	r0 = InitPos.at(0);
      }
      else{
	// Positions of subsequent comptons
	r0 = fehDriftFunctions.PosCompPhot(InitPos,Ei,gen);
      }
      // Print new properties of new compton event
      std::cout << "---------New compton at: " << r0 << " cm, energy: " 
		<< Ei << " keV, Neh: " << Nehi << " ---------" << std::endl;

      // Counter for plotting Compton events
      int plot_comptcounter = 0;
      
      // Loop over ADC values in pulse, starting from the randomly generated pulse time
      for (unsigned long int j = timeIndex; j < max; j++){
	
	// Find start time in tadc (not us)
	double time = (j+1-timeIndex)*tadc;
	
	// Add to charge array
	Qpulse[j] += Nehi*fehDriftFunctions.Q_tNtype(time, r0);
	// std::cout << Qpulse[j]/fehDriftFunctions.q0 << std::endl; 
	
      } // For timeIndex

      // Check if different shapes from different comptons are added to Qpulse
      /* for (unsigned long int j = timeIndex; j < max; j++){
	 grQpulse->SetPoint((j-timeIndex),(j+1-timeIndex)*tadc,Qpulse[j]/fehDriftFunctions.q0);
	 //std::cout << Qpulse[j]/fehDriftFunctions.q0 << std::endl;
	 }
	 grQpulse->SetLineColor(kAzure+1);
	 grQpulse->SetMarkerColor(kAzure+1);
	 grQpulse->SetMarkerStyle(6);
	 grQpulse->Draw("p");
	 canvas->Update();
	 canvas->Modified();
	 canvas->WaitPrimitive();
      */
      
    } // For n_compt_phot   
    
    // Print e-h pair event number
    std::cout << "Neh event: " << Nehevent << std::endl;

    // *******************
    // Preamplifier effect
    // *******************

    // Initialise shaping time (the first time where the induced charge is Nehxq0)
    double ts = 0; // us
    
    // Need to add the preamp shaping to charge induced in the detector (Qpulse) 
    // Loop over ADC values in pulse
    for (unsigned long int j = timeIndex; j < max; j++){
      
	// Find start time in tadc (not us)
      double timeV = (j+1-timeIndex)*tadc;

      // if (Nehevent == 202700){
      //std::cout << "shaping time: " << ts << " Q(t): " << Qpulse[j]/fehDriftFunctions.q0 << " (-1)*Nehevent: " << (-1)*Nehevent << std::endl;}
      
      // When Qpulse/q0 is equal to  Neh, then shape it
      int var1 = Qpulse[j]/fehDriftFunctions.q0;
      int var2 = (-1)*Nehevent;
      //std::cout << "var1: " << var1 << " var2: " << var2 << std::endl;
      
      // If Qpulse/q0 == Neh, then shape it
      // Need to add the +1 or -1 because when converting to int 
      // we can get a difference of +-1 between var1 and var2
      if (var1==var2 || (var1+1)==var2 || (var1-1)==var2){ 
	
	// If shaping time is zero...
	if(ts==0){
	  
	  // Set the shaping time as the pulse start time
	  ts = timeV;	 
	  // Print shaping time to screen
	  std::cout << "------Shaping time: " << ts << " us" << std::endl;	  
	  // Get preamp voltage
	  Voltage[j] += fehDriftFunctions.HighPassFilter(timeV,ts,Nehevent);
	}
	// Else if shaping time is not zero...
	else{
	  // Get preamp voltage
	  Voltage[j] += fehDriftFunctions.HighPassFilter(timeV,ts,Nehevent);
	  /*std::cout << "hi1" << std::endl;*/
	}
      } // If var1==var2
      // Else if Qpulse/q0 /= Neh (+/- 1), don't shape it
      else{
	Voltage[j] += Qpulse[j]; /*std::cout << "hi2" << std::endl;*/
      }
    } // For timeIndex 

    // Reinitalise Qpulse
    for(unsigned long int j = 0; j < sampleNum; j++){
      Qpulse[j]=0;
    }

    // Increment number of simulated pulses
    realpulses_binary++;

  } // for pulseNum

  // Print total number of pulses
  std::cout << "Real number of pulses stored in binary file: " << realpulses_binary << std::endl;

  // Convert voltage output from preamp to ADC counts and add to noise
  for(long unsigned int i = 0; i < sampleNum; i++){
    ADC[i] += fehDriftFunctions.preamptoADC(Voltage[i]/fehDriftFunctions.q0);
  }  

  // Store in a binary file
  // Get output path
  std::string stringfile = output_path;
  // Get file name
  char file_name[stringfile.size()+1];//as 1 char space for null is also required
  // Convert filename to string
  strcpy(file_name, stringfile.c_str());

  // Open binary file
  FILE * fp = fopen(file_name, "wb");
  // Write to file
  fwrite(&ADC[0], sizeof(int16_t), sampleNum, fp );
  // Close file
  fclose(fp);
  std::cout << "Binary file: " << stringfile << " created." << std::endl;

  // Get time at end as t2
  t2 = boost::chrono::high_resolution_clock::now();
  // Print simulation time
  std::cout << "HPGe Simulation computing time = " << 
    boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  //****************************
  //-----PLOTTING---------------
  //****************************
  double xx1[2]={0,1000};
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
  
  // Plot the voltage from preamp in green
  TGraph* grvoltage = new TGraph();
  // ADC counts from ADC in red
  TGraph* grADC = new TGraph();

  // Set the plot time range
  double timerangeplot = 5000; //us
  // Set the plot ADC range
  int ADCrangeplot = int(timerangeplot/tadc); //ADC Counts
  
  // Loop over all points
  for(long unsigned int i = 0; i < sampleNum; i++){
    // Add ADC points to TGraph
    grADC->SetPoint(i,i*tadc,ADC[i]);
  }

  // Loop over all points
  for(long unsigned int i = 0; i < sampleNum; i++){
    // Add voltage points to TGraph
    grvoltage->SetPoint(i,i*tadc,Voltage[i]/fehDriftFunctions.q0);
  }

  // Get TLatex
  std::string ratelatex = rate_stringkHz+" kHz";
  char* ratechar = const_cast<char*>(ratelatex.c_str());
  TLatex latex1;
  latex1.SetTextSize(0.05);
  latex1.DrawLatex((xx1[0]+xx1[1])/2,-1000,ratechar);

  // Set plot styles
  grvoltage->SetLineColor(kGreen+1);
  grvoltage->SetMarkerColor(kGreen+1);
  grvoltage->SetMarkerStyle(6);
  //grvoltage->Draw("same,p");
  grADC->SetLineColor(kRed+1);
  grADC->SetMarkerColor(kRed+1);
  grADC->SetMarkerStyle(6);
  grADC->Draw("same,p");

  // Update TCanvas with plot
  canvas->Modified();
  canvas->Update();
  //canvas->Print(".png");
#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)canvas->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif

} // End Sim Function

// Main function with arguments
int main(int argc, char *argv[]){

  // Photon rate
  std::string rate_stringkHz = std::string(argv[1]);
  // Photon energy (keV)
  double Energy = std::stod(argv[2]);
  // Noise standard deviation
  double noiseSD = std::stod(argv[3]);
  // The simulationn sample time
  double timeSample = std::stod(argv[4]);
  // The random seed
  unsigned int seedoption = std::stod(argv[5]);
  // Path to file
  std::string  output_path = std::string(argv[6]);

  // Set the random seed
  unsigned int seed;
  // If the provided seed is zero...
  if(seedoption==0){
    // Set the seed from the system clock
    seed = setseed_chrono();
  }
  // Else set the seed that is provided
  else{
    seed = setseed_man(seedoption);
  }

  // Define the random engine based on the seed
  std::default_random_engine gen(seed);

  // Call the simulation
  HPGe_ehModelSim(argc,argv,rate_stringkHz,Energy,noiseSD,timeSample,gen,output_path);

  return 0;
}
