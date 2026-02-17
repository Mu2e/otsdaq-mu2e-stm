#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TH1.h"
#include "TTree.h"
#include "TFile.h"
#include <sys/stat.h>
using namespace std;



int main(int narg, char* arg[]){

  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;


  struct stat st;
  stat(filename.c_str(), &st);
  long unsigned int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)                       
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  std::cout << "Read data ... " << std::endl;

  std::cout<<"Number of elements: "<<n<<std::endl;


  //Creo el .root en el que se van a guardar los picos del voltaje
  std::string rootname=filename.substr(filename.find("run"), 8)+"_suppressedsignal_"+filename.substr(filename.find("bin"), 6)+".root";
  cout<<"new file created: "<<rootname<<endl;
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");

  TTree*tree=new TTree("treeADC","treeADC");
  int16_t ADCVolts;
  tree->Branch("ADCVolts",&ADCVolts);


  std::ofstream output_file;
  std::string   output_filename=filename.substr(filename.find("run"), 8)+"_suppressedsignal_"+filename.substr(filename.find("bin"), 6)+".bin";
  cout<<"new file created: "<<output_filename<<endl;  
  output_file.open(output_filename, std::ios::out | std::ios::binary); 


  //Plotting the mean, base of the ADC counts
  TH1F*h2 = new TH1F("TH2","", 100, 0, 1000);
  //Time interval sampling 2.7 ns
  //t=0-t=2.7*500000=1350000 ns
  for(int j = 0; j < 500000; ++j){
    h2->Fill(ADC[j]);}
  double mean= h2->GetMean();
  double sigma= h2->GetRMS();
  double threesigmas=3*sigma;
  cout<<"Threshold ADC: "<<mean<<"  Sigma: "<<sigma<<"  3*Sigma: "<<threesigmas<<endl;


  //-----------------------NON PEAKS SUPPRESSION ALGORITHM-----------------------------------------
  //Los vectores a graficar son ADC.at(i) que son los elementos del vector del voltaje ADC y
  //time.at(i) que es el tiempo (intervalos de 2.7) al que ocurre cada voltaje

  double* time = new double[n];
  vector<int16_t> y;
  y.clear();
  

  double t=0;
  long unsigned int k=0;

  //All the sample
  while(k<n){
    //std::cout<<"good "<<k<<std::endl;  
    if(k+74074>n){break;}
    if(ADC[k]<(mean-sigma)){
      //Store data back 20000 ns, forward 200000 ns --  h = 20000/2.7---200000/2.7
      for(int h=-7407;h<74074;h++){
        if(k+h>n){break;}
	y.push_back(ADC[k+h]);
	//Write ADC peaks to the root file
	ADCVolts=ADC[k+h];
	tree->Fill();
      }
      k=k+74074;
      if(k>n){break;}
      //Add to have into account if the points after k+74074 are under mean-sigma
      while(ADC[k]<(mean-sigma)){
	y.push_back(ADC[k]);
	ADCVolts=ADC[k];
	tree->Fill();
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



  rootfile->Write();
  rootfile->Close();

}

