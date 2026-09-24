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

void plot_Signal_SimulationPeak(std::string pathfile) {
  gROOT->SetStyle("ATLAS");

  auto c1= new TCanvas("c1");

  double counts_K = 0;
  //Calibration (1)
  //double p0 = -300.9;
  //double p1 = -2.543;
  //calibration (2)
  //double p0 = -412;
  //double p1 = -2.411;
  //Best K calibration
  double p0 = -192.8;
  double p1 = -2.552;


  /* double range1_ADC = -3930;
  double range2_ADC = -3200;
  double range1_E = (range1_ADC-p0)/(p1);
  double range2_E = (range2_ADC-p0)/(p1);
  */
  double range1_E = 1400;
  double range2_E = 1470;

  double xx1[2]={range1_E,range2_E};
  double yy1[2]={0,20};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle("E [keV]");
  graph1->GetYaxis()->SetTitle("Counts / (427.0787s)");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  //Complete
  TH1F*hDATA = new TH1F("DATA","", 5200, 0, 3000);
  TH1F*hSIMULATION = new TH1F("SIMULATION","", 5200, 0, 3000);

  //Abrimos el txt
  fstream readfile;
  readfile.open(pathfile,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();


  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    cout<<name<<endl;
  }

  std::cout<<"Size: "<<file_name.size()<<std::endl;
  for (int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    cout<<path.c_str()<<endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("treeADC");
    double peaks;

    tree->SetBranchAddress("peaks",&peaks);

    unsigned long entries=tree->GetEntries();

    cout<<"entries: "<<entries<<endl;
    for(unsigned long i=0;i<entries;i++){
      tree->GetEntry(i);
      hDATA->Fill((peaks-p0)/p1);
      //if((peaks>=range1)&&(peaks<=range2)){counts_K++;}
    }
  }//for int file

  //Simulation sampling gaussian
  /*double NCounts = 394;
  //double NCounts = 199;
  double sigma_resol = 3.5;//HPGe resolution
  double meanE = 1450;//keV

  for(int i = 0; i< int(NCounts); i++){
    double c = gRandom->Gaus(1450,sigma_resol);
    hSIMULATION->Fill(c);
  }

  hSIMULATION->GetXaxis()->SetTitle("E [keV]");
  hSIMULATION->SetTitle("");
  hSIMULATION->SetLineColor(kCyan-3);
  hSIMULATION->Draw("HIST,same");
  */


  //Simulation from geant
  //std::string pathfileroot = "Rootfiles0,0,0.txt";
  std::string pathfileroot = "Rootfiles0,0,-10.txt";


  double sigma_resol = 3.5;//HPGe resolution

  fstream readfileroot;
  readfileroot.open(pathfileroot,ios::in);
  string nameroot;
  vector<string> file_nameroot;
  file_nameroot.clear();


  while(1){
    readfileroot>>nameroot;
    file_nameroot.push_back(nameroot);
    if(readfileroot.eof())break;
    cout<<nameroot<<endl;
  }

  std::cout<<"Size: "<<file_nameroot.size()<<std::endl;
  for (int file=0;file<(file_nameroot.size()-1);file++){
  ///for (int file=0;file<1;file++){
    string pathroot;
    cout<<"PATH: "<<pathroot<<endl;
    pathroot=file_nameroot[file];
    cout<<pathroot.c_str()<<endl;

    TFile *input=new TFile(pathroot.c_str());
    TTree* tree1=(TTree*)input->Get("Events");

    double EdepEvent;
    tree1->SetBranchAddress("EdepEvent", &EdepEvent);
    
    int entries=tree1->GetEntries();
    cout<<entries<<endl;

    //Each entry is an event (a photon generated)
    for(unsigned long i=0;i<entries;i++)
      {
	tree1->GetEntry(i);
	if(EdepEvent!=0){
	  //hSIMULATION->Fill(EdepEvent);
	  double smeared = gRandom->Gaus(EdepEvent,sigma_resol);
	  hSIMULATION->Fill(smeared);  
	}
      }


  }




   hSIMULATION->Scale(1./(4*500000));
   hSIMULATION->Scale(214692);
  hSIMULATION->GetXaxis()->SetTitle("E [keV]");
  hSIMULATION->SetTitle("");
  hSIMULATION->SetLineColor(kCyan-3);
  hSIMULATION->Draw("HIST,same");
  
  double Ncossour_to_Ncos = 1.26;
  double Nweight_source = (1-(1./1.26));
  hDATA->GetXaxis()->SetTitle("E [keV]");
  hDATA->SetTitle("");
  hDATA->Scale(1-(1./1.26));
  hDATA->Draw("HIST,same");




  
  double range1_dat=1440;
  double range2_dat=1460;  

  double range1_sim=1440;
  double range2_sim=1460;

  double integral_DATAK = hDATA->Integral(hDATA->FindFixBin(range1_dat), hDATA->FindFixBin(range2_dat));
  std::cout<<"Number of counts K Integral (DATA) between "<<range1_dat<<" and "<<range2_dat<<" keV: "<<integral_DATAK<<std::endl;

  double integral_SIMK = hSIMULATION->Integral(hSIMULATION->FindFixBin(range1_sim), hSIMULATION->FindFixBin(range2_sim));
  std::cout<<"Number of counts K Integral (SIMULATION) between "<<range1_sim<<" and "<<range2_sim<<" keV: "<<integral_SIMK<<std::endl;

  //std::cout<<"Number of counts K, between "<<range1<<" and "<<range2<<": "<<counts_K<<std::endl;

  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << sigma_resol;
  std::string sim_legend_str = "^{40}K Geant4 simulation #sigma="+stream.str()+" keV";
  char* sim_legend = const_cast<char*>(sim_legend_str.c_str());

  auto legend = new TLegend(0.2,0.55,0.65,0.9);
  legend->AddEntry(hDATA,"^{40}K data (from source)","F");
  legend->AddEntry(hSIMULATION,sim_legend,"F");
  legend->Draw("same");

  c1->Print("Resolutiongeant3.5_saltcyl(0,0,-10)_distancesalt_STM_30.395cm_bestKcalibration.png","png"); 

}
