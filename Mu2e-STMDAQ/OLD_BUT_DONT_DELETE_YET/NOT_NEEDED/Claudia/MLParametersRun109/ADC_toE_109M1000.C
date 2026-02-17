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
#include "TAxis.h"
#include "TH1F.h"

void ADC_toE_109M1000() {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");
  //c1->SetGrid();



  int M=1000;

  int L1=1000;
  int L2=900;
  int L3=800;
  int L4=700;

  std::string Ms=std::to_string(M);
  char* Mchar = const_cast<char*>(Ms.c_str());

  std::string L1s=std::to_string(L1);
  char* L1char = const_cast<char*>(L1s.c_str());
  std::string L2s=std::to_string(L2);
  char* L2char = const_cast<char*>(L2s.c_str());
  std::string L3s=std::to_string(L3);
  char* L3char = const_cast<char*>(L3s.c_str());
  std::string L4s=std::to_string(L4);
  char* L4char = const_cast<char*>(L4s.c_str());



  double xx1[2]={630,680};
  double yy1[2]={0,1100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(630,680);      
  graph1->GetYaxis()->SetRangeUser(0,1100);
  graph1->SetTitle("{}^{137}Cs, M=1000");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  int bins=int((xx1[1]-xx1[0])/0.57);
  double fitrange[2]={630, 675};


  //M1000L1000  
  TH1F*h1 = new TH1F("TH1","", bins, xx1[0], xx1[1]);//binning of 0.57 kev    
  //Abrimos el txt
  fstream readfile;
  //readfile.open("../../../DATA/Claudia/MLParametersRun109/M1000L1000/run_00109.txt",ios::in);
  readfile.open("/data1/cgarcia/DATA/MWD_Analysis/RUN109/M"+Ms+"L"+L1s+"/run_00109.txt",ios::in);     
  string name;
  vector<string> file_name;
  file_name.clear();

  double energypeak;

  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    //cout<<name<<endl;
  }
  std::cout<<"Size: "<<file_name.size()<<std::endl;
  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();
    
    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);
      /*energypeak=(peaks-0.354947)/(-1.75514); //liv old cali M=8000*/
      energypeak=peaks*(-0.57);
      h1->Fill(energypeak);

    }
  }//for int file

  //double numberentries661 = h1->Integral(h1->FindFixBin(640), h1->FindFixBin(680), "");
  //std::cout<<"Number of X-Rays for the 661.7 keV peak integral: "<<numberentries661<<std::endl;
  h1->GetXaxis()->SetTitle("E (keV)");
  h1->SetLineColor(kViolet-9);
  h1->SetTitle("");
  h1->SetStats(1);
  //h1->Draw("same");

  //Fit Energy peak Cessium
  TF1*Fitwide = new TF1("Fitwide", "[0]*TMath::Gaus(x,[1],[2])", xx1[0], xx1[1]);
  Fitwide->SetParameters(5.00221e+02,640,3);
  h1->Fit(Fitwide,"0","",fitrange[0], fitrange[1]);

  
  Fitwide->SetLineColor(kBlue);
  Fitwide->SetLineStyle(2);
  //Fitwide->Draw("same");

  
  cout<<"Mean L="+L1s+": "<<h1->GetMean()<<endl;
  cout<<"RMS L="+L1s+": "<<h1->GetRMS()<<endl;
  cout<<"MeanError L="+L1s+": "<<h1->GetMeanError()<<endl;
  cout<<"RMSError L="+L1s+": "<<h1->GetRMSError()<<endl;

  






  ////
  //M1000L500                                                                                                                         
  int numbercounts=0;
 
  TH1F*h2 = new TH1F("TH2","", bins, xx1[0], xx1[1]);//binning of 0.57 kev

  fstream readfile2;
  //readfile2.open("/data1/cgarcia/DATA/Claudia/MLParametersRun109/M1000L500/run_00109.txt",ios::in);
  readfile2.open("/data1/cgarcia/DATA/MWD_Analysis/RUN109/M"+Ms+"L"+L2s+"/run_00109.txt",ios::in);  
  string name2;
  vector<string> file_name2;
  file_name2.clear();

 

  //Lee cada fila del .txt que es cada uno de los nombres de los .root                                                                                                                                                                                        
  while(1){
    readfile2>>name2;
    file_name2.push_back(name2);
    if(readfile2.eof())break;
    //cout<<name<<endl;                                                                                                                                                                                                                                       
  }
  std::cout<<"Size: "<<file_name2.size()<<std::endl;
  for (int file=0;file<(file_name2.size()-1);file++){
    string path;
    path=file_name2[file];
    //cout<<file_name.size()<<endl;                                                                                                                                                                                                                           
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:                                                                                                                                                                                               
      tree->GetEntry(i);
      /*energypeak=(peaks-0.354947)/(-1.75514); //liv old cali M=8000*/
      energypeak=peaks*(-0.57);
      h2->Fill(energypeak);
      if((energypeak>655)&&(energypeak<675)){numbercounts++;}
    }
  }//for int file                        
  std::cout<<"Number of counts in the photopeak between 655 and 675: "<<numbercounts<<std::endl;

  h2->GetXaxis()->SetTitle("E (keV)");
  h2->SetLineColor(kRed-9);
  h2->SetTitle("");
  h2->SetStats(1);
  h2->Draw("same");

  //Fit Energy peak Cessium                                                                                                                                                                                                                                   
  TF1*Fitwide2 = new TF1("Fitwide2", "[0]*TMath::Gaus(x,[1],[2])",xx1[0], xx1[1]);
  Fitwide2->SetParameters(1.31429e+02,660,2.01827e+00 );
  h2->Fit(Fitwide2,"0","",fitrange[0], fitrange[1]);


  Fitwide2->SetLineColor(kRed);    
  Fitwide2->SetLineStyle(2);
  //Fitwide2->Draw("same");                                                                                                              

  //double countsRaw = h2->Integral(h2->FindFixBin(650), h2->FindFixBin(670), "");
  //std::cout<<"Number of counts in the raw-data photopeak, between 650 and 670 keV: "<<countsRaw<<endl;

  cout<<"Mean L="+L2s+": "<<h2->GetMean()<<endl;
  cout<<"RMS L="+L2s+": "<<h2->GetRMS()<<endl;
  cout<<"MeanError L="+L2s+": "<<h2->GetMeanError()<<endl;
  cout<<"RMSError L="+L2s+": "<<h2->GetRMSError()<<endl;





  //
  //M1000L100                                                                                                                                                                                                                                               
  TH1F*h3 = new TH1F("TH3","", bins, xx1[0], xx1[1]);//binning of 0.57 kev                                                                                                                                                                                           
  //Abrimos el txt                                                                                                                                                                                                                                            
  fstream readfile3;
  //readfile3.open("../../../DATA/Claudia/MLParametersRun109/M1000L100/run_00109.txt",ios::in);
  readfile3.open("/data1/cgarcia/DATA/MWD_Analysis/RUN109/M"+Ms+"L"+L3s+"/run_00109.txt",ios::in);  
  string name3;
  vector<string> file_name3;
  file_name3.clear();

 

  //Lee cada fila del .txt que es cada uno de los nombres de los .root                                                                                                                                                                                        
  while(1){
    readfile3>>name3;
    file_name3.push_back(name3);
    if(readfile3.eof())break;
    //cout<<name<<endl;                                                                                                                                                                                                                                       
  }
  std::cout<<"Size: "<<file_name3.size()<<std::endl;
  for (int file=0;file<(file_name3.size()-1);file++){
    string path;
    path=file_name3[file];
    //cout<<file_name.size()<<endl;                                                                                                                                                                                                                           
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:                                                                                                                                                                                               
      tree->GetEntry(i);
      /*energypeak=(peaks-0.354947)/(-1.75514); //liv old cali M=8000*/
      energypeak=peaks*(-0.57);
      h3->Fill(energypeak);
    }
  }//for int file                                                                                                                                                                                                                                             

  h3->GetXaxis()->SetTitle("E (keV)");
  h3->SetLineColor(kGreen-9);
  h3->SetTitle("");
  h3->SetStats(1);
  h3->Draw("same");

  //Fit Energy peak Cessium                                                                                                                                                                                                                                   
  TF1*Fitwide3 = new TF1("Fitwide3", "[0]*TMath::Gaus(x,[1],[2])", xx1[0], xx1[1]);
  Fitwide3->SetParameters(1.31429e+02,660,2.01827e+00 );
  h3->Fit(Fitwide3,"0","",fitrange[0], fitrange[1]);


  Fitwide3->SetLineColor(kGreen);                                                                                            
  Fitwide3->SetLineStyle(2);
  //Fitwide3->Draw("same");                                                                                                                                                                                                                                    


  cout<<"Mean L="+L3s+": "<<h3->GetMean()<<endl;
  cout<<"RMS L="+L3s+": "<<h3->GetRMS()<<endl;
  cout<<"MeanError L="+L3s+": "<<h3->GetMeanError()<<endl;
  cout<<"RMSError L="+L3s+": "<<h3->GetRMSError()<<endl;





  //M1000L50                                                                                                      
  TH1F*h4 = new TH1F("TH4","", bins, xx1[0], xx1[1]);//binning of 0.57 kev                                        

  fstream readfile4;
  readfile4.open("/data1/cgarcia/DATA/MWD_Analysis/RUN109/M"+Ms+"L"+L4s+"/run_00109.txt",ios::in);  
  string name4;
  vector<string> file_name4;
  file_name4.clear();

  while(1){
    readfile4>>name4;
    file_name4.push_back(name4);
    if(readfile4.eof())break;
    //cout<<name<<endl;                                                                                            
  }
  std::cout<<"Size: "<<file_name4.size()<<std::endl;
  for (int file=0;file<(file_name4.size()-1);file++){
    string path;
    path=file_name4[file];
    //cout<<file_name.size()<<endl;                                                                                
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);
      /*energypeak=(peaks-0.354947)/(-1.75514); //liv old cali M=8000*/
      energypeak=peaks*(-0.57);
      h4->Fill(energypeak);
     
    }
  }//for int file
                                                                                                                   
  h4->GetXaxis()->SetTitle("E (keV)");
  h4->SetLineColor(kAzure-9);
  h4->SetTitle("");
  h4->SetStats(1);
  h4->Draw("same");

  TF1*Fitwide4 = new TF1("Fitwide4", "[0]*TMath::Gaus(x,[1],[2])", xx1[0], xx1[1]);
  Fitwide4->SetParameters(1.31429e+02,665,2.01827e+00 );
  h4->Fit(Fitwide4,"0","",fitrange[0], fitrange[1]);

  Fitwide4->SetLineColor(kCyan);
  Fitwide4->SetLineStyle(2);                                                                                     
  //Fitwide4->Draw("same");

  cout<<"Mean L="+L4s+": "<<h4->GetMean()<<endl;
  cout<<"RMS L="+L4s+": "<<h4->GetRMS()<<endl;
  cout<<"MeanError L="+L4s+": "<<h4->GetMeanError()<<endl;
  cout<<"RMSError L="+L4s+": "<<h4->GetRMSError()<<endl;










  std::string str = "{}^{137}Cs, M="+Ms;
  char* title = const_cast<char*>(str.c_str());
  TLatex T1;
  T1.DrawLatexNDC(.2,.85, title);


  std::string L1s_leg="L="+std::to_string(L1);
  char* L1char_leg = const_cast<char*>(L1s_leg.c_str());

  std::string L2s_leg="L="+std::to_string(L2);
  char* L2char_leg = const_cast<char*>(L2s_leg.c_str());

  std::string L3s_leg="L="+std::to_string(L3);
  char* L3char_leg = const_cast<char*>(L3s_leg.c_str());

  std::string L4s_leg="L="+std::to_string(L4);
  char* L4char_leg = const_cast<char*>(L4s_leg.c_str());




TLine *line661=new TLine(661.7,yy1[0],661.7,yy1[1]);
//line661->Draw("same");
 auto leg1 = new TLegend(0.21,0.6,0.55,0.82);
  /* leg1->AddEntry(h5, "L=100, #sigma=2.62#pm0.03","l");
  leg1->AddEntry(h4, "L=500, #sigma=2.039#pm0.024","l");
  leg1->AddEntry(h3, "L=1000, #sigma=1.923#pm0.022","l");
  leg1->AddEntry(h2, "L=5000, #sigma=1.75#pm0.02","l");
  leg1->AddEntry(h1, "L=8000, #sigma=1.70#pm0.02","l");*/

  leg1->AddEntry(h4, L4char_leg,"l");
  leg1->AddEntry(h3, L3char_leg,"l");
  leg1->AddEntry(h2, L2char_leg,"l");
  //leg1->AddEntry(h1, L1char_leg,"l");
  leg1->Draw("same");



  //c1->Print("M1000new_0.57kev.pdf","pdf");
  //c1->Print("M1000new_0.57kev.png","png");
  //c1->Print("M1000fitnew_0.57kev.pdf","pdf");                                                                           
  //c1->Print("M1000fitnew_0.57kev.png","png"); 
}
