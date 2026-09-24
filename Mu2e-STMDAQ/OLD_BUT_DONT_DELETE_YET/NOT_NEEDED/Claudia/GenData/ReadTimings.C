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
#include "TLatex.h"
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



  int plot=1;


  std::string  filenameraw  = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/GendataNoise_80kHz.txt";
  std::cout << "filenameraw = " << filenameraw << std::endl;
  std::string  filenamesup  = "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/SuppresedFiles_tbefore2us_tafter10us/SupdataNoise_80kHz.txt";
  std::cout << "filenamesup = " << filenamesup << std::endl;

  std::ifstream myFileraw(filenameraw);
  std::ifstream myFilesup(filenamesup);
  // Make sure the files are open
  if(!myFileraw.is_open()) throw std::runtime_error("Could not open file");
  if(!myFilesup.is_open()) throw std::runtime_error("Could not open file");

  // Create a vector of doubles to store the timings result
  std::vector<double> timesraw, timesrawskip, timessup;
  double timeraw,timesup;

  const int MAX = 40;
  std::string lineraw, linesup;
  std::string valraw[MAX],valsup[MAX];
  int counterraw=0;
  int countersup=0;

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
      std::cout<<timeraw<<std::endl;
      //timeraw is the start time of the peaks provided by simulation, so we have to sum ~100microsec to get the peak times
      timesraw.push_back(timeraw+100);
     
      for(int i=3;i<4;i++){
        ss >> valraw[i];
	//std::cout<<valraw[i]<<std::endl;
      }


      counterraw++;
    }



  //SUPPRESSED timings 
  std::cout<<"----Suppressed Peaks----"<<std::endl;
  while(std::getline(myFilesup, linesup))
    {
      if(countersup==0||countersup==1||countersup==2){
        countersup++;
        continue;
      }
      // Create a stringstream from line
      std::stringstream ss(linesup);

      for(int i=0;i<5;i++){
        ss >> valsup[i];
	//std::cout<<valsup[i]<<" ";
      }
      ss >> timesup;
      std::cout<<timesup<<std::endl;

      timessup.push_back(timesup);
      //Exit when we read the last trigger time
      if(timesup==999993){break;}

      countersup++;
    }







  TGraph *grmissingpeaks= new TGraph();
  int missingtrigger=0;
  int notstoredpeaks=0;
  double difference,trigdifference;
  int tbefore=2; //us
  int tafter=10; //us
  double timerawold=timesraw.at(0);
 

  std::cout<<"Comparing timings"<<std::endl;
  for (unsigned long int i=0; i<timesraw.size(); i++){
    //If the difference between the two simulated peaks is less than 12 microsec, then the
    //zero suppression algorithm just shows one trigger but it is storing the next 10 microsec
    
    difference= timesraw.at(i)-timerawold;
    if(i>0){
      trigdifference=timessup.at(i-missingtrigger)-timessup.at(i-1-missingtrigger);}
    std::cout<<"Difference: "<<difference<<std::endl;
    std::cout<<"Trig difference: "<<trigdifference<<std::endl;

    //if(i>0&&difference<=(tbefore+tafter)){
    //A bit of margin
    if((i>0)&&(difference<11.89)){

      if(difference>tafter){notstoredpeaks++;}
      std::cout<<"---------------"<<timesraw.at(i)<<std::endl;
      missingtrigger++;
    }
    else{std::cout<<"---------------"<<timesraw.at(i)<<" "<<timessup.at(i-missingtrigger)<<std::endl;
      if(i>0){
	timerawold=timesraw.at(i);
	}
    }
   
}
  std::cout<<"Not stored peaks because they are between 10 and 12us from previous trigger "<<notstoredpeaks<<std::endl;






  if(plot==1){
    double xx1[2]={0, 1000000};
    double yy1[2]={0,1000000};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(0, 1000000);
    graph1->GetYaxis()->SetRangeUser(0,1000000);
    graph1->SetTitle("");
    graph1->GetXaxis()->SetTitle("Generated Time Raw Data [#mus]");
    graph1->GetYaxis()->SetTitle("Trigger Time Suppressed Data [#mus]");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");

    TGraph *grraw= new TGraph();
    TGraph *grsup= new TGraph();
    //for (unsigned long int i=0; i<timessup.size(); i++){grsup->SetPoint(i,i,timessup.at(i));
    //grraw->SetPoint(i,i,timesrawskip.at(i));
    // }  


    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"1 kHz");


    TGraph *grtimings= new TGraph(timessup.size(),&timesrawskip[0],&timessup[0]);
    grtimings->SetMarkerColor(kBlue-7);
    grtimings->SetMarkerStyle(7);
    grtimings->Draw("same,p");


    /*grraw->SetMarkerColor(kOrange-3);
    grraw->SetMarkerStyle(6);
    grsup->SetMarkerColor(kBlue-3);
    grsup->SetMarkerStyle(6);


    grraw->Draw("same,p");
    grsup->Draw("same,p");

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grraw,"Raw Data","p");
    legend->AddEntry(grsup,"Suppressed Data","p");
    legend->Draw("same");
    */
  }











  if(plot==2){
  double xx1[2]={0, 30};                                                                                                    
  double yy1[2]={0,50};                                                                                                         
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 30);
  graph1->GetYaxis()->SetRangeUser(0,50);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("PeakNo");
  graph1->GetYaxis()->SetTitle("Time(Peak[i]-Peak[i-1]) [#mus]"); 
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");                                                                                                           
   


 
  /*  grmissingpeaks->SetMarkerColor(kCyan-3);
  grmissingpeaks->Draw("same,p");

  TLine *l1 = new TLine(xx1[0],22,xx1[1],22);
  l1->SetLineColor(kBlack);
  l1->SetLineStyle(2);
  l1->Draw();

  TLine *l2 = new TLine(xx1[0],20,xx1[1],20);
  l2->SetLineColor(kBlack);
  l2->SetLineStyle(2);
  l2->Draw();

  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(l1,"#splitline{Range containing non stored peaks}{unless there's a trigger 2#mus after them}","l");
  legend->Draw("same");
  */
  
  }


}
