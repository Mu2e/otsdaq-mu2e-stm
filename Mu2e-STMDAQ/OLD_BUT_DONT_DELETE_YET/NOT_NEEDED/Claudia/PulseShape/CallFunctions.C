#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

#include "TGraph.h"
#include "TAxis.h"
#include "TROOT.h"

#include "Functions.h"


void CallFunctions(double xmin, double xmax){

  Functions fFunctions;

  //Number of us to plot
  int sampleus=500; //us
  //Number of ADC values to plot
  long unsigned int sampleNum=sampleus*fFunctions.fADC;
  //Sampling time of ADC (microsec)                                                                 
  const double tadc=1./(fFunctions.fADC);


  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  //Generate noise with random numbers following a gaussian, the input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC=fFunctions.noiseSD*38.5;
  std::cout<<"NoiseSD: "<<fFunctions.noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts, xshift= "<<fFunctions.xshift<<std::endl;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);
  
  //Generate the noise in ADC Counts
  for(unsigned long int i=0; i<sampleNum;i++){ 
    double noise=distribution(generator);
    ADC[i]=noise; 
  }

  //Generate the number of comptons and photoelectric effects taking place in the falling edge
  //according to geant4 simulation distributions
  //int n_compt_phot= fFunctions.Number_compt_phot(fFunctions.twiceA);
  int n_compt_phot= 1; 
  //vector with the timings of each process in the event
  std::vector<double> times_compt_phot = fFunctions.Times_compt_phot(n_compt_phot); //us
  //vector with the amplitudes of each process in the event
  std::vector<double> twiceAmp = fFunctions.Distribute_amplitudes(fFunctions.twiceA, n_compt_phot); //ADC Counts
  /////////////////////////////////////////////

  double startpeakus= fFunctions.xshift-2*fFunctions.lfall;//us (-Lfall)
  int startpeakADC= startpeakus*fFunctions.fADC;
  std::cout<<"Start peak: "<<startpeakADC<<" ADC, "<<startpeakus<<" us"<<std::endl;
  //Fill the peak
  for(int i=0;i<n_compt_phot;i++){
    std::cout<<"---------New compton starting at: "<< startpeakus+times_compt_phot.at(i)<<" us"<<std::endl;
  for (unsigned long int j = startpeakADC; j < sampleNum; j++){
    unsigned long int b=j+((times_compt_phot.at(i))*fFunctions.fADC);

    if(j== startpeakADC){std::cout<<b<<" ADC "<<startpeakus+times_compt_phot.at(i)<<" us"<<std::endl;
      std::cout<<"Twice the amplitude of the process: "<<twiceAmp.at(i)<<" ADC Counts"<<std::endl;}

    double time = j*tadc;

    // Generate pulse data with noise and add to ADC array
    //ADC[b] += fFunctions.pulseCalc_Joe(time,fFunctions.twiceA);
    //Uncomment this to get different amplitudes
    ADC[b] += fFunctions.pulseCalc_Joe(time,twiceAmp.at(i)); 

    //std::cout<<j<<" time: "<<time<<" compt: "<<i<<" ADC: "<<ADC[j]<<" at "<< b<<std::endl;
   


    //double time = j*tadc; 
    //ADC[j] += fFunctions.pulseCalc_Joe(time,fFunctions.twiceA); 

    // Having into account clipping
    // Baseline of simulated data is always around 0
    //if(ADC[j]>1000){
    //ADC[j]=-32768;
    //}  
  }

  }


  //Plot the peak
  gROOT->SetStyle("ATLAS");
  double xx1[2]={xmin,xmax};
  double yy1[2]={-1500, 500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Plot the function in blue
  TGraph* grfunc = new TGraph();

  for(long unsigned int i=0;i<sampleNum;i++){
    grfunc->SetPoint(i,i*tadc,ADC[i]);
  }

  grfunc->SetLineColor(kAzure+1);
  grfunc->SetMarkerColor(kAzure+1);
  grfunc->SetMarkerStyle(6);
  grfunc->Draw("same,p");


  //Plot one peak from data in black
  
  std::vector<int16_t> DataADC;
  DataADC=fFunctions.data_peak("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/Shapes/pulse10.bin");

  std::vector<int16_t> DataADCshift;
  //fill with noise to have the same shift
  for(int i=0;i<int((fFunctions.xshift-0.5)*fFunctions.fADC);i++){
    double noisedata=distribution(generator);
    DataADCshift.push_back(noisedata);}

  for(long unsigned int i=0;i<DataADC.size();i++){
    DataADCshift.push_back(DataADC.at(i));
  }


  TGraph* grdata = new TGraph();

  for(long unsigned int i=0;i<DataADCshift.size();i++){
    grdata->SetPoint(i,i*tadc,DataADCshift[i]);
  }

  grdata->SetLineColor(kBlack);
  grdata->SetMarkerColor(kBlack);
  grdata->SetMarkerStyle(6);
  grdata->Draw("same,p");
  

}
