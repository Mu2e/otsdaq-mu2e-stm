#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>

#include<stdio.h>
#include<stdlib.h>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"

void ELBE_Eff_DropPackets() {
  gROOT->SetStyle("ATLAS");

  double xx1[2]={0,210};
  //double yy1[2]={0,0.3};
  double yy1[2]={0,40}; 
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("Rate [kHz]");
  //graph1->GetYaxis()->SetTitle("MWD Efficiency");
  graph1->GetYaxis()->SetTitle("Reco Rate = MWD Efficiency#times Rate  [kHz]");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
 
  //Percentage of dropped packets
  int percentage = 75;
  int M =400;
  int L=200;


  TLatex latex;
  latex.SetTextSize(0.05);

  Int_t n;

  if(percentage==50&&M==1000&&L==500){
  //Efficiency using 50% of dropped packets and rates from 1 to 200 kHz
  n=13;
  Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};
  Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  Double_t Npeaksoriginal[13]={1000,5000,10000,20000,30000,40000,49999,80000,99999,120000,150000,180000,199999};
  Double_t Npeaksreco[13]={472,2156,4120,8401,13050,17303,21117,35476,41979,45764,34409,25180,18757};
  latex.DrawLatex(10,0.3,"50% Dropped Packets");
 

   for(int i=0;i<n;i++){
    eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
    eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
    std::cout<<"efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
    recorate[i]=eff[i]*rate[i];
    recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
    std::cout<<"recorate: "<<recorate[i]<<" "<<recorate_error[i]<<std::endl;

    std::cout<<" "<<std::endl;
  }

  TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
  greff->SetMarkerColor(kAzure+1);
  greff->Draw("same,p");
  
  /*TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
  grrecorate->SetMarkerColor(kOrange);
  grrecorate->Draw("same,p");
  */
}





  if(percentage==35&&M==1000&&L==500){
    //Efficiency using 35% of dropped packets and rates from 1 to 200 kHz
    n=15;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[15]={1,2,3,4,5,8,10,15,20,30,50,80,100,150,200};
    Double_t rate_error[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[15]={1000,2000,3000,4000,5000,8000,10000,15000,20000,30000,49999,80000,99999,150000,199999};
    Double_t Npeaksreco[15]={568,1135,1700,2191,2611,3926,4838,7341,9664,15416,25636,41626,48848, 58254,40341};
    latex.DrawLatex(2,0.3,"35% Dropped Packets");


    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    /*TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p");
    */
    TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    

  }



  if(percentage==75&&M==1000&&L==500){
    //Efficiency using 75% of dropped packets and rates from 8 to 200 kHz
    n=13;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[13]={8,10,15,20,30,40,50,80,100,120,150,180,200};
    Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[13]={8000,10000,15000,20000,30000,40000,49999,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksreco[13]={1042,1215,1932,2328,3909,5575,7018,11642,12759,14967,15692,12776,10525};
    latex.DrawLatex(20,10,"75% Dropped Packets");


    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    /*TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p");
    */
    TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    

  }






  if(percentage==60&&M==1000&&L==500){
    //Efficiency using 60% of dropped packets and rates from 20 to 200 kHz 
    n=12;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[12]={20,30,40,50,60,70,80,100,120,150,180,200};
    Double_t rate_error[12]={0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[12]={20000,30000,40000,49999,60000,70000,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksreco[12]={5098,7738,10270,12884,16622,19899,21300,23984,25514,30284,24486,20297};
    //latex.DrawLatex(20,10,"60% Dropped Packets");
    latex.DrawLatex(20,0.1,"60% Dropped Packets");

    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    /*TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]); 
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p");
    */
    TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    

  }






  if(percentage==75&&M==400&&L==200){
    //Efficiency using 75% of dropped packets and rates from 8 to 200 kHz                                                                  
    n=15;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[15]={8,10,15,20,30,40,50,60,70,80,100,120,150,180,200};
    Double_t rate_error[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[15]={8000,10000,15000,20000,30000,40000,49999,60000,70000,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksreco[15]={1707,2018,3041,3963,5900,7951,9976,12250,14058,16096,19884,23699,29884,34006,36257};
    //latex.DrawLatex(20,0.15,"75% Dropped Packets"); 
    latex.DrawLatex(20,30,"75% Dropped Packets");


    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    /*TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p");
    */
    TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    
  }




  if(percentage==35&&M==400&&L==200){
    //Efficiency using 35% of dropped packets and rates from 1 to 200 kHz                                                                  
    n=15;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[15]={1,2,3,4,5,8,10,15,20,30,50,80,100,150,200};
    Double_t rate_error[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[15]={1000,2000,3000,4000,5000,8000,10000,15000,20000,30000,49999,80000,99999,150000,199999};
    Double_t Npeaksreco[15]={619,1247,1839,2395,2966,4839,5997,8835,11741,17690,29007,47250,58838,87867,108652};
    latex.DrawLatex(2,0.3,"35% Dropped Packets");


    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    /*TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p"); 
    */
    TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    

  }







  if(percentage==60&&M==400&&L==200){
    //Efficiency using 60% of dropped packets and rates from 20 to 200 kHz                                                                 
    n=12;
    Double_t eff[n],eff_error[n],recorate[n],recorate_error[n];
    Double_t rate[12]={20,30,40,50,60,70,80,100,120,150,180,200};
    Double_t rate_error[12]={0,0,0,0,0,0,0,0,0,0,0,0};
    Double_t Npeaksoriginal[12]={20000,30000,40000,49999,60000,70000,80000,99999,120000,150000,180000,199999};
    Double_t Npeaksreco[12]={6756,10085,13372,16578,20111,23802,27216,33691,39874,50427,58127,61373};
    //latex.DrawLatex(20,10,"60% Dropped Packets");
    latex.DrawLatex(20,0.1,"60% Dropped Packets");

    for(int i=0;i<n;i++){
      std::cout<<"True Rate: "<<rate[i]<<" kHz"<<std::endl;
      eff[i]=Npeaksreco[i]/Npeaksoriginal[i];
      eff_error[i]=eff[i]/sqrt(Npeaksreco[i]);
      std::cout<<"Efficiency: "<<eff[i]<<" "<<eff_error[i]<<std::endl;
      recorate[i]=eff[i]*rate[i];
      recorate_error[i]=recorate[i]*eff_error[i]/eff[i];
      std::cout<<"Reco Rate: "<<recorate[i]<<" "<<recorate_error[i]<<" kHz"<<std::endl;

      std::cout<<" "<<std::endl;
    }

    TGraphErrors *greff = new TGraphErrors(n,&rate[0],&eff[0],&rate_error[0],&eff_error[0]);
    greff->SetMarkerColor(kAzure+1);
    greff->Draw("same,p"); 
    
    /*TGraphErrors *grrecorate = new TGraphErrors(n,&rate[0],&recorate[0],&rate_error[0],&recorate_error[0]);
    grrecorate->SetMarkerColor(kOrange);
    grrecorate->Draw("same,p");
    */

  }





 
}
