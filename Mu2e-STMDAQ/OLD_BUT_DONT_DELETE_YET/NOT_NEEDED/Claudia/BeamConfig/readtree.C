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


void readtree() {

  //double spillRatekHz=10;
  //int n_maininjector_cycles=2572;
  //std::string  rootname="/data1/cgarcia/DATA/Claudia/Al_GapRate/Spill_"+std::to_string(int(spillRatekHz))+"_Alsim_"+std::to_string(n_maininjector_cycles)+"maincycles.root";
  std::string  rootname="/data1/cgarcia/DATA/Claudia/Al_GapRate/Spill_50_Alsim_2572maincycles.root";

  TFile *input;
 TTree* treeread_HPGe;
 TBranch *HPGe_times =0;
 TBranch *XRay_energies =0;
 TBranch *gap_rate =0;
 TBranch *gap_num =0;
 
 std::vector<double> *HPGe_timesp=0;
 std::vector<double> *XRay_energiesp=0;
 std::vector<double> *gap_ratep=0;
 std::vector<double> *gap_nump=0;


  input=new TFile(rootname.c_str());

  treeread_HPGe=(TTree*)input->Get("tree_HPGe");
  treeread_HPGe->SetBranchAddress("HPGe_times", &HPGe_timesp);
  treeread_HPGe->SetBranchAddress("XRay_energies",&XRay_energiesp);
  treeread_HPGe->SetBranchAddress("gap_rate",&gap_ratep);
  treeread_HPGe->SetBranchAddress("gap_num",&gap_nump);

  unsigned long int entries = treeread_HPGe->GetEntries();
  std::cout<<"Entries: "<<entries<<std::endl;

  treeread_HPGe->GetEntry(0);
  unsigned long int size = HPGe_timesp->size();
  std::cout<<"size: "<<size<<std::endl;


  //treeread_HPGe->Draw("HPGe_times");
  //treeread_HPGe->Draw("XRay_energies"); 
  //treeread_HPGe->Draw("gap_rate"); 
  treeread_HPGe->Draw("gap_num"); 


}
