#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include <TStopwatch.h>
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
    t+=T0;
  }

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////

  //Deconvolution
  double tau;

  //  tau=65544.1;
  tau=55748.2;

  vector<double> a;
  a.clear();

  a.push_back(ADC.at(0));

  for(int i=1; i<n; i++){
    a.push_back(ADC.at(i)-(1-(T0/tau))*ADC.at(i-1)+a.at(i-1));
  }

  //Differentiation

  vector<double> D;
  D.clear();
  int M=8000;

  for (int i = 0; i < M; ++i) {
    D.push_back(a.at(i));
  }

  for (int i = M; i < n; ++i) {
    D.push_back(a.at(i)-a.at(i-M));
  }

  //Averaging

  vector<double> l;
  l.clear();
  int L=1000;
  double sum = 0.;

  for (int i = 0; i < L-1; ++i) {
    sum += D.at(i);
    l.push_back(D.at(i));
  }

  sum += D.at(L-1);
  l.push_back(sum/L);

  for (int i = L; i < n; ++i) {
    l.push_back(sum/L);
    sum += D.at(i)-D.at(i-L);
  }




  ////////////////////////////////     Pulse Finding  //////////////////////////////////////////////////

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
  //Run 75                                                                                        
  double mean =336.308;
  double sigma=50;
  double threesigmas= 4*sigma;
 
  cout<<"Mean signal "<<mean<<" Sigma: "<<sigma<<endl;
  



       double e2[8000], energy[8000];
       int counterpeak=0, auxlow=0;
       double peaks;
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
           //tree->Fill();
	   //h1->Fill(energy[counterpeak]);
	   std::cout<<"Peak "<<counterpeak<<" Energy: "<<peaks<<std::endl;
	   auxlow=0;
	   counterpeak++;
	   //continue;
	 }
    
       }// for int i

       countt.Stop();
       std::cout<<"Time: "<<std::endl;
       countt.Print();

    
    myFile.close();
    //fclose(myFile);

}

