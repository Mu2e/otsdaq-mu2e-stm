#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

void plotGenMomentum(){
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
  TFile f("500000_347keV_spheremomentum_STM1maway.root");

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);
  
  // Get ntuple
  TNtuple* ntupleSteps = (TNtuple*)f.Get("Steps");
  TNtuple* ntupleEvents = (TNtuple*)f.Get("Events");


  TH3D* hpxpypzgen = new TH3D("hpxpypzgen", "Momentum at production;px;py;pz", 1000, -1, 1, 1000, -1, 1,1000,-1,1);
 


  //Define the Ntuples
  ntupleSteps->Draw("Xpos:Ypos:Zpos:EventID:EdepStep:StepLengthStep:PDGID","EventID>-1","goff");
  ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons:pxgen:pygen:pzgen","electrons>-1","goff");

  //Number of total entries in all the events
  std::cout<<ntupleSteps->GetSelectedRows()<<std::endl;
  std::cout<<ntupleEvents->GetSelectedRows()<<std::endl;

  //From the tuple to a pointer
  Double_t *vedepevent  = ntupleEvents->GetVal(0);
  Double_t *vtracklengthevent  = ntupleEvents->GetVal(1);
  Double_t *velectrons  = ntupleEvents->GetVal(2);   
  Double_t *vpx  = ntupleEvents->GetVal(3);
  Double_t *vpy  = ntupleEvents->GetVal(4);
  Double_t *vpz  = ntupleEvents->GetVal(5);


   //From the pointer to a vector;                                                                                                             
   std::vector<double> PX,PY,PZ;

   PX.clear();
   PY.clear();
   PZ.clear();


  
   for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    
     PX.push_back(*vpx);
     PY.push_back(*vpy);
     PZ.push_back(*vpz);

     hpxpypzgen->Fill(*vpx,*vpy,*vpz);
    
     vpx++;
     vpy++;
     vpz++;
   }

 
   
     //ntupleEvents->Draw("pxgen:pygen;pzgen");
   hpxpypzgen->SetMarkerColor(kCyan-3); 
   hpxpypzgen->Draw("");
 
 
}  
