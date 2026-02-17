#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

//#include "TGraph.h"
//#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include <TStopwatch.h>
using namespace std;

//const float T0 = 2.7;

int main(int narg, char* arg[]){
 
  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;
  //  auto c1= new TCanvas("c1","Title",400,10,1500,500);

  std::vector<int16_t> ADC;
  ADC.clear();
  // std::ifstream myFile(filename, ios::out | ios::binary);
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf ;
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
   
    ADC.push_back(inf);
  }

  std::cout << "Read data ... " << std::endl;

  /*  double xx1[2]={0,5000000};
  double yy1[2]={-2500,80000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,5000000);
  graph1->GetYaxis()->SetRangeUser(-2500,80000);
  graph1->SetTitle("Run00106-BinaryFile");
  graph1->GetXaxis()->SetTitle("Time (ns)");
  graph1->GetYaxis()->SetTitle("ADC Counts");

  TGraph* gr = new TGraph();
  */
  TStopwatch countt;
  countt.Start();

  std::vector<double> time;
  time.clear();
  double t=0;
  double T0=2.7;


  int n=ADC.size();

  cout<<ADC.size()<<endl;
  for(int i=0; i < n; i++){
    time.push_back(t);
    //gr->SetPoint(i,t,ADC.at(i));
    t+=T0;
  }

  //  graph1->Draw("ap");

  // gr->SetLineColor(kBlack);
  // gr->SetMarkerStyle(5);
  // gr->Draw("same");


  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////
  //TGraph* dec = new TGraph();
  //TGraph* dif = new TGraph();
  //TGraph* av = new TGraph();

  //bool plotMWD, plotEnergy;
  //int number;
  //cout<<"Enter 1 for plotting Signal+MWD, 2 for plotting Signal+EnergyPeaks "<<endl;
  //cin>>number;
  //plotMWD=false;
  //plotEnergy=false;
  //if(number==1){plotMWD=true;}
  //if(number==2){plotEnergy=true;}




  //Deconvolution
  double tau;

  //  tau=65544.1;
  tau=55748.2;

  vector<double> a;
  a.clear();

  a.push_back(ADC.at(0));
  //dec->SetPoint(0,time.at(0),a.at(0));
  for(int i=1; i<n; i++){
    a.push_back(ADC.at(i)-(1-(T0/tau))*ADC.at(i-1)+a.at(i-1));
    //cout<<a[i]<<endl;
    //dec->SetPoint(i,time.at(i),a.at(i));
  }

  //Differentiation

  vector<double> D;
  D.clear();
  int M=8000;

  for (int i = 0; i < M; ++i) {
    D.push_back(a.at(i));
    //dif->SetPoint(i,time.at(i),D.at(i));
  }

  for (int i = M; i < n; ++i) {
    D.push_back(a.at(i)-a.at(i-M));
    //dif->SetPoint(i,time.at(i),D.at(i));
  }

  //Averaging

  vector<double> l;
  l.clear();
  int L=1000;
  double sum = 0.;

  for (int i = 0; i < L-1; ++i) {
    sum += D.at(i);
    l.push_back(D.at(i));
    //cout<<l.at(i)<<endl;
    //av->SetPoint(i,time.at(i),l.at(i));
  }

  sum += D.at(L-1);
  l.push_back(sum/L);

  for (int i = L; i < n; ++i) {
    l.push_back(sum/L);
    //if(l.at(i)<0){cout<<l.at(i)<<endl;}
    sum += D.at(i)-D.at(i-L);
    //av->SetPoint(i,time.at(i),l.at(i));
  }


  //if(plotMWD==true){
    /*graph1->Draw("ap");

     gr->SetLineColor(kBlack);
     gr->SetMarkerStyle(5);
     gr->Draw("same");

     dec->SetMarkerColor(kRed);
     dec->SetLineColor(kRed);
     dec->SetMarkerStyle(5);
     dec->Draw("same");

     dif->SetMarkerColor(kGreen);
     dif->SetLineColor(kGreen);
     dif->SetMarkerStyle(5);
     dif->Draw("same");

     av->SetMarkerColor(kBlue);
     av->SetLineColor(kBlue);
     av->SetMarkerStyle(5);
     av->Draw("same");

     //Line at t=54756.2ns
     TLine *linet0=new TLine(54756.2 ,-1000,54756.2 ,1000);
     //linet0->Draw("same");
     //Line at t=79059.4 ns
     TLine *linet1=new TLine(79059.4 ,-1000,79059.4 ,1000);
     //linet1->Draw("same");

     TLine *linet2=new TLine(1.08015e+06 ,-2000,1.08015e+06 ,1000);
     //linet2->Draw("same");
     TLine *linet3=new TLine(1.10485e+06 ,-2000,1.10485e+06 ,1000);
     //linet3->Draw("same");

     TLine *linet4=new TLine(1.24839e+06 ,-2000,1.24839e+06 ,1000);
     //linet4->Draw("same");
     TLine *linet5=new TLine(1.27055e+06 ,-2000,1.27055e+06 ,1000);
     //linet5->Draw("same");


     auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
     leg1->AddEntry(gr, "Signal","p");
     //leg1->AddEntry(dec, "Deconvolution","p");
     leg1->AddEntry(dif, "M-Step Differentiation","p");
     leg1->AddEntry(av, "Moving Window Average","p");
     leg1->Draw("same");*/
  //} //if plotMWD








  ////////////////////////////////     Pulse Finding  //////////////////////////////////////////////////
  TH1F*h1 = new TH1F("TH1","", 100, -3000, 0);  
  //std::string rootname=filename.substr(filename.find("run"), 8)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
  std::string rootname=filename.substr(filename.find("run"), 8)+"_energypeaks_time.root";
  cout<<"new file created: "<<rootname<<endl;
  //Creo el .root en el que se van a guardar los picos del voltaje                                                                             
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");


  TTree*tree=new TTree("treeADC","treeADC");
  double peaks;
  tree->Branch("peaks",&peaks);
  //Run 109 and 110
  /* double mean = 354.307;
   double sigma=4.26976;
   double threesigmas= 4*sigma;*/


  //Run 75 and 106
  //media de los puntos azules que no forman parte de los picos
  //TH1F*h4 = new TH1F("TH4","", 500, 0, 500);
  //for(unsigned long j = 64000; j < 208000; ++j){
   //h4->Fill(l.at(j));}
  //Plot the baseline of the energy
  //double mean =h4->GetMean();
  //double sigma =h4->GetRMS();
  //double threesigmas= 4*sigma;
  //Run 106
  //  double mean =336.308;                                                                                                                   
  //double sigma=20;                                                                                                                       
  //double threesigmas= 4*sigma;
  //Run 50                                                                                        
  double mean =336.308;
  double sigma=50;
  double threesigmas= 4*sigma;
 
  cout<<"Mean signal "<<mean<<" Sigma: "<<sigma<<endl;
  



       double e2[8000], energy[8000];
       int counterpeak=0, auxlow=0;
       std::cout<<"Size l: "<<l.size()<<std::endl;
       
       for(unsigned long int i = 1000; i < l.size(); i++){
	
         if(l.at(i)<(mean-threesigmas)){
	   
	   if(l.at(i)<l.at(i-1)){auxlow=l.at(i);
             e2[counterpeak]=auxlow;
	     
             //continue;
	   }
	   else{continue;}
	 }//if
         if (auxlow==0){continue;}
         else if(l.at(i)>(mean-threesigmas)){
	   energy[counterpeak]=e2[counterpeak]-mean;
	   peaks=energy[counterpeak];
           tree->Fill();
	   h1->Fill(energy[counterpeak]);
	   std::cout<<"Peak "<<counterpeak<<" Energy: "<<peaks<<std::endl;
	   auxlow=0;
	   counterpeak++;
	   //continue;
	 }
    
       }// for int i

       countt.Stop();
       std::cout<<"Time: "<<std::endl;
       countt.Print();


    h1->Write();
    rootfile->Write();
    rootfile->Close();
    h1->Reset();


    myFile.close();
    //fclose(myFile);
    std::cout<<"Closing the root file"<<std::endl;

}

