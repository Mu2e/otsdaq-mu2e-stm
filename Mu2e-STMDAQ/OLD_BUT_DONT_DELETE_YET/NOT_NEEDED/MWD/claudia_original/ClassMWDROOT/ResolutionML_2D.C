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
#include "TGaxis.h"

using namespace std;
//If print =0 plot resolution 
//If print =1 plot efficiency                                                                                                  
//If print =2 plot chi2
//If print =3 plot energy shifting 

//For the rate write: "1", "5" , "10" , ... , "200"
void ResolutionML_2D(int print,  std::string rate){
  //gROOT->SetStyle("ATLAS");
  int palette_number = 91;
  gStyle->SetPalette(palette_number);
  

  std::ifstream myfile;
  
  int Mnum = 9;
  int Lnum = 20;


  //Simulated energy in keV
  //double Etrue = 675.36;
  //double Etrue = 511; 
  double Etrue = 662;

  double res, res_error, eff, chi2, meanE;

  vector<double> Mv, Lv, resolutionv,efficiencyv, chi2v, meanEv;
  std::string val, rate_zero;
  auto hprof = new TProfile2D("","",20,300,10000,20,10,10000,-40,40);

  int M[9]={300,400,500,600,700,800,900,1000,2000};
  int L[20]={10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900,1000,2000};

  for(int i = 0; i < Mnum ;i++){
    for (int j = 0; j < Lnum ; j++){
      if(L[j] >= M[i]){continue;}
      
      std::string filename;

      if(Etrue==675.36){
	if(stoi(rate)<10){rate_zero="0"+rate;}
	else{rate_zero=rate;}
	filename= "/data1/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/Resolution_Efficiency_ML/"+rate+"kHz/run_"+rate_zero+"_M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+".txt";}
     
      
      else if(Etrue==511){
	if(stoi(rate)<10){rate_zero="00"+rate;}
	else if(stoi(rate)>=10&&stoi(rate)<100){rate_zero="0"+rate;}
	else{rate_zero=rate;}
	filename= "/data1/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson_511keV/Resolution_Efficiency_ML/"+rate+"kHz/run_"+rate_zero+"_M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+".txt"; 
      }

      else if(Etrue==662){
        if(stoi(rate)<10){rate_zero="0"+rate;}
        else{rate_zero=rate;}
        filename= "/data1/cgarcia/DATA/Claudia/GenDataehHPGeSim/662keV_0.32mV/Resolution_Efficiency_ML/"+rate+"kHz/run_"+rate_zero+"kHz_M"+std::to_string(M[i])+"_L"+std::to_string(L[j])+".txt";
	//std::cout<<"rootfile: "<<filename<<std::endl;
      } 

    
      myfile.open (filename);
      //std::cout<<"filename: "<<filename<<std::endl;

      std::string line;
      int countline = 0;
      //while(getline(myfile, line)){std::cout<<"hi"<<std::endl;}
      
      //read first 11 lines
      //This loop doesn't compile with the ROOT compiler it works with ++ compiler: .x ResolutionML_2D.C+(0,"1");
      /*OLD version
      while(std::getline(myfile, line)){
	std::stringstream ss(line);
	//read the resolution  
	if (countline==14){ss >> res;
	  //std::cout<<"Res "<<res<<std::endl;
	}
	//read the error in the resolution
	if (countline==15){ss >> res_error;}
	//read the efficiency
	if (countline==17){ss >> eff;}
	if(Etrue==675.36){
	  //read the mean pulse height (energy)
	  if (countline==25){ ss >> val; ss >> meanE;}
	  //read the chi2                                                                                                 
	  if (countline==27){ ss >> val; ss >> chi2;}
	}
	else if (Etrue==511){
	//read the mean pulse height (energy)
	if (countline==27){ ss >> val; ss >> meanE;} 
	//read the chi2
	if (countline==29){ ss >> val; ss >> chi2;}
	}

	countline++;
	}*/

      while(std::getline(myfile, line)){
	std::stringstream ss(line);
        //read the resolution
        if (countline==17){ss >> res;
          //std::cout<<"Res "<<res<<std::endl;
        }
        //read the error in the resolution
        if (countline==18){ss >> res_error;}
        //read the efficiency
        if (countline==20){ss >> eff;}
	//read the mean pulse height (energy)
	if (countline==30){ ss >> val; ss >> meanE;}
	//read the chi2
	if (countline==32){ ss >> val; ss >> chi2;}
        
        countline++;
      }

      //std::cout<<"countline: "<<countline<<std::endl;      
      //to account for inclomplete txt files
      if(countline<32){res=0;res_error=0;eff=0;meanE=0;chi2=0;}

      if(res<0){res=res*(-1);}
      //Fill the 2D histogram with M, L and the resolution in color bar      
      std::cout<<"M: "<<M[i]<<" L: "<<L[j]<< " res: " <<res<<" +- "<<res_error<<" eff: "<<eff<<" chi2: "<<chi2<<" True Energy: "<<Etrue<<" Reco Energy: "<<meanE<<" shift: "<<(Etrue-meanE)/Etrue<<std::endl;
      
     
      Mv.push_back(M[i]);
      Lv.push_back(L[j]);
      resolutionv.push_back(res);
      efficiencyv.push_back(eff);
      chi2v.push_back(chi2);
      meanEv.push_back(abs((Etrue-meanE)/Etrue));
     
      //hprof->Fill(M[i],L[j],res,1);
     myfile.close();
      
    }
  }


  double minres = *min_element(resolutionv.begin(), resolutionv.end());
  cout<<"Best resolution: "<<minres<<" keV"<<std::endl;

  double maxeff = *max_element(efficiencyv.begin(), efficiencyv.end());
  cout<<"Best efficiency: "<<maxeff<<std::endl;


  TCanvas *c1  = new TCanvas("c1","c1",0,0,700,500);

  //Rate>=180
  //double latexmin=380;
  //double latexmax1=200;
  //double latexmax2=300;
  
  //Rate<180
  double latexmin=500;
  double latexmax1=300;
  double latexmax2=500;
  

  double xx1[2]={300, 2000};
  double yy1[2]={10, 1500};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitleColor(kBlack);
  graph1->GetXaxis()->SetTitleSize(0.04);
  graph1->GetXaxis()->SetLabelSize(0.04);
  graph1->GetYaxis()->SetTitleSize(0.04);
  graph1->GetYaxis()->SetLabelSize(0.04);
  graph1->GetXaxis()->SetTitle("M");
  graph1->GetYaxis()->SetTitle("L");
  graph1->GetYaxis()->SetTitleOffset(1.3);
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  


//Resolution
  if(print==0){
    TGraph2D *gr = new TGraph2D(Mv.size(),&Mv[0],&Lv[0],&resolutionv[0]);
    /*gr->SetTitle(";M;L;");
    gr->GetHistogram()->GetXaxis()->SetTitleColor(kBlack);
    gr->GetHistogram()->GetXaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetLabelSize(0.04);*/
    gr->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
    gr->Draw("same,colz");
    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13);
    latex.DrawLatex(latexmin,yy1[1]-latexmax1,"Resolution [keV]");
    std::string rate_string = rate+" kHz";
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(latexmin,yy1[1]-latexmax2,rate_char);


    std::string png = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Res_NewSim.png";
    std::string pdf = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Res_NewSim.pdf";
    char* png_char = const_cast<char*>(png.c_str());
    char* pdf_char = const_cast<char*>(pdf.c_str());

    gPad->SetTicks();
    gPad->RedrawAxis();

    //c1->Print(pdf_char,"pdf");
    //c1->Print(png_char,"png");
  }

  //Efficiency
  if(print==1){
    TGraph2D *gr = new TGraph2D(Mv.size(),&Mv[0],&Lv[0],&efficiencyv[0]);   
    /*gr->SetTitle(";M;L;");
    gr->GetHistogram()->GetXaxis()->SetTitleColor(kBlack);
    gr->GetHistogram()->GetXaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetLabelSize(0.04);*/
    gr->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
    gr->Draw("same, colz");
    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13);
    latex.DrawLatex(latexmin,yy1[1]-latexmax1,"Efficiency");
    std::string rate_string = rate+" kHz";
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(latexmin,yy1[1]-latexmax2,rate_char);



    std::string png = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Eff_NewSim.png";
    std::string pdf = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Eff_NewSim.pdf";
    char* png_char = const_cast<char*>(png.c_str());
    char* pdf_char = const_cast<char*>(pdf.c_str());
  
    gPad->SetTicks();
    gPad->RedrawAxis();

    //c1->Print(pdf_char,"pdf");
    //c1->Print(png_char,"png");
  }

  //Chi2
  if(print==2){
    TGraph2D *gr = new TGraph2D(Mv.size(),&Mv[0],&Lv[0],&chi2v[0]); 
    /*gr->GetHistogram()->GetXaxis()->SetTitleColor(kBlack);
    gr->GetHistogram()->GetXaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetLabelSize(0.04);*/
    gr->GetHistogram()->GetZaxis()->SetLabelSize(0.04);
    gr->Draw("same, colz");
    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13);
    latex.DrawLatex(latexmin,yy1[1]-latexmax1,"#chi^{2}");
    std::string rate_string = rate+" kHz";
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(latexmin,yy1[1]-latexmax2,rate_char);

    std::string png = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Chi2_NewSim.png";
    std::string pdf = rate+"kHz_"+std::to_string(int(Etrue))+"keV_Chi2_NewSim.pdf";
    char* png_char = const_cast<char*>(png.c_str());
    char* pdf_char = const_cast<char*>(pdf.c_str());

    gPad->SetTicks();
    gPad->RedrawAxis();

    //c1->Print(pdf_char,"pdf");
    //c1->Print(png_char,"png");
  }



  //Energy shifting
  if(print==3){
    TGraph2D *gr = new TGraph2D(Mv.size(),&Mv[0],&Lv[0],&meanEv[0]);
    /*gr->SetTitle(";M;L;");
    gr->GetHistogram()->GetXaxis()->SetTitleColor(kBlack);
    gr->GetHistogram()->GetXaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetXaxis()->SetLabelSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetTitleSize(0.04);
    gr->GetHistogram()->GetYaxis()->SetLabelSize(0.04);*/
    gr->GetHistogram()->GetZaxis()->SetLabelSize(0.03);
    gr->Draw("same,colz");

    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13);
    latex.DrawLatex(latexmin,yy1[1]-latexmax1,"Height shifting");
    std::string rate_string = rate+" kHz";
    char* rate_char = const_cast<char*>(rate_string.c_str());
    latex.DrawLatex(latexmin,yy1[1]-latexmax2,rate_char);
  
    std::string png = rate+"kHz_"+std::to_string(int(Etrue))+"keV_EnergyShifting_NewSim.png";
    std::string pdf = rate+"kHz_"+std::to_string(int(Etrue))+"keV_EnergyShifting_NewSim.pdf";
    char* png_char = const_cast<char*>(png.c_str());
    char* pdf_char = const_cast<char*>(pdf.c_str());


    gPad->SetTicks();
    gPad->RedrawAxis();

    //c1->Print(pdf_char,"pdf");
    //c1->Print(png_char,"png");
  }




}
