#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
//#include <boost/chrono.hpp>
#include <sys/stat.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"


//#include "TTree.h"
//#include "TFile.h"
//boost::chrono::milliseconds sumGlobal;

using namespace std;

void Signal(std::string  filename, double xmin, double xmax, double ymin, double ymax, int M, int L, double tau){
  gROOT->SetStyle("ATLAS");

  //std::string  filename  = std::string(arg[1]);
  //std::string  filename  = "/work/cgarcia/DATA/MWD_Analysis/RUN109/run00109.new.bin_00";
  //std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/Noise/1sdata_1kHz_noise.bin"; 
  //std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/GendataNoise_05kHz.bin"; 
  //std::string  filename  = "/work/cgarcia/DATA/Claudia/GenData/NoiseZPMWD/SupdataNoise_05kHz.bin";  

  std::cout << "filename = " << filename << std::endl;

  auto c1= new TCanvas("c1");
  //c1->SetGrid();


  //double xx1[2]={1470000, 1472000};
  double xx1[2]={xmin , xmax};
  double yy1[2]={ymin, ymax};
  //double yy1[2]={600, 800};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  //graph1->GetXaxis()->SetRangeUser(1470000, 1472000);
  graph1->GetXaxis()->SetRangeUser(xmin, xmax);
  graph1->GetYaxis()->SetRangeUser(ymin,ymax);
  //graph1->GetYaxis()->SetRangeUser(600, 800);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  


  struct stat st;
  stat(filename.c_str(), &st);
  //int n=10000000;
  int n = 1500000;
  //unsigned long int n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)  
  //int n=370000000;
  int16_t* ADC = new int16_t[n];

  std::cout<<"file size in adc counts: "<<n<<std::endl;
  std::ifstream myFile;

  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  double* time = new double[n];


  //double T0 = 2.7;//ns
  // ADC samling frequency (MHz)
  //const double fADC = 370;
  const double fADC = 320.0520833313;
  //Sampling time of ADC (us)
  const double T0 = (1./(fADC));

  TGraph* gr = new TGraph();
  TGraph* dec = new TGraph();
  TGraph* dif = new TGraph();
  TGraph* av = new TGraph();

  for(int i = 0; i < n; i++){
    time[i] = T0*i;

    //ADC[i]=ADC[i]-920;//restore the signal to 0 for run109
  }

  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////

  //Deconvolution
  //double tau = 55748.2;
  //double tau = 50000;
  double* a = new double[n];
  a[0] = ADC[0];

  for(int i=1; i<n; i++){
    //std::cout<<"i*T0: "<<i*T0<<std::endl;
    //std::cout<<"ADC[i]: "<<ADC[i]<<std::endl;
    //std::cout<<"ADC[i-1]: "<<ADC[i-1]<<std::endl;
    //std::cout<<"(1-(1000*T0/tau)): "<<(1-(1000*T0/tau))<<std::endl;
    //std::cout<<"a[i-1]: "<<a[i-1]<<std::endl;
    //T0 to ns
    a[i] = ADC[i]-(1-(1000*T0/tau))*ADC[i-1] + a[i-1];
    //std::cout<<"result: "<<a[i]<<std::endl;
    //std::cout<<""<<std::endl;
}



  //Differentiation

  double* D = new double[n];

  memcpy( D, a, M*sizeof(a) );
  //  for (int i = 0; i < M; ++i) {
  // D[i] = a[i];
  // }

  for (int i = M; i < n; ++i) {
    D[i] = a[i] - a[i-M] ;
  }



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
    //std::cout<<"l= "<<l[i]<<std::endl;

  }

  //MEAN THRESHOLD MWD l[i] AND SIGNAL
  //Run 109
  /*TH1F *hsignal = new TH1F("hsignal", "", 100, 0, 100000);
  TH1F *hav = new TH1F("hav", "", 100, 0, 100000);
  //t=100000, t=200000ns
  //i=t/2.7
  for(int i=37000;i<74000;i++){
    hsignal->Fill(ADC[i]);
    hav->Fill(l[i]);
    }*/

  //Simulated data  
  /* TH1F *hsignal = new TH1F("hsignal", "", 100, -1000000, 1000000);
  TH1F *hav = new TH1F("hav", "", 100,-1000000, 1000000);
  //t=1000000, t=1500000 ns
  //i=t/2.7
  for(int i=int(1000000/T0);i<int(1500000/T0);i++){
    //for(int i=0;i<n;i++){  
    hsignal->Fill(ADC[i]);
    hav->Fill(l[i]);       
  }

 
  hav->SetLineColor(kBlue);
  hav->GetXaxis()->SetTitle("ADC Counts");
  hav->GetYaxis()->SetTitle("Entries");
  //hav->GetYaxis()->SetRangeUser(0,1100000);
  //hav->Draw();
  //TLine *lineth=new TLine(0,0,0,1100000);
  //lineth->SetLineColor(kBlack);
  //lineth->Draw("same");


  double meansignal=hsignal->GetMean();
  std::cout<<"Mean of the signal: "<<meansignal<<endl;
  double meanav=hav->GetMean();
  std::cout<<"Mean for MWD: "<<meanav<<endl;
  double sigmaav=hav->GetRMS();
  std::cout<<"St dev for MWD: "<<sigmaav<<endl;
  double distance=meansignal-meanav;
  */

  //PLOT GRADIENT MWD OUTPUT
  double* gradient = new double[n];
  bool inpeak=true;
  double* startpeak = new double[n];

  TGraph* grad = new TGraph();
  double thresholdgrad=-2; 
  double sumbasis=0;
  int b=0;
  int z=0;
  //we use M+L to skip the M first points of the MWD output where the gradient goes crazy because the differentiation step is equal to the deconvolution step and L to skip the first L points where the average is taken.
  for(int i=M+L;i<n;i++){
    gradient[i]=l[i+1]-l[i];
    //std::cout<<"gradient: "<<gradient[i]<<std::endl;
    grad->SetPoint(b,time[i],gradient[i]);
    b++;
    //First value where the mwd output is below the threshold of the gradient
    if((gradient[i]<thresholdgrad)&&(inpeak==true)){
      //std::cout<<time[i]<<" "<<gradient[i]<<std::endl;
      startpeak[z]=time[i];

      //skip first L points after the condition
      double startpeakbasis=startpeak[z]+L*T0;
      //std::cout<<"The basis starts in: "<<startpeak[z]<<" us"<<std::endl;
      //std::cout<<"Numbers taken to calculate the average: "<<std::endl;
      //give it 0.1 us of marging when taking the data to average
      double margin=0.1;
      double margin_index=margin/T0;
      //convert the number to index
      int indexstart= int(startpeakbasis/T0)+margin_index;
      sumbasis=0;
      //Take the average with the M-L next points
      for(int j=indexstart;j<(indexstart+M-L-2*margin_index);j++){
	sumbasis+= l[j];
	//std::cout<<"time: "<<j*T0<<" ADC value: "<<l[j]<<std::endl;
      }
      
      double meanMWDoutput=sumbasis/(M-L-2*margin_index);
      //std::cout<<"Mean value of the peak: "<<meanMWDoutput<<std::endl;
      //std::cout<<"The height of the pulse: "<<meanMWDoutput-44.336<<std::endl;
      inpeak=false;
      z++;}
    if(gradient[i]>(-1)*thresholdgrad){inpeak=true;}
  }

  double x1=2000000;
  double x2=2300000;
  double y1=-10;
  double y2=10;
  //grad->GetXaxis()->SetRangeUser(x1, x2);
  //grad->GetYaxis()->SetRangeUser(y1, y2);
  grad->GetXaxis()->SetTitle("Time [ns]");
  grad->GetYaxis()->SetTitle("ADC Counts");
  grad->SetLineColor(kOrange);
  grad->SetMarkerColor(kOrange);
  //grad->Draw("al");
  //grad->Draw("same,l");  

  TLine *start =new TLine(startpeak[0],yy1[0],startpeak[0],yy1[1]);
  start->SetLineColor(kViolet-6);
  //start->Draw("same");

  /*TLine *linethresholdgrad=new TLine(x1,thresholdgrad,x2,thresholdgrad);
  linethresholdgrad->SetLineColor(kRed-6);
  linethresholdgrad->Draw("same");

  TLine *linepeak=new TLine(2.0338e+06,y1,2.0338e+06,y2);
  linepeak->SetLineColor(kViolet-6);
  //linepeak->Draw("same");
  TLine *linepeakwidth=new TLine(2.0338e+06+((M+L)*T0),y1,2.0338e+06+((M+L)*T0),y2);
  linepeakwidth->SetLineColor(kViolet-6);
  //linepeakwidth->Draw("same");
   
*/

  //PLOT GRAPHS MWD
  for(int i=0;i<n;i++){
    gr->SetPoint(i,time[i],ADC[i]);
    dec->SetPoint(i,time[i],a[i]);
    dif->SetPoint(i,time[i],D[i]);
    // av->SetPoint(i,time[i],l[i]+3500);
    av->SetPoint(i,time[i],l[i]-10000);  
    //dec->SetPoint(i,time[i],a[i]-1000);
    //dif->SetPoint(i,time[i],D[i]-6000);
    //av->SetPoint(i,time[i],l[i]-8000);
    //av->SetPoint(i,time[i],l[i]+distance);
   
  }

  /*int n1=32675699;

  for(int i=32651279;i<n1;i++){
    gr->SetPoint(i-32651279,time[i],ADC[i]);
    av->SetPoint(i-32651279,time[i],l[i]);
    }*/


 



  //gr->SetMarkerColor(kBlack);
  //gr->SetLineColor(kBlack);
  gr->SetMarkerColor(kViolet+2);
  gr->SetLineColor(kViolet+2);
  gr->SetMarkerStyle(5);
  //gr->Draw("same, l");
  //gr->Draw("same, p");   
  
  /*gr->SetMarkerColor(kPink);
  gr->SetLineColor(kPink); 
  gr->SetMarkerStyle(1);
  gr->Draw("same, l");
  */
  dec->SetMarkerColor(kRed);
  dec->SetLineColor(kRed);
  dec->SetMarkerStyle(5);
  dec->Draw("same");

  dif->SetMarkerColor(kGreen);
  dif->SetLineColor(kGreen);
  dif->SetMarkerStyle(5);
  //dif->Draw("same, l");

  av->SetMarkerColor(kBlue);
  av->SetLineColor(kBlue);
  av->SetMarkerStyle(5);
  av->Draw("same, l");

  gr->Draw("same, l");

  TLine *l1=new TLine(xx1[0],337.107,xx1[1],337.107);
  l1->SetLineColor(kRed);
  //l1->Draw("same");

  TLine *l2=new TLine(xx1[0],-40,xx1[1],-40);
  l2->SetLineColor(kRed);
  //l2->Draw("same");



  TLine *linethresholdgrad=new TLine(xx1[0],thresholdgrad,xx1[1],thresholdgrad);
  linethresholdgrad->SetLineColor(kRed-6);                                                                                 
  //linethresholdgrad->Draw("same");
  /*TLine *linepeak=new TLine(2.0338e+06,yy1[0],2.0338e+06,yy1[1]);
  linepeak->SetLineColor(kViolet-6);
  linepeak->Draw("same");                                                                                                  
  TLine *linepeakwidth=new TLine(2.0338e+06+((M+L)*T0),yy1[0],2.0338e+06+((M+L)*T0),yy1[1]);
  linepeakwidth->SetLineColor(kViolet-6);
  linepeakwidth->Draw("same");
  
  TLatex latex;
  latex.SetTextSize(0.06);
  latex.DrawLatex(2020000,-9,"M+L");
  */

  /*TLine *line=new TLine(xx1[0],mean,xx1[1],mean);
  line->SetLineColor(kRed);
  line->Draw("same");
  TLine *line2=new TLine(xx1[0],mean+4*sigma,xx1[1],mean+4*sigma);
  line2->SetLineColor(kBlack);
  line2->Draw("same");
  TLine *line3=new TLine(xx1[0],mean-4*sigma,xx1[1],mean-4*sigma);
  line3->SetLineColor(kBlack);
  line3->Draw("same");*/


  //Tau Value
  /*TF1*Fit1 = new TF1("Fit1", "[0]*exp(([1]-x)/[2])", 960, 1250);
    Fit1->SetParameters(-1185,959.5,50000);
    gr->Fit(Fit1,"0","",960,1250);
    Fit1->SetLineColor(kRed);
    Fit1->SetLineStyle(2);
    Fit1->Draw("same");
  */

  //Falling edge
  /*TF1*Fit1 = new TF1("Fit1", "[0]*exp(-x/[1])+[2]", 2219.500, 2219.79);
  Fit1->SetParameters(-2.85e10,200,-432628);
  gr->Fit(Fit1,"0","",2219.500, 2219.79);
  Fit1->SetLineColor(kRed);                                                                                                              
  Fit1->SetLineStyle(2);                                                                                                                 
  Fit1->Draw("same");  
  */


  delete [] ADC;
  delete [] a;
  delete [] D;

  //Find time rise
  /*TLine *line4=new TLine(1470740,yy1[0],1470740,yy1[1]);
  line4->SetLineColor(kRed);
  line4->Draw("same");
  TLine *line5=new TLine(1471120,yy1[0],1471120,yy1[1]);
  line5->SetLineColor(kRed);
  line5->Draw("same");*/

  int tau_mus = tau/1000;
  std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L)+", #tau="+std::to_string(int(tau))+"ns";  
  //std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L);
  char* MLchar = const_cast<char*>(ML.c_str());

  TLatex latex1;
  latex1.SetTextSize(0.05);
  latex1.DrawLatex(xmin+0.6,ymax-25000,MLchar);
  //latex1.DrawLatex(27,0.05,"80 kHz");

  auto legend = new TLegend(0.62,0.2,0.89,0.45);
  //legend->AddEntry(line4,"t_{R}#approx380 ns","l");
  //legend->AddEntry(Fit1,"#tau#approx49.9 #mus","l");
  //legend->AddEntry(Fit1,"#tau#approx0.19 #mus","l");
  //legend->AddEntry(gr,"Simulated Data","l");  
  legend->AddEntry(gr,"LaBr_{3} Data","l");
  //legend->AddEntry(gr,"Raw Data","l");
  //legend->AddEntry(dec,"Deconvolution (-1000 ADC)","l");
  //legend->AddEntry(dif,"M-Step Differentiation (-6000 ADC)","l");  
  //legend->AddEntry(av,"MWD Output (-8000 ADC)","l");
  //legend->AddEntry(gr,"Raw Data","l");
  //legend->AddEntry(dec,"Deconvolution","l");
  //legend->AddEntry(dif,"M-Step Differentiation","l");
  legend->AddEntry(av,"MWD Output","l");
  //legend->AddEntry(l1,"Threshold","l");
  //legend->AddEntry(hav,"MWD output array","f"); 
  //legend->AddEntry(lineth,"MWD mean","l");
  //legend->AddEntry(grad,"Gradient MWD output","l");
  //legend->AddEntry(linethresholdgrad,"Gradient threshold","l");
  //legend->AddEntry(start,"Gradient below threshold","l");
  legend->Draw("same");
  
  c1->Print("LaBrELBE_M7_L3_peak.png");

}
