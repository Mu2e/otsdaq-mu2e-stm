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
void OldNewSim(int print, int M, int L){
  gROOT->SetStyle("ATLAS");

  double Etrue=675; //keV

  //Resolution and efficiency for old and new simulation
  fstream readfileNew, readfileOld;
  ifstream myfileNewSim, myfileOldSim;
  std::string fileNameNewSim, fileNameOldSim;
  std::string nameNew, nameOld;
  vector<string> file_nameNew, file_nameOld;
  std::cout<<"hi"<<std::endl;
  std::string filesNew = "/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/675keV_0.32mV/MWDfilesM"+std::to_string(M)+"L"+std::to_string(L)+".txt";
  std::string filesOld = "/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.32mV/MWDfilesM"+std::to_string(M)+"L"+std::to_string(L)+".txt";
  //char* filesNewchar = const_cast<char*>(filesNew.c_str());
  readfileNew.open(filesNew,ios::in);
  readfileOld.open(filesOld,ios::in);

  
  Double_t rate[13]={1,5,10,20,30,40,50,80,100,120,150,180,200};  
  Double_t rate_error[13]={0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  double resNew, res_errorNew, effNew, eff_errorNew, chi2New, meanENew;
  double resOld, res_errorOld, effOld, eff_errorOld, chi2Old, meanEOld;

  vector<double> resolutionvNew, res_errorvNew, eff_errorvNew, efficiencyvNew, chi2vNew, meanEvNew;
  vector<double> resolutionvOld, res_errorvOld, eff_errorvOld, efficiencyvOld, chi2vOld, meanEvOld;
  std::string val;

  while(1){
    readfileNew>>nameNew;
    file_nameNew.push_back(nameNew);
    if(readfileNew.eof())break;   
  }
  while(1){
    readfileOld>>nameOld;
    file_nameOld.push_back(nameOld);
    if(readfileOld.eof())break;
  }


  std::cout<<"File Size New: "<<file_nameNew.size()<<std::endl;
  std::cout<<"File Size Old: "<<file_nameOld.size()<<std::endl;

  //NEW SIMULATION***************************************

  std::cout<<"*************NEW SIMULATION*************"<<std::endl;
  for (unsigned long int file=0;file<(file_nameNew.size()-1);file++){
    std::string pathNew;
    pathNew=file_nameNew[file];
    std::cout<<pathNew.c_str()<<std::endl;
    
    myfileNewSim.open(pathNew);

    std::string line;
    int countline = 0;

    //This loop doesn't compile with the ROOT compiler it works with ++ compiler:                                                                            
    while(std::getline(myfileNewSim, line)){
      std::stringstream ss(line);
      //read the resolution
      if (countline==16){ss >> resNew;}
      //read the error in the resolution
      if (countline==17){ss >> res_errorNew;}
      //read the efficiency
      if (countline==19){ss >> effNew;}
      //read the efficiency error
      if (countline==20){ss >> eff_errorNew;}
      //read the mean pulse height (energy)                                                                                                                
      if (countline==29){ ss >> val; ss >> meanENew;}
      //read the chi2
      if (countline==31){ ss >> val; ss >> chi2New;}

	countline++;
    }

  if(resNew<0){resNew=resNew*(-1);}
  resolutionvNew.push_back(resNew);
  res_errorvNew.push_back(res_errorNew);
  efficiencyvNew.push_back(effNew);
  eff_errorvNew.push_back(eff_errorNew);
  chi2vNew.push_back(chi2New);
  meanEvNew.push_back(abs((Etrue-meanENew)/Etrue));
  std::cout<<"Res= "<<resNew<<"+-"<<res_errorNew<<std::endl;
  std::cout<<"Eff= "<<effNew<<"+-"<<eff_errorNew<<std::endl;
  std::cout<<"MeanE= "<<meanENew<<std::endl;
  std::cout<<"Chi2= "<<chi2New<<std::endl;
  myfileNewSim.close();
}

  readfileNew.close();
 

//OLD SIMULATION*************************************** 

 std::cout<<"*************OLD SIMULATION*************"<<std::endl;
 for (unsigned long int file=0;file<(file_nameOld.size()-1);file++){
   std::string pathOld;
   pathOld=file_nameOld[file];
   std::cout<<pathOld.c_str()<<std::endl;

   myfileOldSim.open(pathOld);

   std::string line;
   int countline = 0;

   //This loop doesn't compile with the ROOT compiler it works with ++ compiler:                                                                              
   while(std::getline(myfileOldSim, line)){
     std::stringstream ss(line);
     //read the resolution                                                                                                                                    
     if (countline==16){ss >> resOld;}
     //read the error in the resolution                                                                                                                       
     if (countline==17){ss >> res_errorOld;}
     //read the efficiency                                                                                                                                    
     if (countline==19){ss >> effOld;}
     //read the efficiency error
     if (countline==20){ss >> eff_errorOld;}
     //read the mean pulse height (energy)                                                                                                                  
     if (countline==29){ ss >> val; ss >> meanEOld;}
     //read the chi2                                                                                                                                        
     if (countline==31){ ss >> val; ss >> chi2Old;}
     countline++;
   }

   if(resOld<0){resOld=resOld*(-1);}
   resolutionvOld.push_back(resOld);
   res_errorvOld.push_back(res_errorOld);
   efficiencyvOld.push_back(effOld);
   eff_errorvOld.push_back(eff_errorOld);
   chi2vOld.push_back(chi2Old);
   meanEvOld.push_back(abs((Etrue-meanEOld)/Etrue));
   std::cout<<"Res= "<<resOld<<"+-"<<res_errorOld<<std::endl;
   std::cout<<"Eff= "<<effOld<<"+-"<<eff_errorOld<<std::endl;
   std::cout<<"MeanE= "<<meanEOld<<std::endl;
   std::cout<<"Chi2= "<<chi2Old<<std::endl;
   myfileOldSim.close();
 }

 readfileOld.close();



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



 TGraphErrors *grResNew = new TGraphErrors(n,&rate[0],&resolutionvNew[0], &rate_error[0],&res_errorvNew[0]);
 grResNew->SetMarkerColor(kRed+1);
 grResNew->Draw("same,p");

 TGraphErrors *grResOld = new TGraphErrors(n,&rate[0],&resolutionvOld[0], &rate_error[0],&res_errorvOld[0]);
 grResOld->SetMarkerColor(kAzure+1);
 grResOld->Draw("same,p");

 std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L);
 char* MLchar = const_cast<char*>(ML.c_str());
 TLatex latex1;
 latex1.SetTextSize(0.05);
 latex1.DrawLatex(0,1,MLchar);
 latex1.DrawLatex(0,1,"Time Sample=0.05s, #sigma_{noise}=0.32mV");

 auto legend = new TLegend(0.1,0.7,0.48,0.9);
 legend->AddEntry(grResOld,"Old Simulation","p");
 legend->AddEntry(grResNew,"New Simulation","p");
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


   TGraphErrors *grEffNew = new TGraphErrors(n,&rate[0],&efficiencyvNew[0], &rate_error[0],&eff_errorvNew[0]);
   grEffNew->SetMarkerColor(kRed+1);
   grEffNew->Draw("same,p");

   TGraphErrors *grEffOld = new TGraphErrors(n,&rate[0],&efficiencyvOld[0], &rate_error[0],&eff_errorvOld[0]);
   grEffOld->SetMarkerColor(kAzure+1);
   grEffOld->Draw("same,p");

   std::string ML = "M="+std::to_string(M)+", L="+std::to_string(L);
   char* MLchar = const_cast<char*>(ML.c_str());
   TLatex latex1;
   latex1.SetTextSize(0.05);
   latex1.DrawLatex(0,1,MLchar);
   latex1.DrawLatex(0,1,"Time Sample=0.05s, #sigma_{noise}=0.32mV");

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   legend->AddEntry(grEffOld,"Old Simulation","p");
   legend->AddEntry(grEffNew,"New Simulation","p");
   legend->Draw("same");


}

  
}
