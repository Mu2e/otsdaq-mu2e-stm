#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <boost/chrono.hpp>
#include<cmath>

//#include "TTree.h"
//#include "TFile.h"
using namespace std;



int main(int narg, char* arg[]){

  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;


  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;
 


  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  t1 = boost::chrono::high_resolution_clock::now();
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf ;
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
   
    ADC.push_back(inf);
  }

  myFile.close();
  t2 = boost::chrono::high_resolution_clock::now();

  std::cout << "Read data ... " << std::endl;

  std::cout<<"Number of elements: "<<ADC.size()<< " in time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  t1 = boost::chrono::high_resolution_clock::now();


  //Creo el .root en el que se van a guardar los picos del voltaje
  /*std::string rootname=filename.substr(filename.find("run"), 8)+"_suppressedsignal_"+filename.substr(filename.find("bin"), 6)+".root";
  cout<<"new file created: "<<rootname<<endl;
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");

  TTree*tree=new TTree("treeADC","treeADC");
  int16_t ADCVolts;
  tree->Branch("ADCVolts",&ADCVolts);
  */

  //Creo el .bin en el que se van a guardar los picos del voltaje 
  std::ofstream output_file;
  std::string   output_filename=filename.substr(filename.find("run"), 8)+"_suppressedsignal_"+filename.substr(filename.find("bin"), 6)+".bin";
  cout<<"new file created: "<<output_filename<<endl;  
  output_file.open(output_filename, std::ios::out | std::ios::binary); 


 
  //Plotting the mean, base of the ADC counts
  /*TH1F*h2 = new TH1F("TH2","", 100, 0, 1000);
=======
  //Time interval sampling 2.7 ns
  //t=0-t=2.7*500000=1350000 ns
  for(int j = 0; j < 500000; ++j){
    h2->Fill(ADC.at(j));}
  double mean= h2->GetMean();
  double sigma= h2->GetRMS();

  double threesigmas=3*sigma;*/

  double summean=0;
  double nvalues=0;
  //Time interval sampling 2.7 ns
  //t=0-t=2.7*500000=1350000 ns
  //mean
  for(int j = 0; j < 500000; ++j){
    summean= summean+ADC.at(j);
    nvalues++;
  }
  double mean= summean/nvalues;
  double sumsigma= 0;
  //sigma
  for(int j = 0; j < 500000; ++j){
    sumsigma=sumsigma+(ADC.at(j)-mean)*(ADC.at(j)-mean);
  }
  double sigma=sqrt(sumsigma/nvalues);
  double threesigmas=3*sigma;
  cout<<"Threshold ADC: "<<mean<<"  Sigma: "<<sigma<<"  3*Sigma: "<<threesigmas<<endl;


  //-----------------------NON PEAKS SUPPRESSION ALGORITHM-----------------------------------------
 
  vector<double> time;
  vector<int16_t> y;
  time.clear();
  y.clear();
  double t=0;
  long unsigned int k=0;

  //All the sample
  while(k<ADC.size()){
   
    if(k+74074>ADC.size()){break;}
    if(ADC.at(k)<(mean-sigma)){
      //Store data back 20000 ns, forward 200000 ns --  h = 20000/2.7---200000/2.7
      for(int h=-7407;h<74074;h++){
        if(k+h>ADC.size()){break;}
	y.push_back(ADC.at(k+h));
	//Write ADC peaks to the root file
	//ADCVolts=ADC.at(k+h);
	//tree->Fill();
      }
      k=k+74074;
      if(k>ADC.size()){break;}
      //Add to have into account if the points after k+74074 are under mean-sigma
      while(ADC.at(k)<(mean-sigma)){
	y.push_back(ADC.at(k));
	//ADCVolts=ADC.at(k);
	//tree->Fill();
	k++;
      }
    }//if
    else{k++;}
   
  }//while ADC.size

  //Write ADC peaks to the binary file
  for (int16_t element : y){
    output_file.write((char *) &element, sizeof(element));
  }

  output_file.close();

  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "Zero Suppression Algorithm computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  std::cout<<"Number of elements in suppressed file: "<<y.size()<<std::endl;
  //rootfile->Write();
  //rootfile->Close();

}

