#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TPaveStats.h"

using namespace std;

void plot_VD_EdepGe(std::string filename, std::string HPGe_rotation){

  bool plot4=false;
  bool plot1=true;
  
  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  TCanvas* c1;

  if(plot4==true){
    c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
    c1->Divide(2,2);
  }
  if(plot1==true){
    c1 = new TCanvas("");
  }
  TFile* input = new TFile(filename.c_str(),"read");

  double Nphot_gen = 1000000;
  bool electrons = false;
 
  //Energy of the file

  int pos1 = filename.find("10to") + 6;
  
  int pos2 = filename.find("MeV_");

  int diff = pos2-pos1;
  std::cout<<"pos1: "<<pos1<<" pos2: "<<pos2<<" diff: "<<diff<<std::endl;

  double energy = std::stod(filename.substr(pos1, diff));

  std::string plot_name = "10to6photons_"+filename.substr(pos1, diff)+"keV_099_1";

  if(electrons==true){plot_name = "10to6electrons_"+filename.substr(pos1, diff)+"keV_099_1";}

  std::string plot_name_png = plot_name+"_"+HPGe_rotation+"yrot_coll0_NOAl.png";

  char* imagename_png = const_cast<char*>(plot_name_png.c_str());

  //To keV
  energy = energy*1000;

  double mom = energy;

  std::cout<<"mom: "<<mom<<" keV"<<std::endl;

  
  //If electrons the kinetic energy is not the momentum
  if(electrons==true){
    energy = sqrt(energy*energy + 511*511) - 511;
  }

  std::cout<<"kinetic energy: "<<energy<<" keV"<<std::endl;
  
  double xrange_low = energy - 17;
  double xrange_high = energy ;
  int nbins = 100;

  if(electrons==true){ xrange_high = energy + 4; }
  
  double xsignalregion_low = 344;
  double xsignalregion_high = 350;

  double xphotopeak_region_low = energy - 11.3 - 3;
  double xphotopeak_region_high = energy - 11.3 + 3;
 

  double count_photopeakhits = 0;

  double deposit_0energy_STM = 0;
  
  TH1D* hEgen = new TH1D("hEgen", " ;p_{z, gen #gamma}; Events/#gamma gen", nbins, xrange_low, xrange_high);
  TH2D* hyx_VD = new TH2D("hyx", "xy position of the hit in each step", 1000, -10, 10, 1000, -10, 10);
  hyx_VD->GetXaxis()->SetTitle("x_{#gamma, VD} [cm]");
  hyx_VD->GetYaxis()->SetTitle("y_{#gamma, VD} [cm]");
  TH1D* hEtot_Ge = new TH1D("hEtot_Ge", "E deposited at the STM per event by the elec; E deposited in HPGe [keV]; Events/#gamma gen", nbins, 0, xrange_high);
  TH1D* hEtot_Ge_signalregion = new TH1D("hEtot_Ge_signalregion", "E deposited at the STM per event; E deposited in HPGe [keV]; Events/#gamma gen", nbins, xsignalregion_low, xsignalregion_high);

  hEgen->GetYaxis()->SetTitleOffset(1.7);
  
  //hE_phot_virtual->GetXaxis()->SetTitle("E_{#gamma} before HPGe (dark) and E deposited in HPGe (light) [keV]");
  //hE_phot_virtual->GetYaxis()->SetTitle("Events/X-ray");
  //hE_phot_virtual->GetYaxis()->SetTitleOffset(1.7);


  double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen;
  double preXpos,preYpos,preZpos,TrackLengthEvent,EdepEvent,TrackLengthEventgammas, StepLengthStep, EdepStep;
  int PDGID, EventID;

  //Leemos el TTree
  TTree* tree1=(TTree*)input->Get("Events");
  //TTree* tree2=(TTree*)input->Get("StepsVacuumVirtualPlane");  


  tree1->SetBranchAddress("pxgen", &pxgen);
  tree1->SetBranchAddress("pygen", &pygen);
  tree1->SetBranchAddress("pzgen", &pzgen);
  tree1->SetBranchAddress("EdepEvent", &EdepEvent);
  tree1->SetBranchAddress("xgen", &xgen);
  tree1->SetBranchAddress("ygen", &ygen);
  tree1->SetBranchAddress("zgen", &zgen);

  //Vacuum plane step
  /*tree2->SetBranchAddress("preXpos", &preXpos);
  tree2->SetBranchAddress("preYpos", &preYpos);
  tree2->SetBranchAddress("preZpos", &preZpos);
  tree2->SetBranchAddress("pre_px", &pre_px);
  tree2->SetBranchAddress("pre_py", &pre_py);
  tree2->SetBranchAddress("pre_pz", &pre_pz);
  tree2->SetBranchAddress("StepLengthStep", &StepLengthStep);
  tree2->SetBranchAddress("PDGID", &PDGID);
  tree2->SetBranchAddress("EventID", &EventID);
  tree2->SetBranchAddress("EdepStep", &EdepStep);
  */

  int entries = tree1->GetEntries();
  cout<<entries<<endl;
  unsigned long int event =0;
  
  unsigned long int edep_in_347keV =0;
  
  //Each entry is an event (a photon generated)
  for(unsigned long i=0; i<entries; i++)
    {
      tree1->GetEntry(i);
      double Ekingen = sqrt(pxgen*pxgen+pygen*pygen+pzgen*pzgen)*1000;
      if(electrons==true){ Ekingen = sqrt(Ekingen*Ekingen + 511*511) - 511;}
      //std::cout<<"Ekingen: "<<Ekingen<<", kin energy: "<<energy<<std::endl;
      if((Ekingen > (energy+0.1))||(Ekingen < (energy-0.1))){std::cout<<"WRONG energies...ERROR exit..."<<std::endl; exit(0);}
      hEgen->Fill(pzgen*1000);

      /*if(EdepEvent > (energy-11)){
	std::cout<<"event: "<<event<<" EdepEvent "<<EdepEvent<<" pxgen: "<<pxgen<<" pygen: "<<pygen<<" pzgen: "<<pzgen<<std::endl;
	}*/
      //if(EdepEvent!=0){
      //if((EdepEvent>=0)&&(EdepEvent<=energy)){
      if(EdepEvent>=0){
	
	hEtot_Ge->Fill(EdepEvent);

	if(( EdepEvent >= xphotopeak_region_low )&&( EdepEvent <= xphotopeak_region_high )){
	    count_photopeakhits++;
	  }
      }

      if((EdepEvent>=xsignalregion_low)&&(EdepEvent<=xsignalregion_high)){
        hEtot_Ge_signalregion->Fill(EdepEvent);
	edep_in_347keV++;
      }

      if(EdepEvent==0){
	deposit_0energy_STM++;
      }
      event++;
      //std::cout<<"Edep: "<<EdepEvent<<" keV"<<std::endl;}
    }
  

  /*
  int entries2 = tree2->GetEntries();
  cout<<entries2<<endl;

  double Ephoton_Energy;

  double aux_EventID=1;
  
  for(unsigned long i=0; i<entries2; i++)
    {
      tree2->GetEntry(i);
	//Just 1st plane
	//if((PDGID==22)&&(preZpos < -0.195)){
      //Not counting particles twice
      if(aux_EventID!=EventID) {
          hyx_VD->Fill(preXpos, preYpos);
	  aux_EventID=EventID;
      }
      //}
    }
  */


  double hits_integral = hEtot_Ge->Integral(hEtot_Ge->FindFixBin(xphotopeak_region_low), hEtot_Ge->FindFixBin(xphotopeak_region_high), "");
  std::cout<<"Integral between: "<<xphotopeak_region_low<<" and "<<xphotopeak_region_high<<": "<<hits_integral<<std::endl;
  std::cout<<"Counting number of events in: "<<xphotopeak_region_low<<" and "<<xphotopeak_region_high<<": "<<count_photopeakhits<<std::endl;
  std::cout<<"Particles not depositing energy in the STM (0keV): "<<deposit_0energy_STM<<std::endl;
  std::cout<<"Particles depositing energy in the 347keV range: "<<edep_in_347keV<<std::endl;
    
  if(plot4==true){
    
  TPaveStats *stat[4];

  c1->cd(1);
  hEgen->Scale(1./Nphot_gen);
  hEgen->SetFillStyle(3001);
  hEgen->SetLineColor(kBlack);
  hEgen->SetLineWidth(1);
  hEgen->SetFillColor(kBlue);

  TAxis *X = hEgen->GetXaxis();
  X->SetNdivisions(5,3,0);
  hEgen->Draw("HIST");

  gPad->Update();

  stat[0] = (TPaveStats*)hEgen->FindObject("stats"); 
  stat[0]->SetY1NDC(.7);
  stat[0]->SetY2NDC(.91);
  stat[0]->SetX1NDC(0.2); //new x start position
  stat[0]->SetX2NDC(0.6);
  stat[0]->SetTextSize(0.043);
  stat[0]->SetTextColor(kBlue);
  stat[0]->Draw("same");
  
  std::string str_latex1 = "E_{#gamma}="+std::to_string(int(energy))+" keV";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.2,.57,char_latex1);

  c1->cd(2);
  hyx_VD->Scale(1./Nphot_gen);
  hyx_VD->SetFillStyle(3001);
  hyx_VD->SetLineColor(kBlack);
  hyx_VD->SetLineWidth(1);
  hyx_VD->SetFillColor(kCyan-3);
  hyx_VD->Draw("HIST,SAMES");

  gPad->Update();

  stat[1] = (TPaveStats*)hyx_VD->FindObject("stats");
  stat[1]->SetY1NDC(.7);
  stat[1]->SetY2NDC(.91);
  stat[1]->SetX1NDC(0.2); //new x start position
  stat[1]->SetX2NDC(0.6);
  stat[1]->SetTextSize(0.043);
  stat[1]->SetTextColor(kAzure);
  stat[1]->Draw("same");

  c1->cd(3);

  hEtot_Ge->Scale(1./Nphot_gen);
  hEtot_Ge->SetFillStyle(3001);
  hEtot_Ge->SetLineColor(kBlack);
  hEtot_Ge->SetLineWidth(1);
  hEtot_Ge->SetFillColor(kCyan-3);
  
  TAxis *X1 = hEtot_Ge->GetXaxis();
  X1->SetNdivisions(5,3,0);
  hEtot_Ge->Draw("HIST,SAMES");

  gPad->Update();

  stat[2] = (TPaveStats*)hEtot_Ge->FindObject("stats");
  stat[2]->SetY1NDC(.7);
  stat[2]->SetY2NDC(.91);
  stat[2]->SetX1NDC(0.2); //new x start position
  stat[2]->SetX2NDC(0.6);
  stat[2]->SetTextSize(0.043);
  stat[2]->SetTextColor(kCyan-3);
  stat[2]->Draw("same");

  std::string str_latex2 = "Peak integral="+std::to_string(int(count_photopeakhits));
  char* char_latex2 = const_cast<char*>(str_latex2.c_str());
  TLatex latex2;
  latex2.DrawLatexNDC(.2,.57,char_latex2);
  
  c1->cd(4);

  hEtot_Ge_signalregion->Scale(1./Nphot_gen);
  hEtot_Ge_signalregion->SetFillStyle(3001);
  hEtot_Ge_signalregion->SetLineColor(kBlack);
  hEtot_Ge_signalregion->SetLineWidth(1);
  hEtot_Ge_signalregion->SetFillColor(kCyan-3);
  hEtot_Ge_signalregion->Draw("HIST,SAMES");
  

  gPad->Update();

  stat[3] = (TPaveStats*)hEtot_Ge_signalregion->FindObject("stats");
  stat[3]->SetY1NDC(.7);
  stat[3]->SetY2NDC(.91);
  stat[3]->SetX1NDC(0.2); //new x start position 
  stat[3]->SetX2NDC(0.6);
  stat[3]->SetTextSize(0.043);
  stat[3]->SetTextColor(kCyan-3);
  stat[3]->Draw("same");
  }




  if(plot1==true){
    if(electrons==true){ hEtot_Ge->GetYaxis()->SetTitle("Events/e^{-} gen");}
    TPaveStats *stat[1];
    hEtot_Ge->Scale(1./Nphot_gen);
    hEtot_Ge->SetFillStyle(3001);
    hEtot_Ge->SetLineColor(kBlack);
    hEtot_Ge->SetLineWidth(1);
    hEtot_Ge->SetFillColor(kCyan-3);
    

    TAxis *X1 = hEtot_Ge->GetXaxis();
    X1->SetNdivisions(5,3,0);
    hEtot_Ge->Draw("HIST,SAMES");

    gPad->SetLogy();
    
    gPad->Update();
    /*
    stat[0] = (TPaveStats*)hEtot_Ge->FindObject("stats");
    stat[0]->SetY1NDC(.7);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2); //new x start position
    stat[0]->SetX2NDC(0.45);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kCyan-3);
    stat[0]->Draw("same");
    */
    
    std::string energy_str = std::to_string(int(energy));
    std::string mom_str = std::to_string(int(mom));
    
    if(electrons==true){

      std::stringstream streamen;
      streamen << std::fixed << std::setprecision(3) << energy;
  
      std::string str_latex = "E_{kin e^{-}}="+streamen.str()+" (p_{e^{-}}="+mom_str+") keV";
      char* char_latex = const_cast<char*>(str_latex.c_str());
      TLatex latex;
      latex.DrawLatexNDC(.5,.82,char_latex);
    }
    else{

      double fraction = count_photopeakhits/Nphot_gen;
      double fraction_error = fraction * sqrt(count_photopeakhits)/count_photopeakhits;

      std::cout<<fraction_error<<" "<<round(fraction_error)<<std::endl;
      std::stringstream stream_fraction, stream_fraction_err;
      stream_fraction << std::fixed << std::setprecision(3) << fraction;
      stream_fraction_err << std::fixed << std::setprecision(3) << fraction_error;
      
      std::string str_latex = "#splitline{E_{#gamma} = "+energy_str+" keV}{Peak integral = "+stream_fraction.str()+"#pm"+stream_fraction_err.str()+"}";
      char* char_latex = const_cast<char*>(str_latex.c_str());
      TLatex latex;
      latex.DrawLatexNDC(.2,.82,char_latex);
    }
  }
  
  c1->Print("10to6photons_0.8keV_099_1_0yrot_coll0_Al_rmstats.png");
  //c1->Print(imagename_pdf);
  
}
