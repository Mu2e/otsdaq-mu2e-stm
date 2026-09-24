#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

#include "TGraph.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TSystem.h"

#include "ehDriftFunctions.h"


void ehDriftPlot(double xmin, double xmax){
  ehDriftFunctions fehDriftFunctions;
  
  //Energy of the initial photon that we want to simulate in the event
  double energy=600; //keV
  //Sampling interval (plot induced charge Qpulse)
  //double interval=0.005;//us   
  double interval=1./(fehDriftFunctions.fADC);

  bool gif=false;
  int ngifs;
  if(gif==true){ngifs=50;}
  else {ngifs=1;}

  //Plot the peak
  gROOT->SetStyle("ATLAS");
  TCanvas *c1= new TCanvas();
  double xx1[2]={xmin,xmax};
  //double yy1[2]={-300000,1.5};
  double yy1[2]={-1500,100}; 
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

  //Plot the function in blue
  TGraph* grfunc = new TGraph();
  //Plot the voltage from preamp in green
  TGraph* grvoltage = new TGraph();
  //ADC counts from ADC
  TGraph* grADC = new TGraph();



  //Number of us to plot
  int sampleus=300; //us
  //Number of values to plot
  long unsigned int sampleNum=sampleus/interval;
  double* Qpulse = new double[sampleNum];
  double* Voltage = new double[sampleNum];
  double* timeQ = new double[sampleNum];
  
  //initialise Qpulse array to 0
  for(unsigned long int i=0; i<sampleNum;i++){
    Qpulse[i]=0;
  }


  
  //***************Detector (ADC) effects***************//
  //Number of ADC values to plot
  //long unsigned int sampleNum=sampleus*fehDriftFunctions.fADC;
  //Sampling time of ADC (microsec)                                                                 
  const double tadc=1./(fehDriftFunctions.fADC);


  // Intialise data array of ADC values
  int16_t* ADC = new int16_t[sampleNum];
  //Generate noise with random numbers following a gaussian, the input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
  double sigma_noise_ADC=fehDriftFunctions.noiseSD*38.5;
  std::cout<<"NoiseSD: "<<fehDriftFunctions.noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts"<<std::endl;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0,sigma_noise_ADC);
  
  //Generate the noise in ADC Counts
  for(unsigned long int i=0; i<sampleNum;i++){ 
    double noise=distribution(generator);
    ADC[i]=noise; 
    //ADC[i]=0;
  }
  //***************************************************//

  gSystem->Unlink("PulsesehModelplusCompt.gif");
  
  /////////////////////////////////////////////
  //total number of eh pairs in the whole event
  int Nehevent=0; 

  //h number of pictures in the gif   
  for(int g=0;g<ngifs;g++){
    //Generate the number of comptons and photoelectric effects according to geant4 simulation distributions
    int n_compt_phot= fehDriftFunctions.Number_compt_phot(energy);
    //int n_compt_phot=1; 
    
    std::cout<<"NEW EVENT: "<<g<<" with "<< n_compt_phot<<" comptons"<<std::endl;
    //Get the distribution of the energy for each compton
    std::vector<double> energiescomptons = fehDriftFunctions.Distribute_energies(energy, n_compt_phot);
    //Generate initial position
    std::vector<double> InitPos=fehDriftFunctions.PosActive();
    double r0;
    
    //Fill the peak
    for(int i=0;i<n_compt_phot;i++){
      //Get the energy and the number of eh pairs created
      double Ei=energiescomptons.at(i);
      int Nehi=fehDriftFunctions.Neh(Ei);
      Nehevent +=Nehi;
      //Calculate the positions for following comptons
      if(i==0){r0=InitPos.at(0);}
      else{r0=fehDriftFunctions.PosCompPhot(InitPos,Ei);}
      
      std::cout<<"---------New compton at: "<<r0<<" cm, energy: "<<Ei<<", Neh: "<<Nehi<<" ---------"<<std::endl;
      for (unsigned long int j = 1; j < sampleNum; j++){
	double time = j*interval;
	Qpulse[j] += Nehi*fehDriftFunctions.Q_tNtype(time, r0); 
	//std::cout<<time<<" "<<Qpulse[j]<<std::endl;
      }
    }//for ncompts

    if(gif==true){
      for(long unsigned int k=0;k<sampleNum;k++){
	grfunc->SetPoint(k,k*interval,Qpulse[k]/fehDriftFunctions.q0);
      }
      
      for(unsigned long int k=0; k<sampleNum;k++){
	Qpulse[k]=0;
      }

      grfunc->SetLineColor(kAzure+1);
      grfunc->SetMarkerColor(kAzure+1);
      grfunc->SetMarkerStyle(6);
      grfunc->Draw("same,p");
      c1->Modified();
      c1->Update();
    c1->Print("PulsesehModelplusCompt.gif+");
    sleep(1);
    }//if gif          
  }//for g
  
  

  if(gif==false){
  for(long unsigned int i=0;i<sampleNum;i++){
    grfunc->SetPoint(i,i*interval,Qpulse[i]/fehDriftFunctions.q0);
  }

  //shaping time is the first time where the induced charge is Nehxq0
  double ts=0; //in us
  //we have the charge induced in the detector (Qpulse) and we need to add the preamp shaping
  for(long unsigned int i=0;i<sampleNum;i++){
    
    timeQ[i]=i*interval;
    Voltage[i]=Qpulse[i];
    std::cout<<"Neh_event: "<<Nehevent<<std::endl;
    std::cout<<"time: "<<timeQ[i]<<" Q/q0: "<<Qpulse[i]/fehDriftFunctions.q0<<" V/q0: "<<Voltage[i]/fehDriftFunctions.q0<<std::endl; 

   
    //When Qpulse is equal to q0 times Neh, then shape it and plot the voltage from preamp
    if(Qpulse[i]==(-1)*(Nehevent*fehDriftFunctions.q0)){
      if(ts==0){ts=timeQ[i];std::cout<<"------Shaping time: "<<ts<<" us"<<std::endl;}
      else{Voltage[i]=fehDriftFunctions.HighPassFilter(timeQ[i],ts,Nehevent);
	std::cout<<"New V/q0: "<<Voltage[i]/fehDriftFunctions.q0<<std::endl;}
    }
    grvoltage->SetPoint(i,timeQ[i],Voltage[i]/fehDriftFunctions.q0);
    
  }


  //Plot ADC Counts from ADC using fADC for sampling
  for(long unsigned int i=0;i<sampleNum;i++){
    ADC[i] += fehDriftFunctions.preamptoADC(Voltage[i]/fehDriftFunctions.q0);
    grADC->SetPoint(i,i*tadc,ADC[i]);
    //std::cout<<"time: "<<i*tadc<<" V/q0: "<<Voltage[i]/fehDriftFunctions.q0<<" us, ADC: "<<ADC[i]<<std::endl;
 }



  grfunc->SetLineColor(kAzure+1);
  grfunc->SetMarkerColor(kAzure+1);
  grfunc->SetMarkerStyle(6);
  //grfunc->Draw("same,p");
  
  grvoltage->SetLineColor(kGreen+1);
  grvoltage->SetMarkerColor(kGreen+1);
  grvoltage->SetMarkerStyle(6);
  //grvoltage->Draw("same,p");

  grADC->SetLineColor(kRed+1);
  grADC->SetMarkerColor(kRed+1);
  grADC->SetMarkerStyle(6);
  grADC->Draw("same,p"); 



}//if gif false
  
}
