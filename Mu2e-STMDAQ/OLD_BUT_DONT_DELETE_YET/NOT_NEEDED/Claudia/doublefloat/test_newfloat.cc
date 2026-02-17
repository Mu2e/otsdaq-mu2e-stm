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

// hello a change

//#include "TTree.h"
//#include "TFile.h"
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

  float* time = new float[n];
  float t = 0;
  float T0 = 2.7;

  for(int i = 0; i < n; i++){
    time[i] = t;
    t+=T0;
  }

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////

  //Deconvolution
  float tau = 55748.2;
  float* a = new float[n];
  a[0] = ADC[0];

  for(int i=1; i<n; i++){
    a[i] = ADC[i]-(1-(T0/tau))*ADC[i-1] + a[i-1];
  }

  delete ADC;


  //Differentiation

  float* D = new float[n];
  int M=8000;

  memcpy( D, a, M*sizeof(a) );
    //  for (int i = 0; i < M; ++i) {
    // D[i] = a[i];
    // }

  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }

  delete a;

  //Averaging

  float* l = new float[n];
  int L = 1000;
  float sum = 0.;

  memcpy( l, D, (L-1)*sizeof(D) );
  for (int i = 0; i < L-1; ++i) {
    sum += D[i];
    //    l[i] = D[i];
  }

  sum += D[L-1];
  l[L-1] = sum/L;

  for (int i = L; i < n; ++i) {
    l[i] = sum/L;
    sum += D[i]-D[i-L];
  }

  delete D;

  //Run 109 and 110                                                                                                                                                                               
  // double mean = 354.307;
  //double sigma=4.26976;
  //double threesigmas= 4*sigma;

  //Run 106                                                                                                                    
  float mean =336.308;                                                                                                                            
  float sigma=20;                                                                                                    
  float threesigmas= 4*sigma;  
  //Run 75                       
  //double mean  = 336.308;
  //double sigma = 50;
  //double threesigmas = 4*sigma;

  float e2[8000], energy[8000];
  float auxlow = 0.;
  int counterpeak = 0;

  std::cout<<"Using FLOATS"<<std::endl;       
  for(unsigned long int i = 1000; i < n; i++){
    
    if (l[i] < (mean - threesigmas)){
	   
      if (l[i] < l[i-1]){
	  auxlow = l[i];
	  e2[counterpeak]=auxlow;
	  }
	else {
	  continue;
	}
    }

    if (auxlow == 0) {continue;}
    else if (l[i] > (mean-threesigmas)){
      energy[counterpeak] = e2[counterpeak]-mean;
      std::cout<<"min: "<<e2[counterpeak]<<" mean: "<<mean<<std::endl;
       std::cout << "Peak: "<<counterpeak<<" Energy: "<<energy[counterpeak]<<std::endl;
      auxlow=0.;
      counterpeak++;
    }
    
  }// for int i

  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "MWD + peak finder time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl; 
 
  
  //Write energy peaks in a root file

  /*   std::string rootname="run00106_energypeaks_nosup_float.root";
  cout<<"new file created: "<<rootname<<endl;
  //Creo el .root en el que se van a guardar los picos del voltaje                                                                                             
  TFile *rootfile=new TFile(rootname.c_str(),"recreate");
  TTree*tree=new TTree("treeADC","treeADC");
  double peaks;
  tree->Branch("peaks",&peaks);
  for(int i=0;i<counterpeak;i++){
    peaks=energy[i];
    tree->Fill();
  }
  
  rootfile->Write();
  rootfile->Close();*/


}
