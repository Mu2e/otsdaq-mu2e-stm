#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>
#include "TAxis.h"

#include "TGraph.h"
#include "TCanvas.h"

void AdjustingParameters109() {
  auto c1= new TCanvas("c1","Title",400,10,1000,500);

  //  double xx1[2]={1200000,2400000};
  //double xx1[2]={0,6000000};
  double xx1[2]={0,1400000};
  //double xx1[2]={4000000,6000000};
  //double xx1[2]={1000000,2000000};
  //double yy1[2]={0,1000};
  double yy1[2]={-1000,1000};
  //double yy1[2]={-2,2};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  //graph1->GetXaxis()->SetRangeUser(0,6000000);
  graph1->GetXaxis()->SetRangeUser(0,1400000);
  //graph1->GetXaxis()->SetRangeUser(4000000,6000000);
  //graph1->GetYaxis()->SetRangeUser(0,1000);
  graph1->GetYaxis()->SetRangeUser(-1000,1000);
  //graph1->GetXaxis()->SetRangeUser(1000000,2000000);
  //graph1->GetYaxis()->SetRangeUser(-2,2);
  graph1->GetXaxis()->SetTitle("Time (ns)");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");

  //std::vector<uint16_t> ADC;
  std::vector<int16_t> ADC;
  ADC.clear();
  //std::ifstream myFile("run00109.bin_00", ios::in | ios::binary);
  std::ifstream myFile("../../../DATA/MWD_Analysis/RUN109/run00109.new.bin_00", ios::in | ios::binary);
  //size we want to read
  int size=0;
  int16_t inf ;
  //Each number
  //while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )&&(size<limitbytesread) ){
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
    ADC.push_back(inf);
    //std::cout<<inf<<std::endl;
    size +=sizeof(inf);
  }

  TGraph* gr = new TGraph();
  TGraph* grpoint = new TGraph();
  TGraph* dec = new TGraph();
  TGraph* dif = new TGraph();
  TGraph* av = new TGraph();
  std::vector<double> time;
  time.clear();
  double t=0;

  cout<<ADC.size()<<endl;
  int n=10000000;

  for(int i=0; i < n; i++) {
    time.push_back(t);
    gr->SetPoint(i,t,ADC.at(i));
    //if(t>5400000&&t<5700000){
    //cout<<"Event: "<<i<<" ADC value: "<<ADC.at(i)<<endl;}
    t+=2.7;
  }
  //gap
  //grpoint->SetPoint(0,time.at(2060383),ADC.at(2060383));
  //grpoint->SetPoint(1,time.at(2060384),ADC.at(2060384));
  //gap points of the csv
  /*grpoint->SetPoint(0,time.at(89776),0x0113);
    grpoint->SetPoint(1,time.at(89777),0x00ff);
    grpoint->SetPoint(2,time.at(89778),0x00e4);
    grpoint->SetPoint(3,time.at(89779),0x00d4);
    grpoint->SetPoint(4,time.at(89780),0x00c6);
    grpoint->SetPoint(5,time.at(89781),0x00aa);
    grpoint->SetPoint(6,time.at(89782),0x009d);
    grpoint->SetPoint(7,time.at(89783),0x008c);
    grpoint->SetPoint(8,time.at(89784),0x0060);
    grpoint->SetPoint(9,time.at(89785),0x002d);
    grpoint->SetPoint(10,time.at(89786),0x0020);
    grpoint->SetPoint(11,time.at(89787),0x0004);
    grpoint->SetPoint(12,time.at(89788),0x000f);
    grpoint->SetPoint(13,time.at(89789),0xffd3);
    grpoint->SetPoint(14,time.at(89790),0xffbb);
    grpoint->SetPoint(15,time.at(89791),0xff89);*/


  //grpoint->SetPoint(16,time.at(89792),0xff6c-65536);
  //grpoint->SetPoint(17,time.at(89793),0xff4d-65536);
  //cout<<int(0xff6c)-65536<<" "<<int(0xff4d)-65536<<endl;
  //int a = 0x3E;

  graph1->Draw("ap");
  double T0=2.7;

  //Deconvolution
  double tau;

  tau=55748.2;

  vector<double> a;
  a.clear();

  a.push_back(ADC.at(0));
  //dec->SetPoint(0,time.at(0),a.at(0));
  for(int i=1; i<n; i++){
    a.push_back(ADC.at(i)-(1-(T0/tau))*ADC.at(i-1)+a.at(i-1));
    //cout<<a[i]<<endl;
    //dec->SetPoint(i,time.at(i),a.at(i));
  }

  //Differentiation

  vector<double> D;
  D.clear();
  int M=8000;

  for (int i = 0; i < M; ++i) {
    D.push_back(a.at(i));
    dif->SetPoint(i,time.at(i),D.at(i));
  }

  for (int i = M; i < n; ++i) {
    D.push_back(a.at(i)-a.at(i-M));
    dif->SetPoint(i,time.at(i),D.at(i));
  }

  //Averaging

  vector<double> l;
  l.clear();
  int L=1000;
  double sum = 0.;

  for (int i = 0; i < L-1; ++i) {
    sum += D.at(i);
    l.push_back(D.at(i));
    //cout<<l.at(i)<<endl;
    av->SetPoint(i,time.at(i),l.at(i));
  }

  sum += D.at(L-1);
  l.push_back(sum/L);

  for (int i = L; i < n; ++i) {
    l.push_back(sum/L);
    //if(l.at(i)<0){cout<<l.at(i)<<endl;}
    sum += D.at(i)-D.at(i-L);
    av->SetPoint(i,time.at(i),l.at(i));
  }


  gr->SetMarkerColor(kBlack);
  gr->SetMarkerStyle(5);
  gr->Draw("same, p");
  grpoint->SetMarkerColor(kRed);
  grpoint->SetMarkerStyle(5);
  grpoint->Draw("same, p");

  //dec->SetMarkerColor(kRed);
  //dec->SetLineColor(kRed);
  //dec->SetMarkerStyle(5);
  //dec->Draw("same");

  //  dif->SetMarkerColor(kGreen);
  //dif->SetLineColor(kGreen);
  //  dif->SetMarkerStyle(5);
  //  dif->Draw("same");

  //  av->SetMarkerColor(kBlue);
  //  av->SetLineColor(kBlue);
  //  av->SetMarkerStyle(5);
  //  av->Draw("same");


  //Plotting the distribuition of the gradient
  /*TH1F*h2 = new TH1F("TH2","", 100, -0.5, 0.5);
          //Plotting the gradient
          TGraph* grad = new TGraph();
          double gradient1, gradient;

            //Plotting the gradient distribuition of a region to see the mean of the gradient
            //t = 3000000; t <4000000
            for(int j = 1111111; j < 1481481; ++j){
                   gradient1=l.at(j-1)-l.at(j);
                   h2->Fill(gradient1);}

           double mean= h2->GetMean();
           double sigma= h2->GetRMS();
           double threesigmas=5*sigma;
           cout<<"Gradient mean and sigma: "<<mean<<"  "<<sigma<<endl;
           //h2->Draw();
            //double mean= -1.42615e-05 ;
            //double threesigmas=3*0.0403056;


            //Uncomment to plot the gradient
           double auxup=0, timeup=0;
           double auxlow=0, timelow=0;
           //1000- n
           //for(int i = 1000; i < n; ++i){
           for(int i = 370370; i < 740740; ++i){
             gradient=l.at(i-1)-l.at(i);
             grad->SetPoint(i,time.at(i),gradient);
               if(gradient<auxlow){auxlow=gradient;
	       timelow=time.at(i);}
               if(gradient>auxup){auxup=gradient;
  timeup=time.at(i);}
}
           cout<<"Empieza el pulso: "<<timeup<<" Termina el pulso: "<<timelow<<endl;
           cout<<"Duracion del pulso: "<<timelow-timeup<<" ns"<<endl;
           grad->GetXaxis()->SetRangeUser(0,6000000);
           //grad->GetXaxis()->SetRangeUser(550000,650000);
           grad->GetYaxis()->SetRangeUser(-2,2);
           //grad->GetXaxis()->SetRangeUser(40000,100000);
           //grad->GetYaxis()->SetRangeUser(-0.6,0.6);
           grad->SetLineColor(kOrange);
           grad->SetMarkerColor(kOrange);
           grad->GetXaxis()->SetTitle("Time (ns)");
           //grad->GetYaxis()->SetTitle("ADC Counts");
           grad->Draw("same");

           TLine *linenoiseup=new TLine(0,mean+threesigmas,6000000,mean+threesigmas);
           //linenoiseup->Draw("same");

           TLine *linenoisedown=new TLine(0,mean-threesigmas,6000000,mean-threesigmas);
           //linenoisedown->Draw("same");

           TLine *linepulso=new TLine(timeup,auxlow,timelow,auxlow);
           linepulso->SetLineColor(kBlue);
           linepulso->Draw("same");

           TLine *linepulsovert=new TLine(timeup,auxlow,timeup,auxup);
           linepulsovert->SetLineColor(kBlue);
           linepulsovert->Draw("same");

           auto leg2 = new TLegend(0.1,0.7,0.48,0.9);
           leg2->AddEntry(grad, "Voltage gradient","l");
           //leg2->AddEntry(linenoiseup, "5#sigma","l");
           leg2->AddEntry(linepulso, "Pulse duration ~ 22323.6 ns ns","l");
           leg2->Draw("same");*/








  //Mean Signal
  /* TH1F*h1 = new TH1F("TH1","", 1000, -1000, 1000);
          //t=3000000; t <4000000
          for(int i=1111111;i<1481481;i++){
            h1->Fill(l.at(i));
          }
          double mean= h1->GetMean();
          double sigma= h1->GetRMS();

          cout<<"Signal threshold: "<<mean<<"Sigma: "<<sigma<<endl;
          TLine *line=new TLine(0,mean,6000000,mean);
          line->SetLineColor(kRed);
          line->Draw("same");

          TLine *line2=new TLine(0,mean+4*sigma,6000000,mean+4*sigma);
          line2->SetLineColor(kBlack);
          line2->Draw("same");
          TLine *line3=new TLine(0,mean-4*sigma,6000000,mean-4*sigma);
          line3->SetLineColor(kBlack);
          line3->Draw("same");


          auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
          leg1->AddEntry(gr, "Signal","p");
          //leg1->AddEntry(dec, "Deconvolution","p");
          leg1->AddEntry(dif, "M-Step Differentiation","p");
          leg1->AddEntry(av, "Moving Window Average","p");
          leg1->AddEntry(line, "Mean = 354.307", "l");
          leg1->AddEntry(line2, "+- 4#sigma", "l");
          leg1->Draw("same");*/



  //Fitting the peak tau
  //TF1 *f1 = new TF1("f1","[0]*exp(-x/[1])+[2]",1440000,2000000);
  /* TF1 *f1 = new TF1("f1","[0]*exp(-x/[1])+[2]",250000,800000);
     f1->SetParameters(  -1.15105e+14,  55748.2, 914.485);
     gr->Fit("f1","R");


     f1->SetLineColor(kRed);
     f1->Draw("same");

     auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
     //leg1->AddEntry(f1, "#tau = 55748.2 ns","l");
     leg1->AddEntry(f1, "#tau = 60029.5 ns","l");
     leg1->Draw("same");*/


}
