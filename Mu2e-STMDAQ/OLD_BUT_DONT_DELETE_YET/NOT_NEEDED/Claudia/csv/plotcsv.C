#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

#include "csvClass.h"
#include "TGraph.h"
#include "TCanvas.h"

void plotcsv() {
  csv_utils fcsv;
  //std::vector<int16_t> ADC = fcsv.read_csv_values("run00106.csv");
  std::vector<int16_t> ADC = fcsv.read_csv_values("/work/cgarcia/STMDAQ-TestBeam/Claudia/ELBE/stm_hzdr_raw_LaBr__run0000032_subrun00001.csv"); 

  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);
  double xx1[2]={0,20};
 
  double yy1[2]={-8000,4000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  //  graph1->GetXaxis()->SetRangeUser(-100000,5000000);
  //  graph1->GetYaxis()->SetRangeUser(-2000,8000);
  //  graph1->SetTitle("");
  //  graph1->GetXaxis()->SetTitle("Time (ns)");
  //  graph1->GetYaxis()->SetTitle("ADC Counts");


  //n tiene que ser igual al rango de la x
  int n=10000;

  //Plot a graph with time in x and a certain range of adc voltages to see the times at wich occur
  //the peaks

  TGraph* gr = new TGraph();
  //Los vectores aa graficar son prueba.at(i) que son los elementos del vector del voltaje ADC y
  //time.at(i) que es el tiempo (intervalos de 3.125) al que ocurre cada voltaje
  vector<double> time;
  time.clear();
  double t=0;
  const double fADC = 320.0520833313; //MHz
  const double tadc = (1./fADC); //us    
  //Range ADC voltage we want to plot
  //2 bytes 16 bits
  for(int j = 0; j < n; ++j)
    {
      time.push_back(t);
      gr->SetPoint(j,time.at(j),ADC.at(j));
      t+=tadc;
    }
  //cout<<ADC.size()<<" "<<time.size()<<endl;

  graph1->Draw("ap");
  gr->Draw("same,p");

}
