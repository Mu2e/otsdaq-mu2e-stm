#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <iomanip>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TPaveStats.h"
#include "TGraph2D.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TROOT.h"

using namespace std;

void plot_Offset_Angle(double energy){

  gROOT->SetStyle("ATLAS");

  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadTopMargin(0.08);


  Int_t MyPalette[100];
  /* 
  Double_t red[9]   = { 0./255., 145./255., 180./255., 151./255., 86./255., 143./255., 171./255., 185./255.,22./255.};
  Double_t green[9] = { 147./255., 221./255., 93./255., 66./255., 183./255., 94./255., 147./255., 135./255.,108./255.};
  Double_t blue[9]  = { 233./255., 232./255., 236./255., 60./255., 246./255., 7./255., 178./255., 19./255.,9./255.};
*/

  Double_t red[9]   = { 0./255., 145./255., 180./255., 61./255., 86./255., 43./255., 71./255., 85./255.,22./255.};
  Double_t green[9] = { 147./255., 221./255., 93./255., 6./255., 83./255., 4./255., 7./255., 5./255.,8./255.};
  Double_t blue[9]  = { 233./255., 232./255., 236./255., 60./255., 0./255., 7./255., 8./255., 9./255.,10./255.};
  
  Double_t stops[9] = { 0.0000, 0.1250, 0.2500, 0.3750, 0.5000, 0.6250, 0.7500, 0.8750, 1.0000};
  Int_t FI = TColor::CreateGradientColorTable(9, stops, red, green, blue, 255, 100);
  for (int i=0;i<100;i++) MyPalette[i] = FI+i;


  gStyle->SetPalette(100, MyPalette);
 
 
  //int palette_number = 70;
  //int palette_number = 104; 
  //int palette_number = 58; 
  // int palette_number = 74;
  // gStyle->SetPalette(palette_number);

    
  TCanvas *c1  = new TCanvas("c1","c1",0,0,700,500);

  std::vector<double> angles, offsets, Efficiency;

  std::string angles_str[11]={"0","5","10","15","20","25","30","35","40","45","50"};
  //std::string offsets_str[21]={"-0.5", "-1", "-1.2", "-1.5", "-2", "-2.2", "-2.5", "-3", "-3.5", "-4", "0", "0.5", "1", "1.2", "1.5", "2", "2.2", "2.5", "3", "3.5", "4"};

  std::string offsets_str[18]={"-0.5", "-1", "-1.2", "-1.5", "-2", "-2.2", "-2.5", "0", "0.5", "1", "1.2", "1.5", "2", "2.2", "2.5", "3", "3.5", "4"};
  
  int Nangles = 11;
  int Noffsets = 18;
  
  double xx1[2]={0, 50};
  double yy1[2]={-2.5, 4};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitleColor(kBlack);
  graph1->GetXaxis()->SetTitleSize(0.04);
  graph1->GetXaxis()->SetLabelSize(0.04);
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetLabelSize(0.04);
  graph1->GetXaxis()->SetTitle("Angle [#circ]");
  graph1->GetYaxis()->SetTitle("Offset [cm]");
  graph1->GetYaxis()->SetTitleOffset(1.3);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
    
  double Nphot_gen = 1000000;

  angles.clear();
  offsets.clear();
  
  for(int i=0 ; i<Nangles ; i++ ){
    //for(int i=0 ; i<2 ; i++ ){

    std::cout<<" "<<std::endl;
    double angle_stod = std::stod(angles_str[i]);
    std::cout<<"Angle: "<<angle_stod<<std::endl;

    for(int j=0 ; j<Noffsets ; j++){
      //for(int j=0 ; j<2 ; j++ ){
      angles.push_back(angle_stod);
      offsets.push_back(std::stod(offsets_str[j]));
      
      std::cout<<"Offset: "<<std::stod(offsets_str[j])<<std::endl;
      std::stringstream streamenergy;
      double energyMeV = energy/1000;
      streamenergy << std::fixed << std::setprecision(3) << energyMeV;
      
      std::string filename = "/work/mu2e/data1/cgarcia/Systematics/AnglesOffsetStudy/Angle_Offset_"+std::to_string(int(energy))+"keV/"+angles_str[i]+"deg/10to6_"+streamenergy.str()+"MeV_0.99costheta_"+angles_str[i]+"yrot_"+offsets_str[j]+".root";
      
      std::cout<<filename<<std::endl;
      
      TFile* input = new TFile(filename.c_str(),"read");
      
      int nbins = 100;

      double xphotopeak_region_low = energy - 11.3 - 3;
      double xphotopeak_region_high = energy - 11.3 + 3;

      double count_photopeakhits = 0;
      
      double deposit_0energy_STM = 0;
        

      double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen;
      double preXpos,preYpos,preZpos,EdepEvent, StepLengthStep, EdepStep;
      int PDGID, EventID;
      
      std::vector<double> GeEdep, EventEdep, Numphot_photopeak;
      
      //Leemos el TTree
      TTree* tree1=(TTree*)input->Get("Events");
      TTree* tree2=(TTree*)input->Get("StepsGe");

      //Event
      tree1->SetBranchAddress("pxgen", &pxgen);
      tree1->SetBranchAddress("pygen", &pygen);
      tree1->SetBranchAddress("pzgen", &pzgen);
      tree1->SetBranchAddress("EdepEvent", &EdepEvent); //edep event in Ge
      tree1->SetBranchAddress("xgen", &xgen);
      tree1->SetBranchAddress("ygen", &ygen);
      tree1->SetBranchAddress("zgen", &zgen);

      //Ge
      tree2->SetBranchAddress("preXpos", &preXpos);
      tree2->SetBranchAddress("preYpos", &preYpos);
      tree2->SetBranchAddress("preZpos", &preZpos);
      tree2->SetBranchAddress("pre_px", &pre_px);
      tree2->SetBranchAddress("pre_py", &pre_py);
      tree2->SetBranchAddress("pre_pz", &pre_pz);
      tree2->SetBranchAddress("StepLengthStep", &StepLengthStep);
      tree2->SetBranchAddress("PDGID", &PDGID);
      tree2->SetBranchAddress("EventID", &EventID);
      tree2->SetBranchAddress("EdepStep", &EdepStep);
      
      int entries = tree1->GetEntries();
      std::cout<<entries<<std::endl;
      
      //Each entry is an event (a photon generated)
      for(unsigned long i=0; i<entries; i++)
	{
	  tree1->GetEntry(i);
  
	    if(( EdepEvent >= xphotopeak_region_low )&&( EdepEvent <= xphotopeak_region_high )){
	      count_photopeakhits++;
	  }

	}
      
      double eff = count_photopeakhits/Nphot_gen;
      Efficiency.push_back(eff);
      std::cout<<"Photopeak Efficiency: "<<eff<<std::endl;
      
      /*
      int entries2 = tree2->GetEntries();
      std::cout<<entries2<<std::endl;
  
      double Edep_Event_sum=0;
      
      double aux_EventID=0;
        
      for(unsigned long i=0; i<entries2; i++)
	{
	  tree2->GetEntry(i);
	  
	  if(aux_EventID==EventID) {
	    Edep_Event_sum = Edep_Event_sum + EdepStep;
	      
	    aux_EventID=EventID;
	  }
	  EventEdep.push_back(EdepEvent);
	}

      std::cout<<"size edep event: "<<EventEdep.size()<<" size edep ge event: "<<GeEdep.size()<<std::endl;
      */
      std::cout<<"Counting number of events in: "<<xphotopeak_region_low<<" and "<<xphotopeak_region_high<<": "<<count_photopeakhits<<std::endl;
      std::cout<<"Particles not depositing energy in the STM (0keV): "<<deposit_0energy_STM<<std::endl;
      
    } //Noffsets
  } //Nangles

  // Find the max element
  std::cout << "\nMax Element (efficiency)= " << *max_element(Efficiency.begin(), Efficiency.end()) << std::endl;
  

  //2D colz plot with the efficiency
  TGraph2D *gr = new TGraph2D(angles.size(),&angles[0],&offsets[0],&Efficiency[0]);
  gr->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
  gr->GetHistogram()->GetZaxis()->SetRangeUser(0,1);
  gr->Draw("same,colz");

  std::cout<<"size: "<<angles.size()<<std::endl;
  TLatex latex;
  latex.SetTextSize(0.04);
  latex.SetTextAlign(13);
  std::string namelatex = "Photopeak Efficiency at "+std::to_string(int(energy))+"keV (Collimator r=0.5642cm)";
  char* latex_char = const_cast<char*>(namelatex.c_str()); 
  latex.DrawLatex(0.1,4.5,latex_char);

  std::string png = "CollimatorOffset_Angle_Eff_"+std::to_string(int(energy))+"keV_Al_newpalette.png";
  char* png_char = const_cast<char*>(png.c_str());

  gPad->SetTicks();
  gPad->RedrawAxis();

  c1->Print(png_char);
}
