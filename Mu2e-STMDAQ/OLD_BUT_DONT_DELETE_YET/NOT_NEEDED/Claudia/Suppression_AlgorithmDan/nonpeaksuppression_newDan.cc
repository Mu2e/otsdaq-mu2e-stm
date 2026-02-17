#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <boost/chrono.hpp>



#include "TF1.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TROOT.h"
#include "TGraph.h"
#include "TLine.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TLatex.h"

//#include "TTree.h"
//#include "TFile.h"
//using namespace std;

#define LINE_LENGTH 10000
//#define WF_LENGTH 200000
//#define PRE 1850 //ADC values= 5us using 370MHz
//#define POST 3700 //ADC values= 10us using 370MHz 
#define PRE 640 //ADC values= 2us using 320MHz                                                                           
#define POST 3200 //ADC values= 10us using 320MHz      
#define WINDOW_LEN 128
#define THRESHOLD 2


/*#define LINE_LENGTH 10000
//#define WF_LENGTH 200000
#define PRE 2000
#define POST 150000
#define WINDOW_LEN 128
#define THRESHOLD 3    
*/


/*#define LINE_LENGTH 10000
//#define WF_LENGTH 200000
#define PRE 2000
#define POST 150000
#define THRESHOLD 4
#define WINDOW_LEN 128
*/


int main(int argc, char* argv[]){


  //***************************************************************************
  //*******************CONFIG PARAMETERS ZP HZDR*******************************
  //***************************************************************************

  bool printcout =false;
  bool condition =false;
  bool signalsup =true;

  //***************************************************************************

  //std::cout<<"Running nonpeaksuppression_newDan.cc"<<std::endl;
  std::cout<<"Running "<<argv[0]<<std::endl ;

  std::string  filename  = std::string(argv[1]);
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



#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  // This code will just create the png
  TCanvas* c = new TCanvas();


  //double fadc=370; //MHz
  double fadc=320.0520833313; //MHz  
  double tadc=1.0/fadc; //us
  int n=ADC.size();
  //int n=100000;

  //Make th window length equal to the file size in ADC counts
  int WF_LENGTH=n;

  std::cout << "Read data ... " << std::endl;

  std::cout<<"Number of elements: "<<ADC.size()<< " in time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  std::cout<<"Uisng PRE: "<<PRE<<" POST: "<<POST<<" WINDOW_LEN: "<<WINDOW_LEN<<" THRESHOLD: "<<THRESHOLD<<" fadc: "<<fadc<<std::endl;
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
  //std::string   output_filename="/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.17mV/SuppresedFiles_PRE2us_POST10us_HZDR/Sup"+filename.substr(filename.find("dataNoise"), 19);
  std::string  output_filename="hi.bin";
  std::cout<<"new file created: "<<output_filename<<std::endl;  
  output_file.open(output_filename, std::ios::out | std::ios::binary); 


  

  //-----------------------NON PEAKS SUPPRESSION ALGORITHM-----------------------------------------
 
  //Store peaks
  std::vector<int16_t> supdata;
  supdata.clear();
  std::vector<double> suptime;
  std::vector<double> meanv;
  std::vector<double> variancev;
  std::vector<double> heightv;


  int64_t last_trigger = -PRE - POST - 1;
  int64_t trigger=-PRE - POST - 1;
  std::vector<int64_t> storetrigger;
  int ntriggers=0;


  int16_t waveform[WINDOW_LEN + 2];
  int16_t new_values[3];


  //Mean and variance for the first 130 values
  int i=0;
  while(i<(WINDOW_LEN+2))
    {

      for (int j = 0; j < 2; j++) {
	new_values[j] = new_values[j + 1];
      }
    
      new_values[2] = ADC.at(i);
      if(printcout==true){
	std::cout<<"i= "<<i<<std::endl;
	std::cout<<"[0] "<<new_values[0]<<std::endl;
	std::cout<<"[1] "<<new_values[1]<<std::endl;
	std::cout<<"[2] "<<new_values[2]<<std::endl;
      }
      waveform[i]=ADC.at(i);
      i++;
    }

  //mean
  double mean=0;
  for (int i = 0; i < WINDOW_LEN; i++) {
    //std::cout<<"Wave form: "<<waveform[i]<<endl;
    mean += waveform[i];
  }
  mean /= WINDOW_LEN;
  
  //variance
  double variance=0;
  for (int i = 0; i < WINDOW_LEN; i++) {

    variance += (waveform[i] - mean) * (waveform[i] - mean);
    //std::cout<<"ADC value "<<waveform[i]<<" MEAN "<<mean<<" VARIANCE "<<variance<<endl;
  }
  variance /= WINDOW_LEN;


  int window=1;
  int readsize=WINDOW_LEN + 2;


  int counter=0;
  int counter2=0;


  //Read all file as 1 window

  //for (int i = WINDOW_LEN + 2; i < WF_LENGTH + PRE + 2; i++) {
  for (int i = WINDOW_LEN + 2; i < n; i++) {
    for (int j = 0; j < 2; j++) {
      new_values[j] = new_values[j + 1];
    }
    if(printcout==true){
      std::cout<<"window number "<<window<<std::endl;
    }
    //i<1019+256+2 //Leo primera window, en el siguiente loop leo siguiente window...
    if (i < window*WF_LENGTH) {
      //if (i < window*(WF_LENGTH+ PRE + 2)) {
    new_values[2]=ADC.at(i);
    }
    if(printcout==true){
    std::cout<<"/////////////counter "<<counter<<" i= "<<i<<std::endl;
    }

    meanv.push_back(mean);
    variancev.push_back(variance);

    int16_t out_value;
    if (i >= PRE + 2){
      out_value = ADC.at(i - PRE - 2); //Primer valor de ADC en la window en la que guardamos data
    }
    int16_t old_value = ADC.at(i - WINDOW_LEN - 2); //Primer valor de ADC


    double old_mean = mean;
    double height = new_values[0] - mean;

    heightv.push_back(height);

    if(printcout==true){
    std::cout<<"mean "<<mean<<std::endl;
    std::cout<<"[0] "<<new_values[0]<<std::endl;
    std::cout<<"[1] "<<new_values[1]<<std::endl;
    std::cout<<"[2] "<<new_values[2]<<std::endl;
    std::cout<<"height: "<<height<<std::endl;
    std::cout<<"height*height: "<<height*height<<std::endl;
    std::cout<<"variance*threshold^2: "<<variance * THRESHOLD * THRESHOLD<<std::endl;
    std::cout<<"variance: "<<variance<<std::endl;
    }
    //trigger on rising edge
    if ((height*height > variance * THRESHOLD * THRESHOLD) &
        //que el last trigger no este en la window [i-PRE-2,i+POST+2]
        (height <= 0)
	&(new_values[2] < new_values[1])
	&(new_values[1] < new_values[0])
	&(last_trigger < i - PRE - POST))
      {
	last_trigger = i;
      }


    if(trigger!=last_trigger){
      storetrigger.push_back(last_trigger);
      trigger=last_trigger;
      ntriggers++;
    }


    if(printcout==true){
    std::cout<<"last trigger "<<last_trigger<<std::endl;
    }
    //store values
    double time;
    int16_t data;

    if ((i >= PRE + 2) & (last_trigger > (i - PRE - POST))) {
      //window time trigger-PRE-2, trigger+POST-2
      time = (i - PRE - 2);
      //Stored data
      data = out_value;
      supdata.push_back(data);
      suptime.push_back(time);
      //ADCVolts=data;
      //tree->Fill();
      counter2++;
      //std::cout<<"time "<<time<<" data: "<<data<<endl;

    }


    //std::cout<<"Old value: "<<old_value<<std::endl;
    //std::cout<<"New value: "<<new_values[0]<<std::endl;
    //add to the mean the new value and subtract the first one summed to the mean
    mean = mean+(new_values[0] - old_value)/128.0;
  

    //add to the sum for the variance the new value and subtract the old one
    variance += (new_values[0] - old_value) *(new_values[0] + old_value - mean - old_mean) / 128.0;
    //variance += (((new_values[0] - mean)*(new_values[0] - mean))-((old_value-old_mean)*(old_value-old_mean)))/ WINDOW_LEN;


    //std::cout<<" Newmean: "<<mean<<" Newvariance: "<<variance<<endl;

    counter++;
    readsize++;
    if(readsize==window*WF_LENGTH){window++;}
    //if(readsize==window*(WF_LENGTH+ PRE + 2)){window++;}
  }//for int i window length+2

 
    for (int i=0;i<ntriggers;i++){
      std::cout<<"Trigger "<< i <<" in: "<<storetrigger.at(i)<<" clock ticks "<<storetrigger.at(i)*tadc<<" us"<<std::endl;
    }









  //Write ADC peaks to the binary file
  for (int16_t element : supdata){
    output_file.write((char *) &element, sizeof(element));
  }

  output_file.close();
  t2 = boost::chrono::high_resolution_clock::now();
  std::cout << "Zero Suppression Algorithm computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  std::cout<<"Number of elements in suppressed file: "<<supdata.size()<<std::endl;


  //rootfile->Write();
  //rootfile->Close();





   if(condition==true){
     double xminADC= 1933760; //clock ticks
     double xmaxADC= 1933780; //clock ticks
     double xminus= 1100; //us
     double xmaxus= 900; //us 
       

    double xx1[2]={xminus, xmaxus};
    double yy1[2]={500, 10000};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
    //graph1->GetXaxis()->SetTitle("Time [#times2.7ns]");
    graph1->GetXaxis()->SetTitle("Time [#mus]");
    graph1->GetYaxis()->SetTitle("ADC Counts");
    graph1->SetTitle("");
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");

    //Raw Signal
    TGraph* grADC = new TGraph();
    //Variance
    TGraph* grvariancethreshold = new TGraph();
    //Height
    TGraph* grheight = new TGraph();

    int limitplot=2240000;
 

    for(int i=0;i<limitplot;i++){
      grADC->SetPoint(i,(i+WINDOW_LEN+2)*tadc,ADC.at(i+WINDOW_LEN));
      grvariancethreshold->SetPoint(i,(i+WINDOW_LEN+2)*tadc,variancev.at(i)*THRESHOLD*THRESHOLD);
      grheight->SetPoint(i,(i+WINDOW_LEN+2)*tadc,heightv.at(i)*heightv.at(i));
      if(heightv.at(i)*heightv.at(i)>5000){
	std::cout<<"signal (new_value[0]): "<<ADC.at(i+WINDOW_LEN)<<" variance: "<<variancev.at(i)<<" variance*th2: "<<variancev.at(i)*THRESHOLD*THRESHOLD<<" height2: "<<heightv.at(i)*heightv.at(i)<<" clock tick: "<<i+WINDOW_LEN+2<<" time: "<<(i+WINDOW_LEN+2)*tadc<<" us"<<std::endl;}
    }
   
    grADC->SetMarkerColor(kBlack);
    grADC->SetLineColor(kBlack);
    grADC->SetMarkerStyle(6);
    grADC->Draw("lp*,same");

    grvariancethreshold->SetMarkerColor(kGreen);
    grvariancethreshold->SetLineColor(kGreen);
    grvariancethreshold->SetMarkerStyle(6);
    grvariancethreshold->Draw("lp*,same");


    grheight->SetMarkerColor(kRed+2);
    grheight->SetLineColor(kRed+2);
    grheight->SetMarkerStyle(6);
    grheight->Draw("lp*,same");


    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grADC,"Data","l");
    legend->AddEntry(grvariancethreshold,"(3#sigma)^{2}","l");
    legend->AddEntry(grheight,"h^{2}","l");
    legend->Draw("same");

    }


 
  if(signalsup==true){

    double xminADC= 0; //clock ticks
    double xmaxADC= 4000000; //clock ticks
    double xminus= 999200; //us
    double xmaxus= 1000000; //us   


    double xx1[2]={xminus,xmaxus};
    double yy1[2]={-14000, 100};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
    //graph1->GetXaxis()->SetTitle("Time [#times2.7ns]");
    graph1->GetXaxis()->SetTitle("Time [#mus]"); 
    graph1->GetYaxis()->SetTitle("ADC Counts");
    graph1->SetMarkerStyle(1);
    graph1->SetTitle("");
    graph1->Draw("ap");


    //Raw Signal
    TGraph* grADC = new TGraph();
    //Mean
    TGraph* grmean = new TGraph();
    //Variance
    TGraph* grvariancethreshold = new TGraph();
    //Sup Data
    TGraph* grwindow = new TGraph();
    //Height
    TGraph* grheight = new TGraph();

    int startplot=319744000;
    int limitplot=ADC.size()-WINDOW_LEN-2;
    //int limitplot=160000;
    //int limitplot=1600000;   
    int j=0;
    for(int i=startplot;i<limitplot;i++){
      grADC->SetPoint(j,(i+WINDOW_LEN+2)*tadc,ADC.at(i+WINDOW_LEN));
      grmean->SetPoint(j,(i+WINDOW_LEN+2)*tadc,meanv.at(i));
      grvariancethreshold->SetPoint(j,(i+WINDOW_LEN+2)*tadc,variancev.at(i));
      grheight->SetPoint(j,(i+WINDOW_LEN+2)*tadc,heightv.at(i)*heightv.at(i));
      j++;
    }

    for(int i=0;i<supdata.size();i++){
    //for(int i=0;i<limitplot;i++){  
      grwindow->SetPoint(i,suptime.at(i)*tadc, supdata.at(i));
    }

    TLatex latex;
    latex.SetTextSize(0.06);
    //latex.DrawLatex(350,-1500,"200 kHz");
    
    grADC->SetMarkerColor(kBlack);
    grADC->SetLineColor(kBlack);
    grADC->SetMarkerStyle(6);
    grADC->Draw("l");


    grmean->SetMarkerColor(kBlue);
    grmean->SetLineColor(kBlue);
    grmean->SetMarkerStyle(6);
    grmean->Draw("l");

    grvariancethreshold->SetMarkerColor(kGreen);
    grvariancethreshold->SetLineColor(kGreen);
    grvariancethreshold->SetMarkerStyle(6);
    //grvariancethreshold->Draw("l");

    grwindow->SetMarkerColor(kPink);
    grwindow->SetLineColor(kPink);
    grwindow->SetMarkerStyle(6);
    grwindow->Draw("p");

    grheight->SetMarkerColor(kRed+2);
    grheight->SetLineColor(kRed+2);
    grheight->SetMarkerStyle(6);
    //grheight->Draw("p");


    TLine *lt1 = new TLine(storetrigger.at(0),yy1[0],storetrigger.at(0),yy1[1]);
    lt1->SetLineColor(kOrange+8);
    lt1->SetLineStyle(kDashed);
    //lt1->Draw();

    //end window 1
    TLine *l1 = new TLine(WF_LENGTH,yy1[0],WF_LENGTH,yy1[1]);
    l1->SetLineColor(kGreen+3);
    //l1->Draw();

    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grADC,"Data","l");
    legend->AddEntry(grmean,"Mean","l");
    //legend->AddEntry(grvariancethreshold,"Variance","l");
    legend->AddEntry(grwindow,"Stored data","l");
    //legend->AddEntry(lt1,"Trigger","l");
    //legend->AddEntry(l1,"Window size","l");
    legend->Draw("same");
}
 

  c->Modified();
  c->Update();
  //c->Print("Claudia.png");



#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
   



}

