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
#include <random>

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
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TGraph2D.h"
#include "TH3.h"
#include "TH3D.h"
#include "TLatex.h"
#include "TText.h"
#include "TList.h"
#include "TGraphErrors.h"


#define mm_to_cm 10
#define PI 3.14159265358979323846  /* pi */

//================================================================ 
enum ParIndex_t {
   b0, b1, b2,b3};
// Use this map to (re-)name parameters for the plot
const std::map<ParIndex_t,std::string> parNames{
   {b0, "b_{0}"}, {b1, "b_{1}"}, {b2, "b_{2}"},
   {b3, "b_{3}"}
};



void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  gStyle->SetOptFit(01111);


  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

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
  
  TH1F*h1 = new TH1F("TH1","", nbins, Xrange[0], Xrange[1]);
  
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){
    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");
    //TTree* tree=(TTree*)input->Get("readvd/ntvdext");

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
    
    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      //Just fill at Virtual detector ID number:
      //float VDnumber = 89;
      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);
	double pt = sqrt(px*px+py*py);
	double costheta = pz/mom;

	  
	if((debugout==true)/*&&(file==0)*/){
	  std::cout<<"Virtual Detector number: "<<sid<<std::endl;
	  std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" mom: "<<mom<<" MeV, time: "<<time<<std::setprecision(30)<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm, creation code: "<<code<<" trk: "<<trk<<std::endl;
	}

	if((branch==7)&&(pdg==22)&&(code==16)){h1->Fill(mom);} //brems

	
      } //if sid
   
    }//entries

    input->Close();
  }//for int files

  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  //h1->SetFillStyle(3001);
  if(branch==7){h1->SetFillColor(kOrange-3);h1->SetLineColor(kOrange-3);h1->SetMarkerColor(kOrange-3);}
  h1->Draw("");


  double rangex1=0.04;
  double rangex2=1.809;


  double Integral2 = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax), "");
  std::cout<<"Integral histo "<<xmin<<","<<xmax<<" : "<<Integral2<<std::endl;

  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./Integral2);
  std::cout<<"Normalising to integral: "<<Integral2<<std::endl;
  hnorm1->GetYaxis()->SetRangeUser(0., 0.012);
  hnorm1->Draw("HIST");
  

  TF1*FitnormBrems = new TF1("FitnormBrems", "[0] * (1.0/(exp([1]*x)+[2]) + [3])", rangex1, rangex2);
  FitnormBrems->SetParameter(0,1.75733e-02);
  FitnormBrems->FixParameter(1,2.367);
  FitnormBrems->FixParameter(2,-0.9748);
  FitnormBrems->FixParameter(3,0.0743);
  
  hnorm1->Fit(FitnormBrems,"ES0","",rangex1,rangex2);
  FitnormBrems->SetLineWidth(4);

  for (auto& idx_name : parNames) {
     FitnormBrems->SetParName(idx_name.first, idx_name.second.c_str());
   }

  FitnormBrems->SetLineColor(kRed);
  FitnormBrems->SetLineStyle(2);
  FitnormBrems->Draw("same");

  TFitResultPtr r = hnorm1->Fit(FitnormBrems,"ES0","",rangex1,rangex2);
  TMatrixD cov = r->GetCovarianceMatrix();
  cov.Print();

  double signal_region1 = 0.297;
  double signal_region2 = 0.397;

  double Integralnorm1 = hnorm1->Integral(hnorm1->FindFixBin(0), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0,2: "<<Integralnorm1<<std::endl;
  double Integralnorm2 = hnorm1->Integral(hnorm1->FindFixBin(xmin), hnorm1->FindFixBin(xmax), "");
  std::cout<<"Integral norm histo "<<xmin<<","<<xmax<<" : "<<Integralnorm2<<std::endl;
  double Integralnorm3 = hnorm1->Integral();
  std::cout<<"Integral norm histo -0.5,2: "<<Integralnorm3<<std::endl;
  std::cout<<""<<endl;
  double Integralnorm4 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "WIDTH");
  std::cout<<"Integral norm histo 0.04,2 (width): "<<Integralnorm4<<std::endl;
  double Integralnorm5 = hnorm1->Integral(hnorm1->FindFixBin(0.04), hnorm1->FindFixBin(2), "");
  std::cout<<"Integral norm histo 0.04,2 (no width): "<<Integralnorm5<<std::endl;
  double Integralnorm6 = hnorm1->Integral(hnorm1->FindFixBin(signal_region1), hnorm1->FindFixBin(signal_region2), "WIDTH");
  std::cout<<"Integral norm histo signal region "<<signal_region1<<", "<<signal_region2<<" (width): "<<Integralnorm6<<std::endl;
  std::cout<<""<<std::endl;
  double Integralfunc1 = FitnormBrems->Integral(0.04,2);
  std::cout<<"Integral func 0.04,2: "<<Integralfunc1<<std::endl;
  double Integralfunc_signal = FitnormBrems->Integral(signal_region1,signal_region2);
  std::cout<<"Integral func signal region "<<signal_region1<<", "<<signal_region2<<": "<<Integralfunc_signal<<std::endl;

  double ratio = Integralnorm4/Integralfunc1;
  double ratio_signalregion = Integralnorm6/Integralfunc_signal;


  std::cout<<""<<endl;
  std::cout<<"----Ratio histogram/function (width)---: "<<ratio<<std::endl;
  std::cout<<"----Ratio histogram/function (no width)---: "<<Integralnorm5/Integralfunc1<<std::endl;

  //Signal Region histogram to function integral
  std::cout<<"----Ratio histogram/function in signal region (width)---: "<<ratio_signalregion<<std::endl;



  double chi2_fullregion =0;
  double ndof_fullregion =0;
  double chi2_signalregion =0;
  double ndof_signalregion =0;
  double nfit_params = 1;

  //Full region Chi2 calculation
  int diff = hnorm1->FindFixBin(rangex2)-hnorm1->FindFixBin(rangex1);
  int bin1 = hnorm1->FindFixBin(rangex1); std::cout<<"Bin1: "<<bin1<<" bin center: "<<hnorm1->GetBinCenter(bin1)<<std::endl;
  int bin2 = hnorm1->FindFixBin(rangex2); std::cout<<"Bin2: "<<bin2<<" bin center: "<<hnorm1->GetBinCenter(bin2)<<std::endl;
  for(int i=bin1; i<=bin2;i++){
    if(hnorm1->GetBinContent(i) == 0){continue;}
    double y = (hnorm1->GetBinContent(i)-FitnormBrems->Eval(hnorm1->GetBinCenter(i)))/hnorm1->GetBinError(i);
    chi2_fullregion = chi2_fullregion + y*y;
    ndof_fullregion++;

  }
  std::cout<<"Chi2 full region:"<<chi2_fullregion<<std::endl;
  std::cout<<"ndof full region:"<<(ndof_fullregion-nfit_params)<<std::endl;

  //Signal region Chi2 calculation
  diff = hnorm1->FindFixBin(signal_region2)-hnorm1->FindFixBin(signal_region1);
  bin1 = hnorm1->FindFixBin(signal_region1); std::cout<<"Bin1: "<<bin1<<" bin center: "<<hnorm1->GetBinCenter(bin1)<<std::endl;
  bin2 = hnorm1->FindFixBin(signal_region2); std::cout<<"Bin2: "<<bin2<<" bin center: "<<hnorm1->GetBinCenter(bin2)<<std::endl;
  for(int i=bin1; i<=bin2;i++){
    double y = (hnorm1->GetBinContent(i)-FitnormBrems->Eval(hnorm1->GetBinCenter(i)))/hnorm1->GetBinError(i);
    chi2_signalregion = chi2_signalregion + y*y;
    ndof_signalregion++;

  }

  std::cout<<"Chi2 signal region:"<<chi2_signalregion<<std::endl;
  std::cout<<"ndof signal region:"<<ndof_signalregion<<std::endl;


  std::cout<<"Nbins: "<<nbins<<std::endl;


  std::stringstream streamratio;
  streamratio << std::fixed << setprecision(3) << ratio;
  std::string str_latex = "Histogram/Fit Integral = "+streamratio.str();
  char* char_latex = const_cast<char*>(str_latex.c_str());


    c1->Update();

    //stat = (TPaveStats*)hnorm1->GetListOfFunctions()->FindObject("stats");
    TPaveStats *stat = (TPaveStats*) c1->GetPrimitive("stats");
    stat->SetName("mystats");

    h1->SetStats(0);
    hnorm1->SetStats(0);

    TList *listOfLines = stat->GetListOfLines();
    //listOfLines->Print();
    TText *firstEntry = (TText*)listOfLines->At(0);
    if (firstEntry) {
        std::cout << "Text: " << firstEntry->GetTitle() << std::endl;
        std::cout << "Position: (" << firstEntry->GetX() << ", " << firstEntry->GetY() << ")" << std::endl;
        std::cout << "Font Size: " << firstEntry->GetTextSize() << std::endl;
        std::cout << "Color: " << firstEntry->GetTextColor() << std::endl;
    }
    firstEntry->SetText(0, firstEntry->GetY(), "Background Fit");
    stat->GetLineWith("Background Fit")->SetTextColor(kRed);

    stat->SetX1NDC(0.4); //new x start position
    stat->SetX2NDC(0.8); //new x end position
    stat->SetY1NDC(0.4); //new y start position
    stat->SetY2NDC(0.87); //new y end position
    stat->SetTextSize(0.04);

    TText *parTextMean = stat->GetLineWith("Mean");
    TText *parTextRMS = stat->GetLineWith("Std Dev");
    TString formattedTextMean = Form("Mean = %.4f", hnorm1->GetMean());
    parTextMean->SetText(0, parTextMean->GetY(), formattedTextMean);

    TString formattedTextRMS = Form("Std Dev = %.4f", hnorm1->GetRMS());
    parTextRMS->SetText(0, parTextRMS->GetY(), formattedTextRMS);


    for (int i = 0; i < FitnormBrems->GetNpar(); ++i) {
       // Get the line with the fit parameter name
       TString parName = "b_{"+std::to_string(i)+"}";
       std::cout<<"par name "<<parName<<std::endl;
       TText *parText = stat->GetLineWith(parName);

       // Format the parameter value in scientific notation
       if (parText) {
           double parValue = FitnormBrems->GetParameter(i);
           double parValueError = FitnormBrems->GetParError(i);
           TString formattedText = Form(parName+" = %.2e #pm %.2e", parValue, parValueError);

           if(i==0){
             parValue=parValue*1000;
             parValueError=parValueError*1000;
             formattedText = Form(parName+" = (%.2f #pm %.2f) #times 10^{-3}", parValue, parValueError);
           }
           else if(i==1){
             formattedText = Form(parName+" = %.2f (fixed)", parValue, parValueError);
           }
           else if(i==2){
             parValue=parValue*10;
             parValueError=parValueError*10;
             formattedText = Form(parName+" = %.2f (fixed)", parValue, parValueError);
           }
           else{
             parValue=parValue*100;
             parValueError=parValueError*100;
             formattedText = Form(parName+" = %.2f (fixed)", parValue, parValueError);
           }
           parText->SetText(0, parText->GetY(), formattedText);
       }
   }


    c1->Modified();

    //PLOT THE FULL FIT
    hnorm1->Draw("HIST");
    FitnormBrems->Draw("same");
    stat->Draw("same");

    c1->Print("BackgroundFit_NormalisationFactor_MuonBeamBrems.png");
}



//================================================================
void FitMuonBeam_ReadVD(std::string ArtFiles_location, int VDnumber ) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 
  
  //Open txt
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


  PrintHisto(file_name, 1000, 0.04, 2, 0, 400, "p_{bremsstrahlung #gamma, VD=15} [MeV]", "Normalised Counts/(0.00196 MeV)", VDnumber, 7);

  readfile.close();
}

//================================================================
