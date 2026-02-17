#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include <sys/stat.h>

#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"

//boost::chrono::milliseconds sumGlobal;

using namespace std;

void UpdateMean(){
 
  std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/GendataNoise_05kHz.bin";
 
  std::cout << "filename = " << filename << std::endl;

 
  struct stat st;
  stat(filename.c_str(), &st);
  int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  //int n=5000000;
  int16_t* ADC = new int16_t[n];

  std::ifstream myFile;
 
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();
 

 


 

  double* time = new double[n];
  
  // double T0 = 2.7;

  // ADC samling frequency (Hz)
  const double fADC = 370*1e6;
  //Sampling time of ADC (ns)
  const double T0 = (1/(fADC*1e-6))*1e3;

  for(int i = 0; i < n; i++){
    time[i]=i*T0;
  }
  TGraph* gr=new TGraph();
  long int v=0;

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////

  //Deconvolution
  // double tau = 55748.2;
  double tau = 50000;  
  double* a = new double[n];
  a[0] = ADC[0];
  
  for(int i=1; i<n; i++){
    a[i] = ADC[i]-(1-(T0/tau))*ADC[i-1] + a[i-1];
  }

  delete [] ADC;


  //Differentiation

  double* D = new double[n];

  //int M=8000;
  int M=1000;

  memcpy( D, a, M*sizeof(a) );
    //  for (int i = 0; i < M; ++i) {
    // D[i] = a[i];
    // }

  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M];
  }

  delete [] a;

  //Averaging

  double* l = new double[n];

  //int L = 1000;
  int L = 500;

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

  delete [] D;

  //For simulated data
  //double mean = 0;
  //double threesigmas;

  int window=100000;
  double summean=0;
  double thresholdgrad=-0.1;
  int countermean=0;
  double mean=0;
  int k=M;
  double* gradient = new double[n];

  //Prueba histograma
  //TH1F *hmean = new TH1F("hsignal", "", 100, -1500, 100);
  TH1F *hmean = new TH1F("hsignal", "", 100, -10, 10);
  TH1F *hgrad = new TH1F("hgrad", "", 100, -10, 10);

  /*for(int i=0;i<n;i++){ 
  gr->SetPoint(i,time[i],l[i]);
  hmean->Fill(l[i]);
  }
    hmean->Draw("");
  */

  while(k<=window){
    gradient[k]=l[k+1]-l[k];

    if(gradient[k]<thresholdgrad){
      k=k+100*(M+L);
      continue;}
    else{
      hmean->Fill(l[k]);
      //gr->SetPoint(v,time[k],l[k]);
      hgrad->Fill(gradient[k]);
      summean=summean+l[k];
      countermean++;
      v++;
      k++;}
  }
 

  double meanh=hmean->GetMean();
  mean=summean/countermean;
 
  thresholdgrad=(-1)*hgrad->GetRMS();
  double threesigmas=40;
 

  std::cout<<"Using mean signal "<<mean<<" "<<meanh<<" stdevgrad: "<<thresholdgrad<<" threshold signal "<<threesigmas<<" M "<<M<<" L "<<L<<std::endl; 


 double* e2 = new double[n];
 double* energy = new double[n];
 double auxlow = 0.;
 int counterpeak = 0;
 int j=0;
 int countersub=0;
 double timeaux=0;

 

  for(int i = M; i < n; i++){

    //std::cout<<"mean: "<<mean<<std::endl;
    //threesigmas=mean+40;
    //Update the mean
    if((i>window)&&(i>j)){gradient[i]=l[i+1]-l[i];
      
      if(gradient[i]<(4*thresholdgrad)){j=i+100*(M+L);
	continue;}
      else{//mean = mean-(l[countersub]/countermean)+(l[i]/countermean);
	countersub++;
	summean = summean+l[i];
       
	mean=summean/(countermean+countersub);
	
	hmean->Fill(l[i]);
	hgrad->Fill(gradient[i]);
	thresholdgrad=(-1)*hgrad->GetRMS();
        meanh=hmean->GetMean();
	//std::cout<<"Using mean signal "<<mean<<" "<<meanh<<" stdevgrad: "<<thresholdgrad<<std::endl;
	
	//gr->SetPoint(v,time[i],l[i]);
	v++;
      }
      }

    //hmean->Fill(l[i]);  

    //Pulse Finding
    if (l[i] < (mean - threesigmas)){
         
      if ((l[i] < l[i-1])&&l[i]<auxlow){
	auxlow = l[i];
	timeaux = time[i];
	e2[counterpeak]=auxlow;
      }
      else {
	continue;
      }
    }

    if (auxlow == 0) {continue;}
    else if (l[i] > (mean-threesigmas)){
      energy[counterpeak] = e2[counterpeak]-mean;
      //std::cout<<"mean: "<<mean<<std::endl; 
      //std::cout<<"min: "<<e2[counterpeak]<<" mean: "<<mean<<std::endl;
      std::cout << "Peak: "<<counterpeak<<" Time: "<<timeaux*1e-3<<" us"<<" Energy (ADC counts): "<<energy[counterpeak]<<std::endl;
      auxlow=0.;
      counterpeak++;
    }
    
  }// for int i


  

  //hmean->Draw("");
  //gr->Draw("ap");
  //Write energy peaks in a root file

  
  /*std::string rootname="/work/cgarcia/DATA/Claudia/GenData/Noise/Raw1sdata_1kHz_noise_test_updatemean_th40.root";  
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
  */
  

}
