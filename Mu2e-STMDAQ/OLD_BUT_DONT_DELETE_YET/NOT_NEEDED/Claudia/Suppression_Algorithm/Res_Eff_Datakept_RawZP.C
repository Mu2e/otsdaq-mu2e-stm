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
#include "TGraphErrors.h"
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

using namespace std;
//If print =0 plot resolution 
//If print =1 plot efficiency                                                                                                  
//If print =2 plot fraction of raw data kept

//For the rate write: "1", "5" , "10" , ... , "200"
void Res_Eff_Datakept_RawZP(int print,  std::string pathfile){
 

 gROOT->SetStyle("ATLAS");  

  std::ifstream myfile;
  std::string filename=pathfile;
  std::cout<<filename<<std::endl;
  vector<double> ratev, rate_errorv, resRawv, resRaw_errorv, resZPv, resZP_errorv, effRawv, effRaw_errorv, effZPv, effZP_errorv, PeaksGenv, PeaksRawv, PeaksZPv, datakeptv, fractionkeptv, fractionkept_errorv;
  double rate, resRaw, resRaw_error, resZP, resZP_error, PeaksGen, PeaksRaw, PeaksZP, datakept;
  myfile.open (filename);
  std::string line;  

  int countline=0;

  //Compiler: .x Res_Eff_Datakept_RawZP.C+(0,"/path/file");
    
  while(std::getline(myfile, line)){
    std::stringstream ss(line);
    if(countline>0){
      ss >> rate;
      ratev.push_back(rate);
      rate_errorv.push_back(0);
      ss >> PeaksGen;
      PeaksGenv.push_back(PeaksGen);
      ss >> resRaw;
      resRawv.push_back(resRaw);
      ss >> resRaw_error;
      resRaw_errorv.push_back(resRaw_error);
      ss >> PeaksRaw;
      PeaksRawv.push_back(PeaksRaw);
      ss >> datakept;
      datakeptv.push_back(datakept);
      ss >> resZP;
      resZPv.push_back(resZP);
      ss >> resZP_error;
      resZP_errorv.push_back(resZP_error);
      ss >> PeaksZP;
      PeaksZPv.push_back(PeaksZP);

      
      std::cout<<"Rate "<<rate<<", Peaks Gen "<<PeaksGen<<" Res Raw(keV): "<<resRaw<<"+-"<<resRaw_error<<" Peaks Raw: "<<PeaksRaw<<" Data kept after ZP: "<<datakept<<" Res ZP: "<<resZP<<"+-"<<resZP_error<<" Peaks ZP: "<<PeaksZP<<std::endl;
    }
    countline++;
  }

  int n=13;

  //Calculate resolution
  if(print==0){
    double xx1[2]={0,200};
    double yy1[2]={0,4};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
    graph1->SetTitle("");
    graph1->GetXaxis()->SetTitle("Rate [kHz]");
    graph1->GetYaxis()->SetTitle("Resolution [keV]");
    graph1->GetXaxis()->SetTitleOffset(0.9);
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");
    
    TGraphErrors *grSupM400L200 = new TGraphErrors(n,&ratev[0],&resZPv[0],&rate_errorv[0],&resZP_errorv[0]);
    grSupM400L200->SetMarkerColor(kRed);
    grSupM400L200->SetMarkerStyle(21);
    grSupM400L200->Draw("same,p");

    TGraphErrors *grRawM400L200 = new TGraphErrors(n,&ratev[0],&resRawv[0],&rate_errorv[0],&resRaw_errorv[0]);
    grRawM400L200->SetMarkerColor(kViolet+2);
    grRawM400L200->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");
    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grRawM400L200,"Raw","p");
    legend->AddEntry(grSupM400L200,"Suppressed","p");
    legend->Draw("same");

    

}
  //Calculate the efficiency
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




    for(int i=0;i<n;i++){
      effRawv.push_back(PeaksRawv.at(i)/PeaksGenv.at(i));
      effZPv.push_back(PeaksZPv.at(i)/PeaksGenv.at(i));

      effRaw_errorv.push_back(effRawv.at(i)/sqrt(PeaksRawv.at(i)));
      effZP_errorv.push_back(effZPv.at(i)/sqrt(PeaksZPv.at(i)));
    }


    for(int i=0;i<n;i++){
      std::cout<<"Rate: "<<ratev.at(i)<<"kHz Eff Raw: "<< effRawv.at(i)<<"+-"<<effRaw_errorv.at(i)<<std::endl;
      std::cout<<"Rate: "<<ratev.at(i)<<"kHz Eff ZP: "<< effZPv.at(i)<<"+-"<<effZP_errorv.at(i)<<std::endl;
     
    }






    TGraphErrors *grSupM400L200 = new TGraphErrors(n,&ratev[0],&effZPv[0],&rate_errorv[0],&effZP_errorv[0]);
    grSupM400L200->SetMarkerColor(kRed);
    grSupM400L200->SetMarkerStyle(21);
    grSupM400L200->Draw("same,p");

    TGraphErrors *grRawM400L200 = new TGraphErrors(n,&ratev[0],&effRawv[0],&rate_errorv[0],&effRaw_errorv[0]);
    grRawM400L200->SetMarkerColor(kViolet+2);
    grRawM400L200->Draw("same,p");

    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");
    auto legend = new TLegend(0.1,0.7,0.48,0.9);
    legend->AddEntry(grRawM400L200,"Raw","p");
    legend->AddEntry(grSupM400L200,"Suppressed","p");
    legend->Draw("same");





  }


  //Calculate fraction of data kept
  if(print==2){
    double xx1[2]={0,200};
    double yy1[2]={0,1};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
    graph1->SetTitle("");
    graph1->GetXaxis()->SetTitle("Rate [kHz]");
    graph1->GetYaxis()->SetTitle("Fraction of Raw data kept");
    graph1->GetXaxis()->SetTitleOffset(0.9);
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");
  
    //Number of ADC values in each input file: 320052083
    double originalNumberOfADC=320052083;

    for(int i=0;i<n;i++){
      fractionkeptv.push_back(datakeptv.at(i)/originalNumberOfADC);
      fractionkept_errorv.push_back(0);
    }


    for(int i=0;i<n;i++){
      std::cout<<"Fraction kept: "<<fractionkeptv.at(i)<<std::endl;
    }


    TGraphErrors *grSupM400L200 = new TGraphErrors(n,&ratev[0],&fractionkeptv[0],&rate_errorv[0],&fractionkept_errorv[0]);
    grSupM400L200->SetMarkerColor(kViolet-8);
    grSupM400L200->SetMarkerStyle(21);
    grSupM400L200->Draw("same,p");

 
  }
 





  //Calculate the difference in resolution of raw and suppressed
  if(print==3){
    double xx1[2]={0,200};
    double yy1[2]={-0.1,0.1};
    TGraph *graph1 = new TGraph (2,xx1,yy1);
    graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
    graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
    graph1->SetTitle("");
    graph1->GetXaxis()->SetTitle("Rate [kHz]");
    graph1->GetYaxis()->SetTitle("#Delta#sigma=(#sigma_{Raw}-#sigma_{Suppressed}) [keV]");
    graph1->GetXaxis()->SetTitleOffset(0.9);
    graph1->SetMarkerStyle(1);
    graph1->Draw("ap");
    
    vector<double> difratesv, difrates_errorv;

    for(int i=0;i<n;i++){
      difratesv.push_back(resRawv[i]-resZPv[i]);
      difrates_errorv.push_back((resRawv[i]-resZPv[i])*sqrt((resRaw_error/resRaw)*(resRaw_error/resRaw)+(resZP_error/resZP)*(resZP_error/resZP)));
      
      cout<<"#Delta#sigma=(#sigma_{Raw}-#sigma_{Suppressed})"<<resRawv[i]-resZPv[i]<<"+-"<<(resRawv[i]-resZPv[i])*sqrt((resRaw_error/resRaw)*(resRaw_error/resRaw)+(resZP_error/resZP)*(resZP_error/resZP))<<std::endl;
    }
 
   TGraphErrors *grdiffM400L200 = new TGraphErrors(n,&ratev[0],&difratesv[0],&rate_errorv[0],&difrates_errorv[0]);
    grdiffM400L200->SetMarkerColor(kBlue);
    grdiffM400L200->SetMarkerStyle(20);
    grdiffM400L200->Draw("same,p");


    TLatex latex;
    latex.SetTextSize(0.05);
    latex.DrawLatex(.2,.9,"M=400, L=200");


    TLine *line=new TLine(xx1[0],0,xx1[1],0);
    line->SetLineColor(kBlue);
    line->SetLineWidth(2);
    line->SetLineStyle(3); 
    line->Draw("same"); 
  }



  
}
