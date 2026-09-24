//Plotting E Spectrum of Cs and Eu together
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
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"

void E_Cs_Eu() {
  gROOT->SetStyle("ATLAS");
  //TCanvas (const char *name, const char *title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
  auto c1= new TCanvas("c1","Title",400,10,1500,500);



  double xx1[2]={0, 1600};
  double yy1[2]={0,2500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");

  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TH1F*h1 = new TH1F("TH1","", 2807 , 0,1600);//0.57kev bin 

  //Abrimos el txt
  fstream readfile;
  readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN110/M1000L500/run_00110.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Lee cada fila del .txt que es cada uno de los nombres de los .root
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    //cout<<name<<endl;
  }

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
      
      //Using old roots Liverpool Calibration M=8000 L=1000
      //double peakEnergy=(peaks-0.354947)/(-1.75514);
  
      //Using Liverpool Calibration with M=400, L=200
      //double peakEnergy=(peaks-1.738)/(-1.767);

      //Using Liverpool Calibration with M=8000, L=1000
      //double peakEnergy=(peaks+3.392)/(-1.753);

      //Using Liverpool Calibration with M=1000, L=500
      double peakEnergy=(peaks-2.73)/(-1.76);

      //Cut at 30 keV  
      if(peakEnergy>30){
	h1->Fill(peakEnergy);
      }
    }
  }//for int file




  TH1F*h2 = new TH1F("TH2","",2807, 0, 1600);//0.57kev bin

  fstream readfile2;

  readfile2.open("/work/cgarcia/DATA/MWD_Analysis/RUN109/M1000L500/run_00109.txt",ios::in);
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

  for (int file=0;file<(file_name2.size()-1);file++){
    string path2;
    path2=file_name2[file];
    //cout<<file_name.size()<<endl;                                                                                         
    cout<<path2.c_str()<<endl;

    TFile *input2=new TFile(path2.c_str());
    TTree* tree2=(TTree*)input2->Get("treeADC");
    double peaks;

    tree2->SetBranchAddress("peaks",&peaks);

    unsigned long entries2=tree2->GetEntries();

    cout<<"entries: "<<entries2<<endl;
    for(unsigned long i=0;i<entries2;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:                                                             
      tree2->GetEntry(i);


      //Using old roots Liverpool Calibration M=8000 L=1000
      //double peakEnergy=(peaks-0.354947)/(-1.75514);

      //Using Liverpool Calibration with M=400, L=200
      //double peakEnergy=(peaks-1.738)/(-1.767);

      //Using Liverpool Calibration with M=8000, L=1000
      //double peakEnergy=(peaks+3.392)/(-1.753);

      //Using Liverpool Calibration with M=1000, L=500
      double peakEnergy=(peaks-2.73)/(-1.76);

      if(peakEnergy<800&&peakEnergy>600){
	h2->Fill(peakEnergy);}
    }
  }//for int file         

  TLatex T1;
  T1.DrawLatexNDC(.3,.85, "40.1186 keV");
  T1.DrawLatexNDC(.3,.85, "121.78 keV");
  T1.DrawLatexNDC(.3,.85, "244.7 keV");
  T1.DrawLatexNDC(.3,.85, "344.28 keV");
  T1.DrawLatexNDC(.3,.85, "778.91 keV");
  T1.DrawLatexNDC(.3,.85, "964.08 keV");
  T1.DrawLatexNDC(.3,.85, "1085.837 keV");
  T1.DrawLatexNDC(.3,.85, "1112.076 keV");
  T1.DrawLatexNDC(.3,.85, "1408.13 keV");
  T1.DrawLatexNDC(.3,.85, "661.7 keV");
  T1.DrawLatexNDC(.3,.85, "411.1165 keV");
  T1.DrawLatexNDC(.3,.85, "443.965 keV");
  T1.DrawLatexNDC(.3,.85, "867.38 keV");
 


  TLine *line40=new TLine(40.1186,yy1[0],40.1186,yy1[1]);
  line40->SetLineColor(kBlue);
  line40->SetLineWidth(2);
  line40->SetLineStyle(3); 
  line40->Draw("same");                                                                                                     
  /*auto leg1 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg1->AddEntry(line40, "40.1186 keV","l");  
  */                                                                                                                      
  TLine *line121=new TLine(121.78,yy1[0],121.78,yy1[1]);
  line121->SetLineColor(kBlue);
  line121->SetLineWidth(2);
  line121->SetLineStyle(3);       
  line121->Draw("same");
  /*auto leg2 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg2->AddEntry(line121, "121.78 keV","l");*/    
                                                                                                                            
  TLine *line244=new TLine(244.7,yy1[0],244.7,yy1[1]);
  line244->SetLineColor(kBlue);
  line244->SetLineWidth(2);
  line244->SetLineStyle(3);       
  line244->Draw("same");
  /* auto leg3 = new TLegend(0.1,0.7,0.48,0.9);
     leg3->AddEntry(line244, "244.7 keV","l");*/
                                                                                                                            
  TLine *line344=new TLine(344.28,yy1[0],344.28,yy1[1]);
  line344->SetLineColor(kBlue);
  line344->SetLineWidth(2);
  line344->SetLineStyle(3);   
  line344->Draw("same");
  /*auto leg4 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg4->AddEntry(line244, "344.28 keV","l");*/
                                                                                                                            
  TLine *line778=new TLine(778.91,yy1[0],778.91,yy1[1]);
  line778->SetLineColor(kBlue);
  line778->SetLineWidth(2);
  line778->SetLineStyle(3);
  line778->Draw("same");
  /*auto leg5 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg5->AddEntry(line778, "778.91 keV","l");*/
                                                                                                                            
  TLine *line964=new TLine(964.08,yy1[0],964.08,yy1[1]);
  line964->SetLineColor(kBlue);
  line964->SetLineWidth(2);
  line964->SetLineStyle(3);
  line964->Draw("same");
  /*auto leg6 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg6->AddEntry(line964, "964.08 keV","l");*/
                                                                                                                            
  TLine *line1085=new TLine(1085.837,yy1[0],1085.837,yy1[1]);
  line1085->SetLineColor(kBlue);
  line1085->SetLineWidth(2);
  line1085->SetLineStyle(3);
  line1085->Draw("same");
  /*auto leg7 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg7->AddEntry(line1085, "1085.837 keV","l");*/
                                                                                                                            
  TLine *line1112=new TLine(1112.076,yy1[0],1112.076,yy1[1]);
  line1112->SetLineColor(kBlue);
  line1112->SetLineWidth(2);
  line1112->SetLineStyle(3);
  line1112->Draw("same");
  /*auto leg8 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
  leg8->AddEntry(line1112, "1112.076 keV","l");*/
                                                                                                                            
  TLine *line1408=new TLine(1408.13,yy1[0],1408.13,yy1[1]);
  line1408->SetLineColor(kBlue);
  line1408->SetLineWidth(2);
  line1408->SetLineStyle(3);
  line1408->Draw("same");   
  /*auto leg9 = new TLegend(0.1,0.7,0.48,0.9);                                                                                
    leg9->AddEntry(line1408, "1408.013 keV","l");*/

  TLine *line661=new TLine(661.7,yy1[0],661.7,yy1[1]);
  line661->SetLineColor(kBlue);
  line661->SetLineWidth(2);
  line661->SetLineStyle(3);
  line661->Draw("same");
  /*auto leg10 = new TLegend(0.1,0.7,0.48,0.9);
  leg10->AddEntry(line661, "661.7 keV","l");
  */
  TLine *line411=new TLine(411.1165,yy1[0],411.1165,yy1[1]);
  line411->SetLineColor(kBlue);
  line411->SetLineWidth(2);
  line411->SetLineStyle(3);  
  line411->Draw("same");
  /*auto leg11 = new TLegend(0.1,0.7,0.48,0.9);
  leg11->AddEntry(line411, "411.1165 keV","l");
  */

  TLine *line443=new TLine(443.965,yy1[0],443.965,yy1[1]);
  line443->SetLineColor(kBlue);
  line443->SetLineWidth(2);
  line443->SetLineStyle(3);
  line443->Draw("same");
  /*auto leg12 = new TLegend(0.1,0.7,0.48,0.9);
  leg12->AddEntry(line443, "443.965 keV","l");
  */

  TLine *line867=new TLine(867.38,yy1[0],867.38,yy1[1]);
  line867->SetLineColor(kBlue);
  line867->SetLineWidth(2);
  line867->SetLineStyle(3);
  line867->Draw("same");
  /*auto leg13 = new TLegend(0.1,0.7,0.48,0.9);
  leg13->AddEntry(line867, "867.38 keV","l");
  */

  h1->GetXaxis()->SetTitle("E [keV]");
  h1->SetTitle("");
  h1->SetStats(0);
  //h1->SetFillColor(kGreen-9);
  h1->SetFillColor(kCyan-3);
  //h1->SetFillColor(kRed); 
  h1->Draw("same");

  h2->GetXaxis()->SetTitle("E [keV]");
  h2->SetTitle("");
  h2->SetStats(0);
  //h2->SetFillColor(kRed-9);
  h2->SetFillColor(kOrange-3);
  //h2->SetFillColor(kYellow);
  h2->Draw("same");

   auto leg = new TLegend(0.1,0.7,0.48,0.9);
  leg->AddEntry(h1, "{}^{152}Eu","f");
  leg->AddEntry(h2, "{}^{137}Cs","f");

  /*leg1->Draw("same");
  leg2->Draw("same");
  leg3->Draw("same");
  leg4->Draw("same");
  leg5->Draw("same");
  leg6->Draw("same");
  leg7->Draw("same");
  leg8->Draw("same");
  leg9->Draw("same");
  leg10->Draw("same"); 
  leg11->Draw("same");
  leg12->Draw("same");
  leg13->Draw("same");*/
  leg->Draw("same");  
//  c1->Print("EnergyEu_run00110.pdf","pdf");
  // c1->Print("EnergyEu_run00110.png","png");  
}
