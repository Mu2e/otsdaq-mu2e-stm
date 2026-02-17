#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <sys/stat.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TGraph2D.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h" 
#include "TStyle.h"
#include "TAxis.h"
#include "TLatex.h"
#include "TProfile2D.h"
#include "TPaletteAxis.h"
#include "TColor.h"
#include "TGraphErrors.h"

using namespace std;
//If print =0 plot resolution 
//If print =1 plot efficiency                                                                                                  


//For the rate write: "1", "5" , "10" , ... , "200"
void ResEff_OldNewSimML(int print){
  gROOT->SetStyle("ATLAS");

  double Etrue=675;
  bool newsim=true;

  int N=2; //number of files to read
  //Resolution and efficiency for old and new simulation
  fstream readfile[5];
  ifstream myfile[5];
  std::string NameMWDfiles[5];
  std::string Name[5];
  vector<string> file_names[5];

  std::string filespath[5];
  int M1=400,L1=200;
  int M2=1000,L2=500;


  if(newsim==true){
    filespath[0] = "/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/675keV_0.32mV/MWDfilesM"+std::to_string(M1)+"L"+std::to_string(L1)+".txt";
    filespath[1] = "/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/675keV_0.32mV/MWDfilesM"+std::to_string(M2)+"L"+std::to_string(L2)+".txt";
  }
  else{
    filespath[0] = "/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.32mV/MWDfilesM"+std::to_string(M1)+"L"+std::to_string(L1)+".txt"; 
   filespath[1] = "/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.32mV/MWDfilesM"+std::to_string(M2)+"L"+std::to_string(L2)+".txt";
 }
 
  for(int i=0;i<N;i++){
    readfile[i].open(filespath[i],ios::in);
  }

  
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};  
  Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  double res[5], res_error[5], res_error_calc[5], eff[5], eff_error[5], chi2[5], meanE[5], meanE_error[5];
  vector<double> resolutionv[5], res_errorv[5], eff_errorv[5], efficiencyv[5], chi2v[5], meanEv[5], meanE_errorv[5], res_error_calcv[5];
  std::string val;

  for(int i=0;i<N;i++){
    while(1){
      readfile[i]>>NameMWDfiles[i];
      file_names[i].push_back(NameMWDfiles[i]);
      if(readfile[i].eof())break;   
    }
  }
 


  std::cout<<"File Size, M: "<<M1<<" L: "<<L1<<file_names[0].size()<<std::endl;
  std::cout<<"File Size, M: "<<M2<<" L: "<<L2<<file_names[1].size()<<std::endl;
 

  for(int i=0;i<N;i++){
    std::cout<<""<<std::endl;
    std::cout<<"*************NEW MWD file containing MWD files at different rates*************"<<std::endl;
    
    for (unsigned long int file=0;file<(file_names[i].size()-1);file++){
      std::string path;
      path=file_names[i].at(file);
      std::cout<<path.c_str()<<std::endl;
    
      myfile[i].open(path);

      std::string line;
      int countline = 0;
      
      //This loop doesn't compile with the ROOT compiler it works with ++ compiler:                      
      while(std::getline(myfile[i], line)){
	std::stringstream ss(line);
	//read the resolution
	if (countline==16){ss >> res[i];}
	//read the fit error in the resolution
	if (countline==17){ss >> res_error[i];}
	//read the efficiency
	if (countline==19){ss >> eff[i];}
	//read the efficiency error
	if (countline==20){ss >> eff_error[i];}
	//read the mean pulse height (energy)
	if (countline==29){ ss >> val; ss >> meanE[i]; ss >> val; ss >> meanE_error[i];}
	//read calculated error for resolution
	if (countline==30){ ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> res_error_calc[i];}
	//read the chi2
	if (countline==31){ ss >> val; ss >> chi2[i];}

	countline++;
      }

      if(res[i]<0){res[i]=res[i]*(-1);}
      if(res_error[i]<0){res_error[i]=res_error[i]*(-1);}
      if(res_error_calc[i]<0){res_error_calc[i]=res_error_calc[i]*(-1);}
      resolutionv[i].push_back(res[i]);
      res_errorv[i].push_back(res_error[i]);
      res_error_calcv[i].push_back(res_error_calc[i]);
      efficiencyv[i].push_back(eff[i]);
      eff_errorv[i].push_back(eff_error[i]);
      chi2v[i].push_back(chi2[i]);
      meanEv[i].push_back(((Etrue-meanE[i])/Etrue));
      meanE_errorv[i].push_back(abs((Etrue-meanE[i])/Etrue)*(meanE_error[i]/meanE[i]));
      std::cout<<"Res= "<<res[i]<<"+-"<<" Fit error: "<<res_error[i]<<" Calculated error (sigma/sqrt(2Ncounts)): "<<res_error_calc[i]<<std::endl;
      std::cout<<"Eff= "<<eff[i]<<"+-"<<eff_error[i]<<std::endl;
      std::cout<<"MeanE= "<<meanE[i]<<"+-"<<meanE_error[i]<<std::endl;
      std::cout<<"Chi2= "<<chi2[i]<<std::endl;
      myfile[i].close();
    }
    
    readfile[i].close();
    
  }//N
  


 //PLOT******************************

 int n=13;
 //Plot resolution
 if(print==0){
 double xx1[2]={0,200};
 double yy1[2]={0,8}; 
 TGraph *graph1 = new TGraph (2,xx1,yy1);
 graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
 graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
 graph1->SetTitle("");
 graph1->GetXaxis()->SetTitle("Rate [kHz]");
 graph1->GetYaxis()->SetTitle("Resolution [keV]");
 graph1->GetXaxis()->SetTitleOffset(0.9);
 graph1->SetMarkerStyle(1);
 graph1->Draw("ap");



 TGraphErrors *grRes[5];
 for(int i=0;i<N;i++){ 
   grRes[i]= new TGraphErrors(n,&rate[0],&resolutionv[i][0], &rate_error[0],&res_error_calcv[i][0]);
   
 }

 grRes[0]->SetMarkerColor(kViolet+2);
 grRes[0]->SetMarkerStyle(20);
 grRes[0]->SetMarkerSize(1.2);
 grRes[0]->Draw("same,p");

 grRes[1]->SetMarkerColor(kBlack);
 grRes[1]->SetMarkerStyle(20);
 grRes[1]->SetMarkerSize(1.2);
 grRes[1]->Draw("same,p");


 TLatex latex2;
 latex2.SetTextSize(0.03);
 latex2.DrawLatex(0,1.5,"Time Sample=0.05s, #sigma_{noise}=0.32mV");
 
 
 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(grRes[1],"M=1000, L=500","p");
 legend->AddEntry(grRes[0],"M=400, L=200","p");
 legend->Draw("same");
 
 }





 //Plot efficiency
 if(print==1){
   double xx1[2]={0,200};
   double yy1[2]={0,1};
   TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
   graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
   graph1->SetTitle("");
   graph1->GetXaxis()->SetTitle("Rate [kHz]");
   graph1->GetYaxis()->SetTitle("MWD Efficiency");
   graph1->GetXaxis()->SetTitleOffset(0.9);
   graph1->SetMarkerStyle(1);
   graph1->Draw("ap");



   TGraphErrors *grEff[5];
   for(int i=0;i<N;i++){
     grEff[i]= new TGraphErrors(n,&rate[0],&efficiencyv[i][0], &rate_error[0],&eff_errorv[i][0]);

   }

   grEff[0]->SetMarkerColor(kViolet+2);
   grEff[0]->SetMarkerStyle(20);
   grEff[0]->SetMarkerSize(1.2);
   grEff[0]->Draw("same,p");

   grEff[1]->SetMarkerColor(kBlack);
   grEff[1]->SetMarkerStyle(20);
   grEff[1]->SetMarkerSize(1.2);
   grEff[1]->Draw("same,p");


   TLatex latex2;
   latex2.SetTextSize(0.03);
   latex2.DrawLatex(0,1,"Time Sample=0.05s, #sigma_{noise}=0.32mV");

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   legend->AddEntry(grEff[1],"M=1000, L=500","p");
   legend->AddEntry(grEff[0],"M=400, L=200","p");
   legend->Draw("same");


   }







 //Plot (Etrue-Ereco)/Etrue
 if(print==2){
   double xx1[2]={0,200};
   double yy1[2]={-0.008,0.008};
   TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
   graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
   graph1->SetTitle("");
   graph1->GetXaxis()->SetTitle("Rate [kHz]");
   graph1->GetYaxis()->SetTitle("Height Shifting [(E_{true}-E_{reco})/E_{true}]");
   graph1->GetXaxis()->SetTitleOffset(0.9);
   graph1->SetMarkerStyle(1);
   graph1->Draw("ap");



   TGraphErrors *grE[5];
   for(int i=0;i<N;i++){
     grE[i]= new TGraphErrors(n,&rate[0],&meanEv[i][0], &rate_error[0],&meanE_errorv[i][0]);
   }
 
   grE[0]->SetMarkerColor(kViolet+2);
   grE[0]->SetMarkerStyle(20);
   grE[0]->SetMarkerSize(1.2);
   grE[0]->Draw("same,p");

   grE[1]->SetMarkerColor(kBlack);
   grE[1]->SetMarkerStyle(20);
   grE[1]->SetMarkerSize(1.2);
   grE[1]->Draw("same,p");


   TLatex latex2;
   latex2.SetTextSize(0.03);
   latex2.DrawLatex(0,0,"Time Sample=0.05s, #sigma_{noise}=0.32mV");

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   legend->AddEntry(grE[1],"M=1000, L=500","p");
   legend->AddEntry(grE[0],"M=400, L=200","p");
   legend->Draw("same");



}



  
}
