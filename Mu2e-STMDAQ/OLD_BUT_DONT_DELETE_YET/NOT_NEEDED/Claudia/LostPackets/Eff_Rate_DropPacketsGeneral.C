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
#include "TGraphErrors.h"

using namespace std;
//If print =0 plot resolution 
//If print =1 plot efficiency

void Eff_Rate_DropPacketsGeneral(int print,  std::string rate){
  gROOT->SetStyle("ATLAS"); 

  
  TCanvas *c1  = new TCanvas();

  std::ifstream myfile;

  double res, res_error, eff, eff_error;

  vector<double> resv,effv,res_errorv,eff_errorv,droppackets,droppackets_error;
  int drop[9] = {0,10,20,35,50,60,70,80,90}; //(% dropped packets)

  std::string val;

  int num = 9;
  

  for(int i = 0; i<num; i++){
    droppackets.push_back(drop[i]);
    int droperror = 0;
    droppackets_error.push_back(droperror);
  }
  


  for(int i = 0; i < num ;i++){
  
    std::string filename= "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/DropPackets/Drop4kPackets_SimulationEff/run_"+rate+"kHz_"+std::to_string(drop[i])+"percent_th500.txt";
      myfile.open (filename);
      std::cout<<"filename: "<<filename<<std::endl;

      std::string line;
      int countline = 0;
      
      //This loop doesn't compile with the ROOT compiler it works with ++ compiler
      while(std::getline(myfile, line)){
	std::stringstream ss(line);
	//read the resolution  
	if (countline==14){ss >> res;
	  std::cout<<"Res "<<res<<std::endl;
	}
	//read the error in the resolution
	if (countline==15){ss >> res_error;
	  std::cout<<"+- "<<res_error<<std::endl;
	}
	//read the efficiency
	if (countline==17){ss >> eff;
	  std::cout<<"Eff "<<eff<<std::endl;
	}
	//read the efficiency error
	if (countline==18){ss >> eff_error;
	  std::cout<<"+- "<<eff_error<<std::endl;
	}
	countline++;
      }//while  

      if(res<0){res=res*(-1);}
      
      
      resv.push_back(res);
      res_errorv.push_back(res_error);
      effv.push_back(eff);
      eff_errorv.push_back(eff_error);

      myfile.close();
  }//for
std::cout<<num<<std::endl;




//Plot MWD efficiency
 if(print==1){
 double xx1[2]={0,100};
 double yy1[2]={0,1};
 TGraph *graph1 = new TGraph (2,xx1,yy1);
 graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
 graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
 graph1->SetTitle("");
 graph1->GetXaxis()->SetTitle("Dropped Packets [%]");
 graph1->GetYaxis()->SetTitle("MWD Efficiency");
 graph1->GetXaxis()->SetTitleOffset(0.9);
 graph1->SetMarkerStyle(1);
 graph1->Draw("ap");


 TGraphErrors *greff = new TGraphErrors(num,&droppackets[0],&effv[0],&droppackets_error[0],&eff_errorv[0]);
 greff->SetMarkerColor(kViolet+1);
 greff->Draw("same,p");


 
 std::string pngeff = "Eff_DropPackets"+rate+"kHz_4kpacket_prob_th500.png";
 char* pngeff_char=const_cast<char*>(pngeff.c_str());

 std::string pdfeff = "Eff_DropPackets"+rate+"kHz_4kpacket_prob_th500.pdf";
 char* pdfeff_char=const_cast<char*>(pdfeff.c_str());


 std::string rate_string = rate+" kHz";
 char* rate_char = const_cast<char*>(rate_string.c_str());
 TLatex latex;
 latex.SetTextSize(0.05);
 latex.SetTextAlign(13);
 latex.DrawLatex(10,0.6,rate_char);

 c1->Print(pngeff_char);
 c1->Print(pdfeff_char);

 }




 //Plot resolution
 if(print==0){
   double xx1[2]={0,100};
   double yy1[2]={0,5};
   TGraph *graph1 = new TGraph (2,xx1,yy1);
   graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
   graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
   graph1->SetTitle("");
   graph1->GetXaxis()->SetTitle("Dropped Packets [%]");
   graph1->GetYaxis()->SetTitle("Resolution [keV]");
   graph1->GetXaxis()->SetTitleOffset(0.9);
   graph1->SetMarkerStyle(1);
   graph1->Draw("ap");

   TGraphErrors *grres = new TGraphErrors(num,&droppackets[0],&resv[0],&droppackets_error[0],&res_errorv[0]);
   grres->SetMarkerColor(kTeal+1);
   grres->Draw("same,p");

   std::string pngres = "Res_DropPackets"+rate+"kHz_4kpacket_prob_th500.png";
   char* pngres_char=const_cast<char*>(pngres.c_str());

   std::string pdfres = "Res_DropPackets"+rate+"kHz_4kpacket_prob_th500.pdf";
   char* pdfres_char=const_cast<char*>(pdfres.c_str());

   std::string rate_string = rate+" kHz";
   char* rate_char = const_cast<char*>(rate_string.c_str());
   TLatex latex;
   latex.SetTextSize(0.05);
   latex.SetTextAlign(13);
   latex.DrawLatex(10,yy1[1]/1.25,rate_char);


   c1->Print(pngres_char);
   c1->Print(pdfres_char);
}




 
}
