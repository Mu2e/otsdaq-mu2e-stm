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

void ADC_toE_110() {
  gROOT->SetStyle("ATLAS");
  //auto c1= new TCanvas("c1","Title",400,10,1500,500); 
  auto c1= new TCanvas("c1");


  //double xx1[2]={0, 800};
  double xx1[2]={330, 350};
  double yy1[2]={0,2500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]); 
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("{}^{152}Eu");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TH1F*h1 = new TH1F("TH1","", 1404, 0, 800); //0.57 kev binning
  //TH1F*h1 = new TH1F("TH1","", 2105, 800, 2000); //0.57 kev binning 

  //Abrimos el txt
  fstream readfile;
  //Open (old generated root files) ROOT files for RUN110 using M=8000,L=1000  
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN110/run_00110.txt",ios::in);

  //Open ROOT files for RUN110 using M=400,L=200
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN110/M400L200/run_00110.txt",ios::in);
  
  //Open ROOT files for RUN110 using M=8000,L=1000
  //readfile.open("/work/cgarcia/DATA/MWD_Analysis/RUN110/M8000L1000/run_00110.txt",ios::in);
  //Open ROOT files for RUN110 using M=1000,L=500
  readfile.open("/work/mu2e/data1/cgarcia/DATA/MWD_Analysis/RUN110/M1000L500/run_00110.txt",ios::in);
  
  string name;
  vector<string> file_name;
  file_name.clear();

  int xrays[12];
  for(int i=0;i<12;i++){
    xrays[i]=0;
  }

  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
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
    double peakEnergy;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      //Cada punto es una entrada del arbol, tiene 10 entradas:
      tree->GetEntry(i);

      //Using (old generated root files) Liverpool Calibration with M=8000, L=1000 
      //peakEnergy=(peaks-0.354947)/(-1.75514);

      //Using Liverpool Calibration with M=400, L=200
      //peakEnergy=(peaks-1.738)/(-1.767);
      
      //Using Liverpool Calibration with M=8000, L=1000
      //peakEnergy=(peaks+3.392)/(-1.753); 

      //Using Liverpool Calibration with M=1000, L=500
      //peakEnergy=(peaks-2.73)/(-1.76);

      peakEnergy=peaks*(-0.57);
      //standard
      
      if(peakEnergy>30){
	//Number of X-Rays inside the gaussian 0-800
        if(peakEnergy<45&&peakEnergy>35){xrays[0]++;}
	if(peakEnergy<125&&peakEnergy>117){xrays[1]++;}
	if(peakEnergy<249&&peakEnergy>238){xrays[2]++;}
	if(peakEnergy<348&&peakEnergy>338){xrays[3]++;}
	if(peakEnergy<418&&peakEnergy>404){xrays[5]++;}
	if(peakEnergy<450&&peakEnergy>438){xrays[6]++;}
	if(peakEnergy<786&&peakEnergy>774){xrays[4]++;}
	//Number of X-Rays inside the gaussian 800-2000
	if(peakEnergy<876&&peakEnergy>864){xrays[7]++;}
	if(peakEnergy<972&&peakEnergy>961){xrays[8]++;}
        if(peakEnergy<1097&&peakEnergy>1081){xrays[9]++;}
        if(peakEnergy<1120&&peakEnergy>1110){xrays[10]++;}
	if(peakEnergy<1416&&peakEnergy>1403){xrays[11]++;}
	h1->Fill(peakEnergy);
      }
    }
  }//for int file
  //0-800
   std::cout<<"Number of X-Rays for the 40.1186 keV peak: "<<xrays[0]<<std::endl;
   std::cout<<"Number of X-Rays for the 121.78 keV peak: "<<xrays[1]<<std::endl;
   std::cout<<"Number of X-Rays for the 244.7 keV peak: "<<xrays[2]<<std::endl;
   std::cout<<"Number of X-Rays for the 344.28 keV peak: "<<xrays[3]<<std::endl;
   std::cout<<"Number of X-Rays for the 411.1165 keV peak: "<<xrays[4]<<std::endl;
   std::cout<<"Number of X-Rays for the 443.965 keV peak: "<<xrays[5]<<std::endl;
   std::cout<<"Number of X-Rays for the 778.91 keV peak: "<<xrays[6]<<std::endl;
   //800-2000
   std::cout<<"Number of X-Rays for the 867.380 keV peak: "<<xrays[7]<<std::endl;
   std::cout<<"Number of X-Rays for the 964.08 keV peak: "<<xrays[8]<<std::endl;
   std::cout<<"Number of X-Rays for the 1085.837 keV peak: "<<xrays[9]<<std::endl;
   std::cout<<"Number of X-Rays for the 1112.076 keV peak: "<<xrays[10]<<std::endl;
   std::cout<<"Number of X-Rays for the 1410.65 keV peak: "<<xrays[11]<<std::endl;
   
   //Fit Energy peak Europium
  TF1*Fitpeak1 = new TF1("Fitpeak1", "[0]*TMath::Gaus(x,[1],[2])", 20,60);
  Fitpeak1->SetParameters(8.53016e+02,40,-3.54625e+01);

  TF1*Fitpeak2 = new TF1("Fitpeak2", "[0]*TMath::Gaus(x,[1],[2])", 110, 130);
  Fitpeak2->SetParameters(4.93847e+02,120,-5.10241e+01);

  TF1*Fitpeak3 = new TF1("Fitpeak3", "[0]*TMath::Gaus(x,[1],[2])", 237,247 );
  //Fitpeak3->SetParameters(2.04668e+02,242,-5.13933e+02);
  Fitpeak3->SetParameters(1.19396e+02 ,242.2,3.6); 
  TF1*Fitpeak4 = new TF1("Fitpeak4", "[0]*TMath::Gaus(x,[1],[2])", 330, 350);
  Fitpeak4->SetParameters(2.04668e+02,340,-5.13933e+01);

  TF1*Fitpeak41 = new TF1("Fitpeak41", "[0]*TMath::Gaus(x,[1],[2])", 407, 415);
  Fitpeak41->SetParameters(14,411,-5);

  TF1*Fitpeak42 = new TF1("Fitpeak42", "[0]*TMath::Gaus(x,[1],[2])", 440, 448);
  Fitpeak42->SetParameters(50,444,2);

  TF1*Fitpeak5 = new TF1("Fitpeak5", "[0]*TMath::Gaus(x,[1],[2])", 770, 790);
  Fitpeak5->SetParameters(2.04668e+02,780,-5.13933e+01);
  
   /*
     TF1*Fitpeak60 = new TF1("Fitpeak60", "[0]*TMath::Gaus(x,[1],[2])", 865,875);
     Fitpeak60->SetParameters(8.53016e+02,870,-3.54625e+01);
  
     TF1*Fitpeak6 = new TF1("Fitpeak6", "[0]*TMath::Gaus(x,[1],[2])", 950,980);
     Fitpeak6->SetParameters(8.53016e+02,961,-3.54625e+01);
                   
     TF1*Fitpeak7 = new TF1("Fitpeak7", "[0]*TMath::Gaus(x,[1],[2])", 1060, 1100);
     Fitpeak7->SetParameters(4.93847e+02,1080,-5.10241e+01);
     
     TF1*Fitpeak8 = new TF1("Fitpeak8", "[0]*TMath::Gaus(x,[1],[2])", 1100,1120);
     Fitpeak8->SetParameters(2.04668e+02,1110,-5.13933e+01);
     
     TF1*Fitpeak9 = new TF1("Fitpeak9", "[0]*TMath::Gaus(x,[1],[2])", 1390, 1430);
     Fitpeak9->SetParameters(200,1408,3); 
   */
  h1->GetXaxis()->SetTitle("E [keV]");
  h1->SetTitle("");
  h1->SetStats(0);
  h1->Draw("same");
  
  
  h1->Fit(Fitpeak1,"0","",35, 43);
  h1->Fit(Fitpeak2,"0","",110, 130);
  h1->Fit(Fitpeak3,"0","",237, 247);
  h1->Fit(Fitpeak4,"0","",330, 350);
  h1->Fit(Fitpeak41,"0","",407, 415);
  h1->Fit(Fitpeak42,"0","",440, 448);
  h1->Fit(Fitpeak5,"0","",770, 790);

  Fitpeak1->SetLineColor(kRed);
  Fitpeak1->SetLineStyle(2);
  Fitpeak1->Draw("same");

  Fitpeak2->SetLineColor(kRed);
  Fitpeak2->SetLineStyle(2);
  Fitpeak2->Draw("same");

  Fitpeak3->SetLineColor(kRed);
  Fitpeak3->SetLineStyle(2);
  Fitpeak3->Draw("same");
 
  Fitpeak4->SetLineColor(kRed);
  Fitpeak4->SetLineStyle(2);
  Fitpeak4->Draw("same");

  Fitpeak41->SetLineColor(kRed);
  Fitpeak41->SetLineStyle(2);
  Fitpeak41->Draw("same");

  Fitpeak42->SetLineColor(kRed);
  Fitpeak42->SetLineStyle(2);
  Fitpeak42->Draw("same");
  
  Fitpeak5->SetLineColor(kRed);
  Fitpeak5->SetLineStyle(2);
  Fitpeak5->Draw("same");
  
  /*

  h1->Fit(Fitpeak60,"0","",865,875);
  h1->Fit(Fitpeak6,"0","",950,970);               
  h1->Fit(Fitpeak7,"0","",1060, 1100);
  h1->Fit(Fitpeak8,"0","",1100,1120);
  h1->Fit(Fitpeak9,"0","",1400,1420);                                                         

  Fitpeak60->SetLineColor(kRed);
  Fitpeak60->SetLineStyle(2);
  Fitpeak60->Draw("same");

  Fitpeak6->SetLineColor(kRed);
  Fitpeak6->SetLineStyle(2); 
  Fitpeak6->Draw("same");
  
  Fitpeak7->SetLineColor(kRed);
  Fitpeak7->SetLineStyle(2);
  Fitpeak7->Draw("same");

  Fitpeak8->SetLineColor(kRed);
  Fitpeak8->SetLineStyle(2);
  Fitpeak8->Draw("same");
  
  Fitpeak9->SetLineColor(kRed);
  Fitpeak9->SetLineStyle(2);
  Fitpeak9->Draw("same");
 
  //cout<<"Mean: "<<h1->GetMean()<<endl;
  //cout<<"RMS: "<<h1->GetRMS()<<endl;
  //cout<<"MeanError: "<<h1->GetMeanError()<<endl;
 
  */ 
  auto leg1 = new TLegend(0.6,0.5,0.85,0.9);
  leg1->AddEntry(h1, "raw data","f");
  leg1->AddEntry(Fitpeak1, "fit","l");
 
   TLine *line40=new TLine(40.1186,yy1[0],40.1186,yy1[1]);
  line40->Draw("same");
  line40->SetLineStyle(3);
  line40->SetLineColor(kBlue);
  line40->SetLineWidth(2);
  leg1->AddEntry(line40, "40.1186 keV","l");

  TLine *line121=new TLine(121.78,yy1[0],121.78,yy1[1]);
  line121->Draw("same");
  line121->SetLineStyle(3);
  line121->SetLineColor(kBlue);
  line121->SetLineWidth(2);
  leg1->AddEntry(line121, "121.78 keV","l");

  TLine *line244=new TLine(244.7,yy1[0],244.7,yy1[1]);
  line244->Draw("same");
  line244->SetLineStyle(3);
  line244->SetLineColor(kBlue);
  line244->SetLineWidth(2);
  leg1->AddEntry(line244, "244.7 keV","l");

  TLine *line344=new TLine(344.28,yy1[0],344.28,yy1[1]);
  line344->Draw("same");
  line344->SetLineStyle(3);
  line344->SetLineColor(kBlue);
  line344->SetLineWidth(2);
  leg1->AddEntry(line344, "344.28 keV","l");

  TLine *line411=new TLine(411.1165,yy1[0],411.1165,yy1[1]);
  line411->Draw("same");
  line411->SetLineStyle(3);
  line411->SetLineColor(kBlue);
  line411->SetLineWidth(2);
  leg1->AddEntry(line411, "411.1165 keV","l");

  TLine *line443=new TLine(443.965,yy1[0],443.965,yy1[1]);
  line443->Draw("same");
  line443->SetLineStyle(3);
  line443->SetLineColor(kBlue);
  line443->SetLineWidth(2);
  leg1->AddEntry(line244, "443.965 keV","l");

  TLine *line778=new TLine(778.91,yy1[0],778.91,yy1[1]);
  line778->Draw("same");
  line778->SetLineStyle(3);
  line778->SetLineColor(kBlue);
  line778->SetLineWidth(2);
  leg1->AddEntry(line778, "778.91 keV","l");
  /*
  TLine *line867=new TLine(867.380,yy1[0],867.380,yy1[1]);
  line867->Draw("same");
  line867->SetLineStyle(3);
  line867->SetLineColor(kBlue);
  line867->SetLineWidth(2);
  leg1->AddEntry(line867, "867.380 keV","l");
  
  TLine *line964=new TLine(964.08,yy1[0],964.08,yy1[1]);
  line964->Draw("same");
  line964->SetLineStyle(3);
  line964->SetLineColor(kBlue);
  line964->SetLineWidth(2);
  leg1->AddEntry(line964, "964.08 keV","l");

  TLine *line1085=new TLine(1085.837,yy1[0],1085.837,yy1[1]);
  line1085->Draw("same");
  line1085->SetLineStyle(3);
  line1085->SetLineColor(kBlue);
  line1085->SetLineWidth(2);
  leg1->AddEntry(line1085, "1085.837 keV","l");

  TLine *line1112=new TLine(1112.076,yy1[0],1112.076,yy1[1]);
  line1112->Draw("same");
  line1112->SetLineStyle(3);
  line1112->SetLineColor(kBlue);
  line1112->SetLineWidth(2);
  leg1->AddEntry(line1112, "1112.076 keV","l");

  TLine *line1408=new TLine(1408.13,yy1[0],1408.13,yy1[1]);
  line1408->Draw("same");
  line1408->SetLineStyle(3);
  line1408->SetLineColor(kBlue);
  line1408->SetLineWidth(2);
  leg1->AddEntry(line1408, "1408.013 keV","l");
  
  */

  leg1->Draw("same");
  
  //c1->Print("EnergyEu_run00110.pdf","pdf");
  //c1->Print("EnergyEu_run00110_secondpart_2.png","png");  
}
