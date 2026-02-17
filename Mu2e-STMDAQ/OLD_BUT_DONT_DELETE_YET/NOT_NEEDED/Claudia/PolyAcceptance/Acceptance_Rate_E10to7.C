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
#include <iomanip>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TRandom.h"
#include "TFitResult.h"
#include "TMatrixD.h"

#define mm_to_cm 10
#define PI 3.14159265359

//================================================================
void PlotAcceptance_E(std::string ArtFiles_location, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, double cutVD10, double branch){

  gROOT->SetStyle("ATLAS");

  if((branch==0)||(branch==1)||(branch==2)||(branch==4)){
    gStyle->SetOptStat(1110);
    gStyle->SetOptFit(01111);
  }
  //gStyle->SetOptStat(1110);

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  //gPad->SetLogx();

  int N = 0;

  double E[15] = {0.05, 0.07, 0.09, 0.2, 0.347, 0.5, 0.7, 0.9, 1, 1.5, 2, 2.5, 3, 4, 5}; 
  double E_error[15];
  double acceptance_phot[15], acceptance[15], gammasdet_stm[15];
  double acceptance_error[15], gammasdet_stm_error[15];  
  double acceptance_VD10_STM[15],acceptance_VD10_STM_error[15]; 
  double acceptance_xray[15],acceptance_xray_error[15];
  
  double nparticles_sim_costhetarange = 10000000;

  double xrayphotons = nparticles_sim_costhetarange*4/((-0.99999+1)*2);
    
  double nparticles_md2020_costhetarange , nparticles_md2020_full;
  double Acc, deltaAcc;
  
  //cut 7.50224 cut VD10
  if( cutVD10 == 7.50224 ){
    nparticles_md2020_costhetarange = 263;
    nparticles_md2020_full = 1414626;
  }
  
  //no cut VD10
  if( cutVD10 == 0 ){
    nparticles_md2020_costhetarange = 269;
    nparticles_md2020_full = 8914495;
  }
  
  //cut 7.86 VD10
  if( cutVD10 == 7.86 ){
    nparticles_md2020_costhetarange = 263;
    nparticles_md2020_full = 1493267;
  }
   
  //cut 5.5cm VD10
  if( cutVD10 == 5.5 ){
    nparticles_md2020_costhetarange = 237;
    nparticles_md2020_full = 922836;
  }

  //cut 5cm VD10
  if( cutVD10 == 5 ){
    nparticles_md2020_costhetarange = 218;
    nparticles_md2020_full = 794536;
  }
  
  //cut 4.5cm VD10
  if( cutVD10 == 4.5 ){
    nparticles_md2020_costhetarange = 190;
    nparticles_md2020_full = 665589;
  }

  //cut 3.5cm VD10
  if( cutVD10 == 3.5 ){
    nparticles_md2020_costhetarange = 121;
    nparticles_md2020_full = 417873;
  }

  //cut 2.5cm VD10
  if( cutVD10 == 2.5 ){
    nparticles_md2020_costhetarange = 30;
    nparticles_md2020_full = 200474;
 }


  double detphot_VD10_VD89[15]={224, 335, 470, 1038, 1702, 2358, 3013, 3576, 3895, 5210, 6080, 6950, 7682, 8687, 9444};
  double detphot_VD10_VD90[15]={207, 348, 464, 1083, 1652, 2357, 2996, 3828, 3968, 5179, 6100, 7006, 7647, 8982, 9310};
  
  
  for(int i = 0; i < 15; i++){

    E_error[i] = 0;
    /*
    //Introduce the number of photons in costheta vs. the number of photons in the full range from MDCD2020 as energy dependent
     if(E[i]<0.1){
      nparticles_md2020_costhetarange = 64;
      nparticles_md2020_full = 480178;
    }
    else if((E[i]>=0.1)&&(E[i]<0.3)){
      nparticles_md2020_costhetarange = 42;
      nparticles_md2020_full = 258533;
    }
    else if((E[i]>=0.3)&&(E[i]<0.4)){
      nparticles_md2020_costhetarange = 15;
      nparticles_md2020_full = 66424;
    }
    else if((E[i]>=0.4)&&(E[i]<0.5)){
      nparticles_md2020_costhetarange = 11;
      nparticles_md2020_full = 48253;
    }
    else if((E[i]>=0.5)&&(E[i]<1)){
      nparticles_md2020_costhetarange = 77;
      nparticles_md2020_full = 272177;
    }
    else{
      nparticles_md2020_costhetarange = 42;
      nparticles_md2020_full = 211057;
      }
*/
    

    Acc = nparticles_md2020_costhetarange / nparticles_md2020_full; //p binomial error
    deltaAcc = sqrt(Acc*(1-Acc)/nparticles_md2020_full);
    
    //Acc=0.0001719;
    //deltaAcc=0.00001102;
    
     std::cout<<"E[i]: "<<E[i]<<std::endl;
     std::cout<<"ratio MDC2020: "<<Acc<<" +- "<<deltaAcc<<" binomial error"<<std::endl;
     double nparticles_sim_full = nparticles_sim_costhetarange / Acc;
     double nparticles_sim_full_error = nparticles_sim_full * deltaAcc / Acc;
     std::cout<<"Total number of particles in the acceptance for this energy: "<<nparticles_sim_full<<" +- "<<nparticles_sim_full_error<<std::endl;


     gammasdet_stm[i] = (detphot_VD10_VD89[i]+detphot_VD10_VD90[i])/2;     
     
     gammasdet_stm_error[i] = sqrt(detphot_VD10_VD89[i]+detphot_VD10_VD90[i])/2;

     acceptance[i] = gammasdet_stm[i] / nparticles_sim_full;

     std::cout<<"gammas det stm: "<<gammasdet_stm[i]<<" +- "<<gammasdet_stm_error[i]<<" Acc: "<<Acc<<" total photons gen: "<<nparticles_sim_full<<" total acceptance: "<<acceptance[i]<<std::endl;
       
     acceptance_error[i] = acceptance[i] * sqrt((nparticles_sim_full_error/nparticles_sim_full)*(nparticles_sim_full_error/nparticles_sim_full)+(gammasdet_stm_error[i]/gammasdet_stm[i])*(gammasdet_stm_error[i]/gammasdet_stm[i])) ;


     acceptance_VD10_STM[i] = gammasdet_stm[i]/nparticles_sim_costhetarange ;
     acceptance_VD10_STM_error[i] = sqrt(acceptance_VD10_STM[i]*(1-acceptance_VD10_STM[i])/nparticles_sim_costhetarange);

     acceptance_xray[i] = gammasdet_stm[i] / xrayphotons;
     acceptance_xray_error[i] = acceptance_xray[i] * gammasdet_stm_error[i] / gammasdet_stm[i];
       
     N++;
  }

  if(branch == 0){
  
  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance, E_error, acceptance_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kBlue-9);
  gracc->SetLineColor(kBlue-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);

  if( cutVD10 == 7.50224 ){
    //Fit->SetParameters( 6.74813e-07, 592290, 4.45523e+11, 3.50593e+11 );
    Fit->SetParameters( 4.14952e-07, 1.26081e+06, 1.51919e+12, 1.58937e+12 );
  }
  if( cutVD10 == 0 ){
    Fit->SetParameters( 1.10584e-07, 3.5573e+06, 1.63318e+13, 1.26467e+13);
  }
  if( cutVD10 == 7.86 ){
    Fit->SetParameters( 6.65616e-07, 617247, 4.69888e+11, 3.80758e+11);
  }
  if( cutVD10 == 5.5 ){
    Fit->SetParameters( 9.38481e-07, 420464, 2.27214e+11, 1.7668e+11);
  }
  if( cutVD10 == 5 ){
    Fit->SetParameters( 9.93327e-07, 404120, 2.06505e+11, 1.63212e+11);
  }
  if( cutVD10 == 4.5 ){
    Fit->SetParameters( 1.04556e-06, 376565, 1.82846e+11, 1.41715e+11 );
  }
  if( cutVD10 == 3.5 ){
    Fit->SetParameters( 1.04556e-06, 376565, 1.82846e+11, 1.41715e+11 );
  }
  if( cutVD10 == 2.5 ){
    Fit->SetParameters( 5.4991e-07, 711497, 6.56943e+11, 5.05919e+11 );
  }
  
  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);
  
  Fit->SetLineColor(kBlue+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);

  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();
 
  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");

  TMatrixD _cov = fitptr->GetCovarianceMatrix();
 
  //Store best fit paramters and errors (p0 +- p0_error...) and the covariance matrix
  double NP = 4; //Number fit parameters
  TMatrixD _BestFitPars(1,NP,Fit->GetParameters(),""); 
  TMatrixD _BestFitParsErrors(1,NP,Fit->GetParErrors(),"");
  
  TMatrixD* cov = new TMatrixD(NP,NP,_cov.GetMatrixArray(),"");
  TMatrixD* BestFitPars = new TMatrixD(NP,1,_BestFitPars.GetMatrixArray(),"");
  TMatrixD* BestFitParsErrors = new TMatrixD(NP,1,_BestFitParsErrors.GetMatrixArray(),"");

  std::cout<<"Cov matrix..."<<std::endl;
  cov->Print();
  std::cout<<"Best fit parameters..."<<std::endl;
  BestFitPars->Print();
  std::cout<<"Best fit parameters errors..."<<std::endl;
  BestFitParsErrors->Print();

  
  TFile *output;    
  std::stringstream streamcutVD10;
  streamcutVD10 << std::fixed << std::setprecision(4) << cutVD10;

  std::string rootfile = "CovMatrix_Bestfits_"+streamcutVD10.str()+"cut.root";
  output = new TFile(rootfile.c_str(),"recreate");

  output->WriteObject(cov, "Covmatrix");
  output->WriteObject(BestFitPars, "BestFitPars");
  output->WriteObject(BestFitParsErrors, "BestFitParsErrors");

  output->Close();
  }



if(branch == 1){

  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance_VD10_STM, E_error, acceptance_VD10_STM_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kRed-9);
  gracc->SetLineColor(kRed-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  Fit->SetParameters(0.00222227,237.841,53489.4,56559.9);

  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);

  Fit->SetLineColor(kMagenta+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);
  
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetTitleOffset(1.9);
  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();

  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");
  
 }



 if(branch == 2){

  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance_xray, E_error, acceptance_xray_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kGreen-9);
  gracc->SetLineColor(kGreen-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  Fit->SetParameters( 1.10226e-08, 4.8904e+07, 2.21491e+15, 2.39138e+15 );
  TFitResultPtr fitptr = gracc->Fit(Fit,"0S","", xmin, xmax);
 
  Fit->SetLineColor(kGreen+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);

  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();

  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");
 
  std::cout<<"The acceptance at 66 keV from the fit: "<<Fit->Eval(0.066)<<std::endl;
  std::cout<<"The acceptance at 347 keV from the fit: "<<Fit->Eval(0.347)<<std::endl;
  std::cout<<"The acceptance at 844 keV from the fit: "<<Fit->Eval(0.844)<<std::endl;
  std::cout<<"The acceptance at 1809 keV from the fit: "<<Fit->Eval(1.809)<<std::endl;

 }


 
if(branch == 4){

  double normalise = acceptance_xray[4];
  for(int i=0; i<N; i++){
    acceptance_xray[i] = acceptance_xray[i]/normalise;
    acceptance_xray_error[i] = acceptance_xray_error[i]/normalise;
  }

  TGraphErrors *gracc = new TGraphErrors( N, E, acceptance_xray, E_error, acceptance_xray_error );
  gracc->Print("");
  gracc->SetMarkerStyle(21);
  gracc->SetMarkerColor(kGreen-9);
  gracc->SetLineColor(kGreen-9);
  gracc->SetLineWidth(2);
  gracc->Draw("same,p");

  TF1*Fit = new TF1("Fit", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  double p_0 = 1.10226e-08/normalise;
  double p_1 = 4.8904e+07*normalise;
  double p_2 = 2.21491e+15*normalise*normalise;
  double p_3 = 2.39138e+15*normalise*normalise;
  std::cout<<"p_0: "<<p_0<<" p_1: "<<p_1<<" p_2: "<<p_2<<" p_3: "<<p_3<<std::endl;
  Fit->SetParameters( p_0, p_1, p_2, p_3 );
  TFitResultPtr fitptr_norm = gracc->Fit(Fit,"0S","", xmin, xmax);

 
  Fit->SetLineColor(kGreen+3);
  Fit->SetLineStyle(2);
  Fit->SetLineWidth(3);

  gracc->Draw("same,p");
  Fit->Draw("same");

  gPad->Update();
  TPaveStats* ps = (TPaveStats *)gracc->FindObject("stats");
  ps->SetY1NDC(.22);
  ps->SetY2NDC(.55);
  ps->SetX1NDC(0.52);
  ps->SetX2NDC(0.9);
  ps->SetLineWidth(6);
  ps->SetLineColor(kWhite);

  TAxis *Y = graph1->GetYaxis();
  Y->SetNdivisions(6,6,1);

  c1->Modified();
  c1->Update();

  graph1->GetYaxis()->SetTitleSize(0.035);
  graph1->Draw("ap");
  gracc->Draw("same,p");
  Fit->Draw("same");
 
  std::cout<<"The acceptance at 66 keV from the fit: "<<Fit->Eval(0.066)<<std::endl;
  std::cout<<"The acceptance at 347 keV from the fit: "<<Fit->Eval(0.347)<<std::endl;
  std::cout<<"The acceptance at 844 keV from the fit: "<<Fit->Eval(0.844)<<std::endl;
  std::cout<<"The acceptance at 1809 keV from the fit: "<<Fit->Eval(1.809)<<std::endl;

 }


  
  if(branch == 3){
    double xlimit = 80;
    int Nbins_in = 80000;
    //int Nbins_in = 1600;
    double count_photopeak =0;
    
    TH1D*h1 = new TH1D("h1","", Nbins_in, 0, xlimit);
    TH1D*h2 = new TH1D("h2","", Nbins_in, 0, xlimit);
 
    double POT = 200000000;
      
    fstream readfile;
    readfile.open(ArtFiles_location,ios::in);
    string name;
    vector<string> file_name;
    file_name.clear();

  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<name<<std::endl;
  }


  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  
    //Read tree
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){  

    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");

    tree->SetBranchAddress("evt",&evt);
    tree->SetBranchAddress("trk",&trk);
    tree->SetBranchAddress("sid",&sid);
    tree->SetBranchAddress("pdg",&pdg);
    tree->SetBranchAddress("run",&run);
    tree->SetBranchAddress("subrun",&subrun);
    tree->SetBranchAddress("time",&time);
    tree->SetBranchAddress("x",&x);
    tree->SetBranchAddress("y",&y);
    tree->SetBranchAddress("z",&z);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("ke",&ke);
    tree->SetBranchAddress("code",&code);
    
    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;
    

     for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      //Just fill at Virtual detector ID number:

      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);

	if( pdg==22 ){
	 
	  double xcentre = -3904; //mm
	  double ycentre = 0;
	  double r = sqrt((x-xcentre)*(x-xcentre)+(y-ycentre)*(y-ycentre));

	  if((r < (cutVD10*mm_to_cm))&&(pz > 0)){
	    //if(code==16){ //bremsstrahlung
	    h1->Fill(mom);
	    if((mom<=0.35)&&(mom>=0.344)){
	      count_photopeak++;
	    }
	    //}
	  }
	}
	
      }//sid

     }//entries

  }//file

  //Plot histogram
  TAxis *X = h2->GetXaxis();
  X->SetNdivisions(5,3,0);
  h2->SetFillStyle(3001);
  h2->SetFillColor(kGreen+2);
  //h2->SetFillColor(kOrange-3);
  
  int j =0;

  //Get bins and multiply each bin by the acceptance:
  int Nbins = h1->GetNbinsX();
  double p_fitvalues[4] = { 4.149e-7, 1.261e6, 1.519e12, 1.589e12 };
  double Full_Acceptance;
  double E_times_acc;
  
  for(int i = 0; i < Nbins; i++){

    double bincontent = h1->GetBinContent(i);
    double bincenter = h1->GetXaxis()->GetBinCenter(i);

    if(bincenter < 0.036){ Full_Acceptance = 0; }
    else { Full_Acceptance = p_fitvalues[0] + 1./(p_fitvalues[1]*bincenter) - sqrt(1./(p_fitvalues[2]*bincenter) + 1./(p_fitvalues[3]*bincenter*bincenter)); }

    E_times_acc = bincontent * Full_Acceptance;
    std::cout<<"Bin content: "<<bincontent<<" counts, Bin center: "<<bincenter<<" MeV, Acceptance: "<<Full_Acceptance<<std::endl;
    std::cout<<"New bin content for bin "<<i<<": "<<E_times_acc<<std::endl;
    std::cout<<"Number of counts in the brems photopean 347+-3keV from MDC2020: "<<count_photopeak<<std::endl;
    h2->SetBinContent( i , E_times_acc );

  }

  
  bool drawhisto = true;
  
  if(drawhisto ==true){
    char* Xtitle = const_cast<char*>(Xtitlest.c_str());
    char* Ytitle = const_cast<char*>(Ytitlest.c_str());
    h2->GetXaxis()->SetTitle(Xtitle);
    h2->GetYaxis()->SetTitle(Ytitle);
  
    //h2->Scale(1./POT);
    //h2->Draw("HIST");
    h2->Draw("");
    /*
    c1->Update();
    TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
    ps->SetX1NDC(0.71);
    ps->SetX2NDC(0.91);
    ps->SetY1NDC(0.76);
    ps->SetY2NDC(0.9);
    ps->SetTextSize(0.032);
    ps->SetName("mystats");
 
    h2->SetStats(0);
    ps->GetLineWith("Entries")->SetTextColor(kBlue);

    c1->Modified();
    */
    //gPad->SetLogy();
    

    double timeMDC2020 = 0.00001695; //s
    double xlowlim = 0;


    std::cout<<"Time mdc2020: "<<timeMDC2020<<" sec"<<std::endl;
    
    //Integral of the histogram to get the rate
    double integral =  h2->Integral(h2->FindFixBin(xlowlim), h2->FindFixBin(xlimit), "");
    std::cout<<"Integral of the histogram "<<xlowlim<<"-"<<xlimit<<" MeV: "<<integral<<std::endl;
    double integral_width =  h2->Integral(h2->FindFixBin(xlowlim), h2->FindFixBin(xlimit), "width");
    //std::cout<<"Integral of the histogram (multiplying by bin width) "<<xlowlim<<"-"<<xlimit<<" MeV: "<<integral_width<<std::endl;
    double rate = integral / (1000*timeMDC2020); //kHz
    std::cout<<"Rate of flash photons at STM: "<<rate<<" kHz"<<std::endl;

    xlowlim = 0.1;
    integral =  h2->Integral(h2->FindFixBin(xlowlim), h2->FindFixBin(xlimit), "");
    std::cout<<"Integral of the histogram "<<xlowlim<<"-"<<xlimit<<" MeV: "<<integral<<std::endl;
    rate = integral / (1000*timeMDC2020); //kHz 
    std::cout<<"Rate of flash photons at STM: "<<rate<<" kHz"<<std::endl;

    int Nbinsh2 = h2->GetNbinsX();
    double sum=0;
    for(int i = 0; i < Nbinsh2; i++){
      double bincontent = h2->GetBinContent(i);
      double bincenter = h2->GetXaxis()->GetBinCenter(i);
      if((bincenter>xlowlim)&&(bincenter<xlimit)){sum=sum+bincontent;}
    }
    std::cout<<"Integral of the histogram by summing the bin content "<<xlowlim<<"-"<<xlimit<<" MeV: "<<sum<<std::endl;

    h2->GetXaxis()->SetRangeUser(0.1, xlimit);
    std::cout<<"Mean E between 0.1 and "<<xlimit<<" MeV: "<<h2->GetMean()<<std::endl;
  
    TFile *output;
        
    std::string rootfile = "SpectrumFlashPhotonsAtSTM_80000bins.root";
    output = new TFile(rootfile.c_str(),"recreate");
    output->WriteObject(h2, "h2");
    
    output->Close();
  
  }//If draw histo

  }



 

  //c1->Print("MDC2020Ele_BremsPhotonsEdistrVD10cut_Acc_1_0.0032_2MeV_newfit_nolegend_new.png");
  //c1->Print("MDC2020MuonsEle_FlashPhotonsEdistrVD10cut_1stepAcc_0_80_nolegend.pdf");
}


//================================================================

void Acceptance_Rate_E10to7(std::string ArtFiles_location, int VDnumber, double cutVD10){
  
  //PlotAcceptance_E("", 0, 6, 0, 0.0000002, "E_{#gamma} [MeV]", "Flash Acceptance VD10 - VD89,90", VDnumber, cutVD10, 0);

  //PlotAcceptance_E("", 0, 6, 0, 0.0012, "E_{#gamma} [MeV]", "Acceptance (VD=10)/(VD=89,90) in cos(#theta)=[0.99999,1]", VDnumber, cutVD10, 1);

  //PlotAcceptance_E("", 0, 6, 0, 0.000000005, "E_{#gamma} [MeV]", "X-Ray Acceptance VD10 - VD89,90", VDnumber, cutVD10, 2);

  //PlotAcceptance_E("", 0, 6, 0, 6, "E_{#gamma} [MeV]", "X-Ray acceptance relative to 347 keV X-Ray acceptance", VDnumber, cutVD10, 4);
  
  PlotAcceptance_E(ArtFiles_location, 0, 0, 0, 0, "E_{#gamma} [MeV]", "Counts #times Acceptance(VD10-VD89,90)", VDnumber, cutVD10, 3);

}
