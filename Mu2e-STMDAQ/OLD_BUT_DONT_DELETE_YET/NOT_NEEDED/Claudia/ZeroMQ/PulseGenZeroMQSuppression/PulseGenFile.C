#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"


using namespace std;


void PulseGenFile() {

  double xx1[2]={0,900};
  double yy1[2]={-2000, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,900);
  graph1->GetYaxis()->SetRangeUser(-2000, 100);
  graph1->GetXaxis()->SetTitle("time (#mus)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->Draw("ap");

  
  TF1* f3 = new TF1("f3", "-([0] / (1 + exp(-(x - [1])*[2])) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x - [1])*[3]))))+[4]*sin(x)", 0,3000);

  f3->SetNpx(10000);
  //The amplitude of the pulse is the half of this value
  f3->SetParameter(0,2370);
  double twiceA=2370;
  //The x point in where we have the pulse (100 us)
  f3->SetParameter(1,50);
  double xshift=50;
  //fall from baseline time
  f3->SetParameter(2,6);
  double invtaufall=6;
  //decaytime(rise to baseline)
  f3->SetParameter(3,0.028);
  double invtaudecay=0.028;
  //Amplitude of noise
  f3->SetParameter(4,10);
  double Anoise=10;

  //Sampling time of ADC (microsec)
  double tadc=0.0027;
  std::cout<<"Sampling time ADC: "<<tadc<<std::endl;

  //Pulse duration
  double pulselength=xshift+220;
  std::cout<<"Pulse duration: "<<pulselength<<" microsec"<<std::endl;

  //Frequency is the inverse of the pulse duration 
  double pulsefreq=1000./(xshift+220);
  std::cout<<"Pulse frequency: "<<pulsefreq<<" kHz"<<std::endl;
  
  //Number of ADC values in one peak ([1]+220)/0.0027: (El pulso dura xshift+220 microsec de subida aprox). Si en 0.0027 microsec hay 1 ADC value
  //en la duracion del pulso habra n ADC values.
  unsigned long int n=(xshift+220)/tadc;
  std::cout<<"ADC values in one peak: "<<n<<std::endl;
  
  //Number of bytes in one peak
  unsigned long int nbytes=n*2;
  std::cout<<"Bytes in one peak: "<<nbytes<<std::endl;

  //Number of peaks in one event
  unsigned long int npeaks=1;
  std::cout<<"Number of peaks in one event: "<<npeaks<<std::endl;

  //Number of bytes per event
  unsigned long int byteschunk=npeaks*nbytes;
  std::cout<<"Bytes in one event: "<<byteschunk<<std::endl;
  
  //Number of events
  unsigned long nevents=2;
  std::cout<<"Number of events: "<<nevents<<std::endl;

  double* x = new double[n];
  //int16_t* data_element = new int16_t[n];
  vector<int16_t> data_element;
  int16_t ADC;

  TGraph* grADC = new TGraph();

  unsigned long counter;

  //Number of events
  for (unsigned long k=0;k<nevents;k++){
    //Number of peaks in one event
    for (unsigned long j=0;j<npeaks;j++){
      //Number of bytes in one peak
      for (unsigned long i=0;i<n;i++){
      x[i]=tadc*i;
      //ADC=-(2370 / (1 + exp(-(x[i] - 50)*6)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x[i] - 50)*0.028))))+10*sin(x[i]);
      ADC=-(twiceA / (1 + exp(-(x[i] - xshift)*invtaufall)) ) * ( 1.0  - ( 1.0/ (1 + exp(-(x[i] - xshift)*invtaudecay))))+Anoise*sin(x[i]);
      data_element.push_back(ADC);
      //Shows just one peak because the sampling time x[i] is repated, so it print all the pulses overlapping
      grADC->SetPoint(i,x[i],data_element.at(i));
      counter++;
      }
    }
  }
 

//Write events with peaks to a binary file
std::ofstream output_file;
std::string output_filename="pulsegen.bin";
output_file.open(output_filename, std::ios::out | std::ios::binary);  
for (int16_t element : data_element){
  output_file.write((char *) &element, sizeof(element));
 }

output_file.close();
f3->Draw("same");
grADC->Draw("p*");

}
