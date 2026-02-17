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
#include "TSystem.h"

using namespace std;

void plot_MeanEnergy_Al_NoAl(std::string txtroot_location0, std::string txtroot_location1, int branch){

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(11110);

  TCanvas* c1 = new TCanvas();

  //gSystem->RedirectOutput("Electrons_NoAl_1.5mmAl.log");
  
  //Open txt
  fstream readfile[2];
  readfile[0].open(txtroot_location0,ios::in);
  readfile[1].open(txtroot_location1,ios::in);
  
  string name[2];
  
  vector<string> file_name[2];
  file_name[0].clear();
  file_name[1].clear();
  
  //Read each art file from txt
  while(1){
    readfile[0]>>name[0];
    file_name[0].push_back(name[0]);
    if(readfile[0].eof())break;
    std::cout<<name[0]<<std::endl;
  }

  std::cout<<" "<<std::endl;
  
  while(1){
    readfile[1]>>name[1];
    file_name[1].push_back(name[1]);
    if(readfile[1].eof())break;
    std::cout<<name[1]<<std::endl;
  }


  double pre_px,pre_py,pre_pz,pxgen,pygen,pzgen,xgen,ygen,zgen, EdepEvent;
  int nbins = 100;
  unsigned long int particles_gen = 1000000;
 
  TH1D* hEtot_Ge[2];
  TFile *input[2];
  TTree* tree[2];
   
  std::vector<double> momparticle, momparticle_error;

  std::vector<double> meanE0, meanE1, diffv, countphotopeak0, countphotopeak1, deposit0energy0, deposit0energy1, depositing347keVrange0, depositing347keVrange1;
  std::vector<double> meanE_error0, meanE_error1, diff_error;

   bool electrons = true;
   
  for (long unsigned int file=0;file<(file_name[0].size()-1);file++){
  //for (long unsigned int file=0;file<1;file++){
   string path[2];
   
   path[0]=file_name[0].at(file);
   std::cout<<path[0].c_str()<<std::endl;

   path[1]=file_name[1].at(file);
   std::cout<<path[1].c_str()<<std::endl;


   int pos1 = file_name[0].at(file).find("10to") + 6;

   int pos2 = file_name[0].at(file).find("MeV_");

   int diff = pos2-pos1;
   std::cout<<"pos1: "<<pos1<<" pos2: "<<pos2<<" diff: "<<diff<<std::endl;

   double mom = std::stod(file_name[0].at(file).substr(pos1, diff));

   //In MeV
   momparticle.push_back(mom);
   momparticle_error.push_back(0);

   double energy;
   
   if(electrons==true){
     //Electrons: energy is kinetic energy
     energy = sqrt(mom*mom + 0.511*0.511) - 0.511;
   }
   else{
     //Photons: energy is kinetic energy = mom
     energy = mom;
   }
   
   //In MeV
   double xrange_high = energy + 0.5;

   double xsignalregion_low = 0.347 - 0.0113 - 0.003;
   double xsignalregion_high = 0.347 - 0.0113 + 0.003;

   double xphotopeak_region_low = energy - 0.0113 - 0.003;
   double xphotopeak_region_high = energy - 0.0113 + 0.003;

   double count_photopeakhits[2];
   count_photopeakhits[0] = 0;
   count_photopeakhits[1] = 0;
   
   double deposit_0energy_STM[2];
   deposit_0energy_STM[0] = 0;
   deposit_0energy_STM[1] = 0;

   double deposit_347keVregion[2];
   deposit_347keVregion[0] = 0;
   deposit_347keVregion[1] = 0;
   
   hEtot_Ge[0] = new TH1D("hEtot_Ge0", "E deposited at the STM per event by the elec", nbins, 0, xrange_high);
   hEtot_Ge[1] = new TH1D("hEtot_Ge1", "E deposited at the STM per event by the elec", nbins, 0, xrange_high);
  
    
   int entries[2];
   input[0]=new TFile(path[0].c_str());   
   tree[0]=(TTree*)input[0]->Get("Events");
   tree[0]->SetBranchAddress("pxgen", &pxgen);
   tree[0]->SetBranchAddress("pygen", &pygen);
   tree[0]->SetBranchAddress("pzgen", &pzgen);
   tree[0]->SetBranchAddress("EdepEvent", &EdepEvent);
   tree[0]->SetBranchAddress("xgen", &xgen);
   tree[0]->SetBranchAddress("ygen", &ygen);
   tree[0]->SetBranchAddress("zgen", &zgen);
   
   entries[0] = tree[0]->GetEntries();
   
   for(unsigned long i=0;i<entries[0];i++){
     
     tree[0]->GetEntry(i);
     
     if(EdepEvent==0){
        deposit_0energy_STM[0]++;
      }
     else{
       //In MeV
       double edepMeV = EdepEvent/1000;
       
       hEtot_Ge[0]->Fill(edepMeV);
       
       if(( edepMeV >= xphotopeak_region_low )&&( edepMeV <= xphotopeak_region_high )){
	 count_photopeakhits[0]++;
       }
       if(( edepMeV >= xsignalregion_low )&&( edepMeV <= xsignalregion_high )){
	 deposit_347keVregion[0]++;
       }
	 
     } //else
   }
   
   input[1]=new TFile(path[1].c_str());	
   tree[1]=(TTree*)input[1]->Get("Events");
   tree[1]->SetBranchAddress("pxgen", &pxgen);
   tree[1]->SetBranchAddress("pygen", &pygen);
   tree[1]->SetBranchAddress("pzgen", &pzgen);
   tree[1]->SetBranchAddress("EdepEvent", &EdepEvent);
   tree[1]->SetBranchAddress("xgen", &xgen);
   tree[1]->SetBranchAddress("ygen", &ygen);
   tree[1]->SetBranchAddress("zgen", &zgen);

   entries[1] = tree[1]->GetEntries();
   

   for(unsigned long i=0;i<entries[1];i++){
     
     tree[1]->GetEntry(i);

     if(EdepEvent==0){
       deposit_0energy_STM[1]++;
     }
     else{
       //In MeV
       double edepMeV = EdepEvent/1000;
       
       hEtot_Ge[1]->Fill(edepMeV);

       if(( edepMeV >= xphotopeak_region_low )&&( edepMeV <= xphotopeak_region_high )){
	 count_photopeakhits[1]++;
       }
       if(( edepMeV >= xsignalregion_low )&&( edepMeV <= xsignalregion_high )){
         deposit_347keVregion[1]++;
       }
     } //else
   }


   double mean[2];
   double errormean[2];
    
   mean[0] = hEtot_Ge[0]->GetMean();
   mean[1] = hEtot_Ge[1]->GetMean();
   errormean[0] = hEtot_Ge[0]->GetMeanError();
   errormean[1] = hEtot_Ge[1]->GetMeanError();
    
   meanE0.push_back(mean[0]);
   meanE1.push_back(mean[1]);
   meanE_error0.push_back(errormean[0]);
   meanE_error1.push_back(errormean[1]);

   countphotopeak0.push_back(count_photopeakhits[0]);
   countphotopeak1.push_back(count_photopeakhits[1]);
   deposit0energy0.push_back(deposit_0energy_STM[0]);
   deposit0energy1.push_back(deposit_0energy_STM[1]);
   depositing347keVrange0.push_back(deposit_347keVregion[0]);
   depositing347keVrange1.push_back(deposit_347keVregion[1]);
      
   double difference = mean[0] - mean[1];
   diffv.push_back(difference);
   diff_error.push_back(0);

   std::cout<<"kin Energy: "<<energy<<" MeV, mom: "<<mom<<std::endl;
   std::cout<<"Mean E (no al): "<<mean[0]<<" (al): "<<mean[1]<<" MeV"<<std::endl;
   std::cout<<"Counts in photopeak (no al): "<<count_photopeakhits[0]<<" (al): "<<count_photopeakhits[1]<<std::endl;
   std::cout<<"Deposit 0 energy at STM (no al): "<<deposit_0energy_STM[0]<<" (al): "<<deposit_0energy_STM[1]<<std::endl;
   std::cout<<"Counts in 347keV range (no al): "<<deposit_347keVregion[0]<<" (al): "<<deposit_347keVregion[1]<<std::endl;
   std::cout<<" "<<std::endl;
   
   input[0]->Close();
   input[1]->Close();
   delete gROOT->FindObject("hEtot_Ge0");
   delete gROOT->FindObject("hEtot_Ge1");
  }
 
  TGraphErrors *greff[2];

  int N;


  
  //Plot the mean energy at the STM as a function of the incident energy
  if(branch==0){
    
    std::cout<<"mean energy at the STM as a function of the incident energy"<<std::endl;
    N = meanE0.size();
    /*
    greff[0] = new TGraphErrors( N, &momparticle[0], &meanE0[0], &momparticle_error[0], &meanE_error0[0]);
    greff[0]->Print("");
    greff[1] = new TGraphErrors( N, &momparticle[0], &meanE1[0], &momparticle_error[0], &meanE_error1[0]);
    greff[1]->Print("");
    
    greff[0]->SetMarkerStyle(21);
    greff[0]->SetMarkerColor(kOrange-9);
    greff[0]->SetLineColor(kOrange-9);
    greff[0]->SetLineWidth(2);
    greff[0]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[0]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]");}
    greff[0]->GetYaxis()->SetTitle("<E_{STM}> [MeV]");
    //greff[0]->Draw("same,ap");

    greff[1]->SetMarkerStyle(21);
    greff[1]->SetMarkerColor(kOrange-9);
    greff[1]->SetLineColor(kOrange-9);
    greff[1]->SetLineWidth(2);
    greff[1]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[1]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); }
    greff[1]->GetYaxis()->SetTitle("<E_{STM}> (+1.5mm Al) [MeV]");
    greff[1]->Draw("same,ap");
    */
    

    
    greff[0] = new TGraphErrors( N, &momparticle[0], &diffv[0], &momparticle_error[0], &diff_error[0]);
    greff[0]->Print("");
    greff[0]->SetMarkerStyle(21);
    greff[0]->SetMarkerColor(kViolet-9);
    greff[0]->SetLineColor(kViolet-9);
    greff[0]->SetLineWidth(2);
    greff[0]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[0]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); } 
    greff[0]->GetYaxis()->SetTitle("#Delta<E_{STM}> (No Al-Al layer) [MeV]");
    greff[0]->Draw("same,ap");
    
    std::cout<<" "<<std::endl;
    }


  

  //Plot the photopeak efficiency as a function of the incident energy
  if(branch==1){

    std::cout<<"photopeak efficiency as a function of the incident energy"<<std::endl;
    
    N = countphotopeak0.size();

    std::vector<double> photopeakeff0, photopeakeff1;
    std::vector<double> photopeakeff0_error, photopeakeff1_error;

    for(unsigned long int i = 0; i < N ; i++){

      double eff0 = countphotopeak0.at(i) / particles_gen;

      double eff1 = countphotopeak1.at(i) / particles_gen;

      double eff0_error = sqrt( eff0 * (1-eff0) / particles_gen);
      double eff1_error	= sqrt( eff1 * (1-eff1) / particles_gen);
      
      photopeakeff0.push_back(eff0);
      photopeakeff1.push_back(eff1);

      photopeakeff0_error.push_back(eff0_error);
      photopeakeff1_error.push_back(eff1_error);
    }

    std::cout<<"No Al:"<<std::endl;
    greff[0] = new TGraphErrors( N, &momparticle[0], &photopeakeff0[0], &momparticle_error[0], &photopeakeff0_error[0]);
    greff[0]->Print("");
    std::cout<<"Al:"<<std::endl;
    greff[1] = new TGraphErrors( N, &momparticle[0], &photopeakeff1[0], &momparticle_error[0], &photopeakeff1_error[0]);
    greff[1]->Print("");

    greff[0]->SetMarkerStyle(21);
    greff[0]->SetMarkerColor(kBlue-9);
    greff[0]->SetLineColor(kBlue-9);
    greff[0]->SetLineWidth(2);
    greff[0]->GetYaxis()->SetRangeUser(0,1);
    greff[0]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[0]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); }
    greff[0]->GetYaxis()->SetTitle("Photopeak fraction at STM");
    //greff[0]->Draw("same,ap");

    greff[1]->SetMarkerStyle(21);
    greff[1]->SetMarkerColor(kBlue-9);
    greff[1]->SetLineColor(kBlue-9);
    greff[1]->GetYaxis()->SetRangeUser(0,1);
    greff[1]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[1]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); }
    greff[1]->GetYaxis()->SetTitle("Photopeak fraction at STM (+1.5mm Al)");
    greff[1]->SetLineWidth(2);
    greff[1]->Draw("same,ap");

    std::cout<<""<<std::endl;
  }


  
  //Plot number of particles depositing 0 energy in the STM
   if(branch==2){

     std::cout<<"number of particles depositing 0 energy in the STM"<<std::endl;
     N = deposit0energy0.size();

    std::vector<double> frac_deposit0energy0, frac_deposit0energy1;
    std::vector<double> frac_deposit0energy0_error, frac_deposit0energy1_error;

    for(unsigned long int i = 0; i < N ; i++){

      double eff0 = deposit0energy0.at(i) / particles_gen;

      double eff1 = deposit0energy1.at(i) / particles_gen;

      double eff0_error = sqrt( eff0 * (1-eff0) / particles_gen);
      double eff1_error = sqrt( eff1 * (1-eff1) / particles_gen);

      frac_deposit0energy0.push_back(eff0);
      frac_deposit0energy1.push_back(eff1);

      frac_deposit0energy0_error.push_back(eff0_error);
      frac_deposit0energy1_error.push_back(eff1_error);
    }

    std::cout<<"No Al:"<<std::endl;
    greff[0] = new TGraphErrors( N, &momparticle[0], &frac_deposit0energy0[0], &momparticle_error[0], &frac_deposit0energy0_error[0]);
    greff[0]->Print("");
    std::cout<<"Al:"<<std::endl;
    greff[1] = new TGraphErrors( N, &momparticle[0], &frac_deposit0energy1[0], &momparticle_error[0], &frac_deposit0energy1_error[0]);
    greff[1]->Print("");

    greff[0]->SetMarkerStyle(21);
    greff[0]->SetMarkerColor(kGreen-9);
    greff[0]->SetLineColor(kGreen-9);
    greff[0]->SetLineWidth(2);
    greff[0]->GetYaxis()->SetRangeUser(0,0.5);
    greff[0]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[0]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]");
    }
    greff[0]->GetYaxis()->SetTitle("Fraction not depositing E at STM");
    //greff[0]->Draw("same,ap");

    greff[1]->SetMarkerStyle(21);
    greff[1]->SetMarkerColor(kGreen-9);
    greff[1]->SetLineColor(kGreen-9);
    greff[1]->GetYaxis()->SetRangeUser(0,0.5);
    greff[1]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){
      greff[1]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]");
      greff[1]->GetYaxis()->SetRangeUser(0,1);
    }
    greff[1]->GetYaxis()->SetTitle("Fraction not depositing E at STM (+1.5mm Al)");
    greff[1]->SetLineWidth(2);
    greff[1]->Draw("same,ap");

    std::cout<<""<<std::endl;
   }




  
  //Plot electrons depositing energy in the 347keV range
  if( branch==3 ){

    std::cout<<"electrons depositing energy in the 347keV range"<<std::endl;
    N = depositing347keVrange0.size();
    
    std::vector<double> frac_depositing347keVrange0, frac_depositing347keVrange1;
    std::vector<double> frac_depositing347keVrange0_error, frac_depositing347keVrange1_error;

    for(unsigned long int i = 0; i < N ; i++){

      double eff0 = depositing347keVrange0.at(i) / particles_gen;

      double eff1 = depositing347keVrange1.at(i) / particles_gen;

      double eff0_error = sqrt( eff0 * (1-eff0) / particles_gen);
      double eff1_error = sqrt( eff1 * (1-eff1) / particles_gen);

      frac_depositing347keVrange0.push_back(eff0);
      frac_depositing347keVrange1.push_back(eff1);

      frac_depositing347keVrange0_error.push_back(eff0_error);
      frac_depositing347keVrange1_error.push_back(eff1_error);
    }

    std::cout<<"No Al:"<<std::endl;
    greff[0] = new TGraphErrors( N, &momparticle[0], &frac_depositing347keVrange0[0], &momparticle_error[0], &frac_depositing347keVrange0_error[0]);
    greff[0]->Print("");
    std::cout<<"Al:"<<std::endl;
    greff[1] = new TGraphErrors( N, &momparticle[0], &frac_depositing347keVrange1[0], &momparticle_error[0], &frac_depositing347keVrange1_error[0]);
    greff[1]->Print("");

    greff[0]->SetMarkerStyle(21);
    greff[0]->SetMarkerColor(kRed-9);
    greff[0]->SetLineColor(kRed-9);
    greff[0]->SetLineWidth(2);
    greff[0]->GetYaxis()->SetRangeUser(0,0.016);
    greff[0]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[0]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); }
    greff[0]->GetYaxis()->SetTitle("Fraction contributing to 347keV");
    //greff[0]->Draw("same,ap");

    greff[1]->SetMarkerStyle(21);
    greff[1]->SetMarkerColor(kRed-9);
    greff[1]->SetLineColor(kRed-9);
    greff[1]->GetYaxis()->SetRangeUser(0,0.004);
    greff[1]->GetXaxis()->SetTitle("E_{#gamma} [MeV]");
    if( electrons == true){ greff[1]->GetXaxis()->SetTitle("p_{e^{-}} [MeV]"); }
    greff[1]->GetYaxis()->SetTitle("Fraction contributing to 347keV (+1.5mm Al)");
    greff[1]->SetLineWidth(2);
    greff[1]->Draw("same,ap");

    std::cout<<" "<<std::endl;
  }

  c1->Print("GenElectros_deposit_in_347keVrangeSTM_E_Al.png");

}
