#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
//#include <boost/chrono.hpp>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h"

#include "TFile.h"
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

using namespace std;

void ReadTimings(){
  gROOT->SetStyle("ATLAS");



  double xx1[2]={0, 1000000}; 
  double yy1[2]={0,1000000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 1000000); 
  graph1->GetYaxis()->SetRangeUser(0,1000000);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Time Sup Data [#mus]");
  graph1->GetYaxis()->SetTitle("Time MWD Data [#mus]");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  


   

  //std::string  filenameraw  = "/work/cgarcia/DATA/Claudia/GenData/Noise/1sdata_1kHz_noise.txt";
  std::string  filenameraw  = "/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/GendataNoise_01kHz.txt"; 
 
  std::cout << "filenameraw = " << filenameraw << std::endl;
  //std::string  filenamemwd  = "/work/cgarcia/DATA/Claudia/GenData/Noise/MWD1sdata_1kHz_noisePeaks_th40.txt";
  //std::string  filenamemwd  = "/work/cgarcia/DATA/Claudia/GenData/Noise/MWD1sdata_1kHz_noisePeaks.txt"; 
  std::string  filenamemwd  = "/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/MWDGendataNoise_01kHz_Peaks.txt"; 

  std::cout << "filenamemwd = " << filenamemwd << std::endl;

  std::ifstream myFileraw(filenameraw);
  std::ifstream myFilemwd(filenamemwd);
  // Make sure the files are open
  if(!myFileraw.is_open()) throw std::runtime_error("Could not open file");
  if(!myFilemwd.is_open()) throw std::runtime_error("Could not open file");

  // Create a vector of doubles to store the timings result
  std::vector<double> timesraw, timesrawskip, timesmwd;
  double timeraw,timemwd;

  const int MAX = 40;
  std::string lineraw, linemwd;
  std::string valraw[MAX],valmwd[MAX];
  int counterraw=0;
  int countermwd=0;

  //RAW timings
  std::cout<<"----Raw Peaks----"<<std::endl;
  while(std::getline(myFileraw, lineraw))
    {
      if(counterraw==0||counterraw==1){
	counterraw++; 
	continue;}
     
      // Create a stringstream from line
      std::stringstream ss(lineraw);
     
      for(int i=0;i<3;i++){
	ss >> valraw[i];
	//std::cout<<valraw[i]<<" ";
      }
      ss >> timeraw;
      std::cout<<"raw "<<timeraw<<std::endl;
      //timeraw is the start time of the peaks provided by simulation, so we have to sum ~100microsec to get the peak
      timesraw.push_back(timeraw+100);
     
      for(int i=3;i<4;i++){
        ss >> valraw[i];
	//std::cout<<valraw[i]<<std::endl;
      }


      counterraw++;
    }



  //MWD timings 
  std::cout<<"----Mwd Peaks----"<<std::endl;
  while(std::getline(myFilemwd, linemwd))
    {
      if(countermwd==0||countermwd==1||countermwd==2||countermwd==999||countermwd==1000){
        countermwd++;
        continue;
      }
      // Create a stringstream from line
      std::stringstream ss(linemwd);

      for(int i=0;i<3;i++){
        ss >> valmwd[i];
	//std::cout<<valmwd[i]<<" ";
      }
      ss >> timemwd;
      std::cout<<"mwd "<<timemwd<<std::endl;
      timesmwd.push_back(timemwd);

      for(int i=3;i<7;i++){
        ss >> valmwd[i];
        //std::cout<<valmwd[i]<<" ";
      }

      countermwd++;
    }


  TGraph *grraw= new TGraph();
  TGraph *grmwd= new TGraph();
  TGraph *grmissingpeaks= new TGraph();
  int missingpeak=0;


  for (unsigned long int i=0; i<timesraw.size(); i++){
    
    if(i>=1&&(timesraw.at(i)-timesmwd.at(i-missingpeak)<-10 || timesraw.at(i)-timesmwd.at(i-missingpeak)>10)){
      std::cout<<i<<" Raw "<<timesraw.at(i)<<"      Difference: "<<timesraw.at(i)-timesmwd.at(i-missingpeak)<<std::endl;
      missingpeak++;
    }

    else{
      timesrawskip.push_back(timesraw.at(i));
      std::cout<<i<<" Raw "<<timesraw.at(i)<<" MWD "<<timesmwd.at(i-missingpeak)<<" Difference: "<<timesraw.at(i)-timesmwd.at(i-missingpeak)<<std::endl;}
   
    //grmwd->SetPoint(i,i,timesmwd.at(i));
    //grraw->SetPoint(i,i,timesrawskip.at(i));
   }

 

  for (unsigned long int i=0; i<timesmwd.size(); i++){
    std::cout<<i<<" Plotting Raw "<<timesrawskip.at(i)<<" MWD "<<timesmwd.at(i)<<" Difference: "<<timesrawskip.at(i)-timesmwd.at(i)<<std::endl;
} 


 TGraph *grtimings= new TGraph(timesmwd.size(),&timesrawskip[0],&timesmwd[0]);
 grtimings->SetMarkerColor(kBlue-7);
 grtimings->SetMarkerStyle(7);
 grtimings->Draw("same,p");
 


/*grraw->SetMarkerColor(kOrange-3);
  grraw->SetMarkerStyle(6);
  grmwd->SetMarkerColor(kBlue-3);
  grmwd->SetMarkerStyle(6);


  grraw->Draw("same,p");
  grmwd->Draw("same,p");

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(grraw,"Raw Data","p");
  legend->AddEntry(grmwd,"Mwd Data","p");
  legend->Draw("same");*/
}
