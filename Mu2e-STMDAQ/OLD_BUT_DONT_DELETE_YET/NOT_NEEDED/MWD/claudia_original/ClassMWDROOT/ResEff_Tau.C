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

void ResEff_Tau(int print){
  gROOT->SetStyle("ATLAS");

  double Etrue=662;
  bool newsim=true;

  int N=1; //number of files to read
  //Resolution and efficiency for old and new simulation
  fstream readfile[1];
  ifstream myfile[1];
  std::string NameMWDfiles[1];
  std::string Name[1];
  vector<string> file_names[1];

  std::string filespath[1];
  int M=400,L=200;

  filespath[0] = "/work/mu2e/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/M400L200Tau/20kHz/MWDfilesM400L200_taus.txt";
 
  for(int i=0;i<N;i++){
    readfile[i].open(filespath[i],ios::in);
  }

  
  Double_t tau[36]={20000, 30000, 40000, 45000, 45500, 46000, 46500, 47000, 47500, 48000, 48500, 49000, 49500, 50000, 50500, 51000, 51500, 52000, 52500, 53000, 53600, 54000, 54500, 55000, 55500, 56000, 56500, 57000, 57500, 58000, 58500, 59000, 59500, 60000, 70000, 80000};  
  Double_t tau_error[36]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  double res[36], res_error[36], res_error_calc[36], eff[36], eff_error[36], chi2[36], meanE[36], meanE_error[36];
  vector<double> resolutionv, res_errorv, eff_errorv, efficiencyv, chi2v, meanEv, meanE_errorv, res_error_calcv;
  std::string val;

  for(int i=0;i<N;i++){
    while(1){
      readfile[i]>>NameMWDfiles[i];
      file_names[i].push_back(NameMWDfiles[i]);
      if(readfile[i].eof())break;
    }
  }
 


  std::cout<<"File Size, M: "<<M<<" L: "<<L<<file_names[0].size()<<std::endl;
 

  for(int i=0;i<N;i++){
    std::cout<<""<<std::endl;
    std::cout<<"*************NEW MWD file containing*************"<<std::endl;
    
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
	if (countline==17){ss >> res[i];}
	//read the fit error in the resolution
	if (countline==18){ss >> res_error[i];}
	//read the efficiency
	if (countline==20){ss >> eff[i];}
	//read the efficiency error
	if (countline==21){ss >> eff_error[i];}
	//read the mean pulse height (energy)
	if (countline==30){ ss >> val; ss >> meanE[i]; ss >> val; ss >> meanE_error[i];}
	//read calculated error for resolution
	if (countline==31){ ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> val;ss >> res_error_calc[i];}
	//read the chi2
	if (countline==32){ ss >> val; ss >> chi2[i];}

	countline++;
      }

      if(res[i]<0){res[i]=res[i]*(-1);}
      if(res_error[i]<0){res_error[i]=res_error[i]*(-1);}
      if(res_error_calc[i]<0){res_error_calc[i]=res_error_calc[i]*(-1);}
      resolutionv.push_back(res[i]);
      res_errorv.push_back(res_error[i]);
      res_error_calcv.push_back(res_error_calc[i]);
      efficiencyv.push_back(eff[i]);
      eff_errorv.push_back(eff_error[i]);
      chi2v.push_back(chi2[i]);
      double heightshift = (Etrue-meanE[i])/Etrue;
      meanEv.push_back(heightshift);
      meanE_errorv.push_back(abs((Etrue-meanE[i])/Etrue)*(meanE_error[i]/meanE[i]));
      std::cout<<"Res= "<<res[i]<<"+-"<<" Fit error: "<<res_error[i]<<" Calculated error (sigma/sqrt(2Ncounts)): "<<res_error_calc[i]<<std::endl;
      std::cout<<"Eff= "<<eff[i]<<"+-"<<eff_error[i]<<std::endl;
      std::cout<<"MeanE= "<<meanE[i]<<"+-"<<meanE_error[i]<<std::endl;
      std::cout<<"Chi2= "<<chi2[i]<<std::endl;
      std::cout<<"Height shifting: "<<heightshift<<std::endl;
      myfile[i].close();
    }
    
    readfile[i].close();
    
  }//N
  


 //PLOT******************************

 int n=36;
 //Plot resolution
 if(print==0){
 double xx1[2]={10,90};
 double yy1[2]={0.5,2.5}; 
 TGraph *graph1 = new TGraph (2,xx1,yy1);
 graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
 graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
 graph1->SetTitle("");
 graph1->GetXaxis()->SetTitle("#tau [#mus]");
 graph1->GetYaxis()->SetTitle("Resolution [keV]");
 graph1->GetXaxis()->SetTitleOffset(0.9);
 graph1->SetMarkerStyle(1);
 graph1->Draw("ap");

 for(int i=0;i<n;i++){
   tau[i] = tau[i]/1000;}

 TGraphErrors *grRes[1];
 for(int i=0;i<N;i++){
   grRes[i]= new TGraphErrors(n,&tau[0],&resolutionv[0], &tau_error[0],&res_error_calcv[0]);
   
 }

 grRes[0]->SetMarkerColor(kViolet+2);
 grRes[0]->SetMarkerStyle(20);
 grRes[0]->SetMarkerSize(1.2);
 grRes[0]->Draw("same,p");

 TLatex latex2;
 latex2.SetTextSize(0.04);
 //latex2.DrawLatex(0,1.5,"Time Sample=0.05s, #sigma_{noise}=0.32mV");
 latex2.DrawLatex(0,1,"M=400, L=200, #sigma_{noise}=0.32mV, E=662keV, 20kHz");

 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(grRes[0],"M=400, L=200","p");
 //legend->Draw("same");
 
 }





 //Plot efficiency
 if(print==1){
   double xx1[2]={10,90};
   double yy1[2]={0.8,1.2};
   TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
   graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
   graph1->SetTitle("");
   graph1->GetXaxis()->SetTitle("#tau [#mus]");
   graph1->GetYaxis()->SetTitle("MWD Efficiency");
   graph1->GetXaxis()->SetTitleOffset(0.9);
   graph1->SetMarkerStyle(1);
   graph1->Draw("ap");


   for(int i=0;i<n;i++){
     tau[i] = tau[i]/1000;}

   TGraphErrors *grEff[1];
   for(int i=0;i<N;i++){
     grEff[i]= new TGraphErrors(n,&tau[0],&efficiencyv[0], &tau_error[0],&eff_errorv[0]);

   }

   grEff[0]->SetMarkerColor(kViolet+2);
   grEff[0]->SetMarkerStyle(20);
   grEff[0]->SetMarkerSize(1.2);
   grEff[0]->Draw("same,p");

   TLatex latex2;
   latex2.SetTextSize(0.04);
   //latex2.DrawLatex(0,1,"Time Sample=0.05s, #sigma_{noise}=0.32mV");
   latex2.DrawLatex(0,1,"M=400, L=200, #sigma_{noise}=0.32mV, E=662keV, 20kHz");  

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   legend->AddEntry(grEff[0],"M=400, L=200","p");
   //legend->Draw("same");


   }







 //Plot (Etrue-Ereco)/Etrue
 if(print==2){
   double xx1[2]={10,90};
   double yy1[2]={-0.05,0.02};
   TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
   graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
   graph1->SetTitle("");
   graph1->GetXaxis()->SetTitle("#tau [#mus]");
   graph1->GetYaxis()->SetTitle("Height Shifting [(E_{true}-E_{reco})/E_{true}]");
   graph1->GetXaxis()->SetTitleOffset(0.9);
   graph1->SetMarkerStyle(1);
   graph1->Draw("ap");


   for(int i=0;i<n;i++){
     tau[i] = tau[i]/1000;}

   TGraphErrors *grE[1];
   for(int i=0;i<N;i++){
     grE[i]= new TGraphErrors(n,&tau[0],&meanEv[0], &tau_error[0],&meanE_errorv[0]);
   }
 
   grE[0]->SetMarkerColor(kViolet+2);
   grE[0]->SetMarkerStyle(20);
   grE[0]->SetMarkerSize(1.2);
   grE[0]->Draw("same,p");

   TLatex latex2;
   latex2.SetTextSize(0.04);
   //latex2.DrawLatex(0,0,"Time Sample=0.05s, #sigma_{noise}=0.32mV");
   latex2.DrawLatex(0,0,"M=400, L=200, #sigma_{noise}=0.32mV, E=662keV, 20kHz");

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   legend->AddEntry(grE[0],"M=400, L=200, 20kHz","p");
   //legend->Draw("same");



}



  
}
