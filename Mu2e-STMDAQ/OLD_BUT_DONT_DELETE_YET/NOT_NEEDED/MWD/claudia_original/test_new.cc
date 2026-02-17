#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <boost/chrono.hpp>
#include <sys/stat.h>

#include "TTree.h"
#include "TFile.h"
boost::chrono::milliseconds sumGlobal;

using namespace std;

int main(int narg, char* arg[]){
 
  std::string  filename  = std::string(arg[1]);
  std::cout << "filename = " << filename << std::endl;

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;
  boost::chrono::high_resolution_clock::time_point t3 ;

  struct stat st;
  stat(filename.c_str(), &st);
  int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;
  t1 = boost::chrono::high_resolution_clock::now();
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();
  t2 = boost::chrono::high_resolution_clock::now();

  std::cout << "Read data: # ADC values =  " << n << " in time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl; 


  t1 = boost::chrono::high_resolution_clock::now();

  double* time = new double[n];
  
  // double T0 = 2.7;

  // ADC samling frequency (Hz)
  const double fADC = 370*1e6;
  //const double fADC = 320.0520833313*1e6;
  //Sampling time of ADC (ns)
  const double T0 = (1/(fADC*1e-6))*1e3;

  for(int i = 0; i < n; i++){
    time[i]=i*T0;
  }

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////

  //Parameters
  double tau = 55748.2;
  //double tau = 50000;  
  int M=8000;
  //int M = 400;
  int L = 1000; 
  //int L = 200;

  //Deconvolution
  double* a = new double[n];
  a[0] = ADC[0];

  for(int i=1; i<n; i++){
    a[i] = ADC[i]-(1-(T0/tau))*ADC[i-1] + a[i-1];
  }

  delete ADC;


  //Differentiation

  double* D = new double[n];


  memcpy( D, a, M*sizeof(a) );
    //  for (int i = 0; i < M; ++i) {
    // D[i] = a[i];
    // }

  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }

  delete a;

  //Averaging

  double* l = new double[n];


  double sum = 0.;

  memcpy( l, D, (L-1)*sizeof(D) );
  for (int i = 0; i < L-1; ++i) {
    sum += D[i];
    //    l[i] = D[i];
  }

  sum += D[L-1];
  l[L-1] = sum/L;

  for (int i = L; i < n; ++i) {
    sum += D[i]-D[i-L];
    l[i] = sum/L;
    // std::cout<<"l= "<<l[i]<<std::endl;
  }

  delete D;

  //Run 109
  //for M=8000    
  double mean = 354.307;
  //for M=20000
  //double mean = 887.108;
  //for M=15000
  //double mean=665.766;
  //for M=5000
  //double mean=221.768;
  //for M=1000
  //double mean=44.336;
  //for M=2000
  //double mean=88.6859;
  //for M=800
  //double mean=35.4777;
  //for M=600
  //double mean=26.6149;
  //for M=500
  //double mean=22.1857;
  //for M=400
  //double mean=17.7;
  //for M=200
  //double mean=8.87635;
  //for M=100
  //double mean=4.76151;


  double sigma=4.3;
  double threesigmas= 4*sigma;

  //For simulated data
  //double mean = 0;
  //double threesigmas= 40;

 std::cout<<"Using mean "<<mean<<" threshold "<<threesigmas<<" M "<<M<<" L "<<L<<std::endl; 

  //Run 109                        
  //double mean = 354.307;
  //double sigma=4.26976;
  //double threesigmas= 4*sigma;

  //Run 110
  //double mean = 354.307;
  //double sigma = 10.9537;
  //double threesigmas= 4*sigma;

  //Run 106                                                                                                                    
  //double mean =336.308;        
  //double sigma=20;                                                                                                    
  //double threesigmas= 4*sigma;  
  
  //Run 75                       
  //double mean  = 336.308;
  //double sigma = 50;
  //double threesigmas = 4*sigma;

  
 //double e2[8000], energy[8000];
 double* e2 = new double[n];
 double* energy = new double[n];
 double auxlow = 4000.;
 int counterpeak = 0;
 double timeaux=0;

  for( int i = M; i < n; i++){
    
    if (l[i] < (mean - threesigmas)){
	   
      if ((l[i] < l[i-1])&&l[i]<auxlow){
	  auxlow = l[i];
	  //std::cout<<auxlow<<std::endl;
	  timeaux = time[i];
	  e2[counterpeak]=auxlow;
	  }
	else {
	  continue;
	}
    }

    if (auxlow == 4000) {continue;}
    else if (l[i] > (mean-threesigmas)){
      energy[counterpeak] = e2[counterpeak]-mean;
      //std::cout<<"min: "<<e2[counterpeak]<<" mean: "<<mean<<std::endl;
      std::cout << "Peak: "<<counterpeak<<" Time: "<<timeaux*1e-3<<" us"<<" Energy (ADC counts): "<<energy[counterpeak]<<" auxlow: "<<auxlow<<std::endl;
      auxlow=4000.;
      counterpeak++;
    }
    
  }// for int i


  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "MWD + peak finder time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl; 

  
  //Write energy peaks in a root file
  //Run 109
  //MWD
  std::string rootname="hi.root"; 
  //std::string rootname="/work/cgarcia/DATA/Claudia/MLParametersRun109/M100L100/"+filename.substr(filename.find("run"), 12)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
  //Suppression
  //std::string rootname="/work/cgarcia/DATA/Claudia/Suppression_Algorithm/M1000L500/"+filename.substr(filename.find("run"), 12)+"_energypeaks_"+filename.substr(filename.find("bin"), 6)+".root";
 
  
  //Simulation
  //std::string rootname="/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/M500L200/MWD"+filename.substr(filename.find("GendataNoise_"), 18)+"_Peaks.root";
  //std::string rootname="/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/MWD"+filename.substr(filename.find("SupdataNoise_"), 18)+".root";

  cout<<"new file created: "<<rootname<<endl;

  //Creo el .root en el que se van a guardar los picos del voltje
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");
  TTree*tree=new TTree("treeADC","treeADC");
  double peaks;
  tree->Branch("peaks",&peaks);
  for(int i=0;i<counterpeak;i++){
    peaks=energy[i];
    tree->Fill();
  }
  
  rootfile->Write();
  rootfile->Close();
  
  

}
