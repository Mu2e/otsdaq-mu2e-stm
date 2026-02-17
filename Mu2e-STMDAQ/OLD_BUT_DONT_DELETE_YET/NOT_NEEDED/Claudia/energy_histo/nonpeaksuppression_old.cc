#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
using namespace std;

//const float T0 = 2.7;

int main(int narg, char* arg[]){
 
  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf ;
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
   
    ADC.push_back(inf);
  }

  std::cout << "Read data ... " << std::endl;


  int n=ADC.size();

   //Creo el .root en el que se van a guardar los picos del voltaje
  std::string rootname=filename.substr(filename.find("run"), 8)+"_suppressedsignal_"+filename.substr(filename.find("bin"), 6)+".root";
  cout<<"new file created: "<<rootname<<endl;

  //Creo el .root en el que se van a guardar los picos del voltaje
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");


  TTree*tree=new TTree("treeADC","treeADC");
  int16_t ADCVolts;
  tree->Branch("ADCVolts",&ADCVolts);



  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);


  //Plotting the mean, base of the ADC counts
  TH1F*h2 = new TH1F("TH2","", 100, 0, 1000);
  //Time interval sampling 2.7 ns 
  //t=0-t=2.7*500000=1350000 ns
  for(int j = 0; j < 500000; ++j){
    h2->Fill(ADC.at(j));}
  double mean= h2->GetMean();
  double sigma= h2->GetRMS();
  double threesigmas=3*sigma;
  cout<<"Threshold ADC: "<<mean<<"  Sigma: "<<sigma<<"  3*Sigma: "<<threesigmas<<endl;


  //-----------------------NON PEAKS SUPPRESSION ALGORITHM-----------------------------------------
  //Los vectores a graficar son ADC.at(i) que son los elementos del vector del voltaje ADC y
  //time.at(i) que es el tiempo (intervalos de 2.7) al que ocurre cada voltaje

  vector<double> time;
  vector<int16_t> y;
  time.clear();
  y.clear();
  double t=0;
  int k=0;
  int low=0;
  int up=ADC.size()/10;
  //cout<<up<<endl;
  int i=0;


  //low=low+ADC.size()/10;
  //i=low;
  //up=up+ADC.size()/10;
  //cout<<low<<" "<<up<<endl;
  //Para toda la muestra
  while(k<ADC.size()){
    //cout<<k<<endl;
    
    //Miro si en ese intervalo hay algun valor por debajo de mean-threesigmas
    //Para toda la window
    while((i >= low)&&(i < up)){
      if(i+74074>ADC.size()){break;}
      if(ADC.at(i)<(mean-sigma)){
	//20000/2.7---200000/2.7
	//i=17364;
	for(int h=-7407;h<74074;h++){
	  y.push_back(ADC.at(i+h));
	  ADCVolts=ADC.at(i+h);
	  tree->Fill();
	}
	i=i+74074;
	//Add to have into account if the points after i+74074 are under mean-sigma
	while(ADC.at(i)<(mean-sigma)){
	  y.push_back(ADC.at(i));
	  ADCVolts=ADC.at(i);
	  tree->Fill();
	  i++;}
      }//if
      else{i++;}
    }//while up

    i=up;
    low=low+ADC.size()/10;
    up=up+ADC.size()/10;
    k =k+ADC.size()/10;

  }//while ADC.size
  rootfile->Write();
  rootfile->Close();



}

