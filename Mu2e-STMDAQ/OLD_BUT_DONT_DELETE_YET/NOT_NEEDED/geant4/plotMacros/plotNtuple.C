#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

void plotNtuple(){
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

  TH1* hx = new TH1F("hx", "x position of the hit in each step", 100, 200., 700.);
  hx->GetXaxis()->SetTitle("x (mm)");
  TH1* hy = new TH1F("hy", "y position of the hit in each step", 100, -20., 20.);
  hy->GetXaxis()->SetTitle("y (mm)");
  TH1* hz = new TH1F("hz", "z position of the hit in each step", 100, 4000., 12000.);
  hz->GetXaxis()->SetTitle("z (mm)");
  TH1* heventid = new TH1F("heventid", "EventID in each event", 200, 0., 100000.);
  heventid->GetXaxis()->SetTitle("EventID");
  TH1* hedepsteps = new TH1F("hedepsteps", "Edep by electrons+photons in each step", 100, 0., 0.5);
  hedepsteps->GetXaxis()->SetTitle("Edep Step e- (MeV)");
  TH1* hsteplengthstep = new TH1F("hsteplengthstep", "Step Length in each step", 100, 0., 1.);
  hsteplengthstep->GetXaxis()->SetTitle("Step Length (mm)");
  TH1* hPDGID = new TH1F("hPDGID", "PDG ID", 1000, 10., 25.);
  hPDGID->GetXaxis()->SetTitle("PDGID");
 

  TH2* hzx = new TH2F("hzx", "zx position of the hit in each step", 1000, 4000, 12000, 1000, 200, 700);
  hzx->GetXaxis()->SetTitle("z (mm)");
  hzx->GetYaxis()->SetTitle("x (mm)");  
  TH2* hzy = new TH2F("hzy", "zy position of the hit in each step", 1000, 4000, 12000, 1000, -20, 20);
  hzy->GetXaxis()->SetTitle("z (mm)");
  hzy->GetYaxis()->SetTitle("y (mm)");  
  TH2* hxy = new TH2F("hxy", "xy position of the hit in each step", 1000, 200, 700, 1000, -20, 20);
  hxy->GetXaxis()->SetTitle("x (mm)");
  hxy->GetYaxis()->SetTitle("y (mm)");

  TH1* hedepevent = new TH1F("hedepevent", "Total Edep by electrons generated in each event", 100, 0., 0.5);
  hedepevent->GetXaxis()->SetTitle("Total Edep e- (MeV)");  
  TH1* htracklengthevent = new TH1F("htracklengthevent", "Total Track Length by electrons in each event", 100, 0., 10.);
  htracklengthevent->GetXaxis()->SetTitle("Total Track Length (mm)");  
  TH1* helectrons = new TH1F("helectrons", "#Electrons in each event", 20, 0., 20.);
  helectrons->GetXaxis()->SetTitle("#Electrons");

  //Define the Ntuples
  ntupleSteps->Draw("Xpos:Ypos:Zpos:EventID:EdepStep:StepLengthStep:PDGID","EventID>-1","goff");
  ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons","electrons>-1","goff");

    //Long64_t numberofpoints= ntuple->GetEstimate();
    //Double_t rows=ntuple-> GetSelectedRows();

  //Number of total entries in all the events
  std::cout<<ntupleSteps->GetSelectedRows()<<std::endl;
  std::cout<<ntupleEvents->GetSelectedRows()<<std::endl;

  //From the tuple to a pointer
   Double_t *vx  = ntupleSteps->GetVal(0);
   Double_t *vy  = ntupleSteps->GetVal(1);
   Double_t *vz  = ntupleSteps->GetVal(2);
   Double_t *veventid  = ntupleSteps->GetVal(3);
   Double_t *vedepstep  = ntupleSteps->GetVal(4);
   Double_t *vsteplengthstep  = ntupleSteps->GetVal(5);
   Double_t *vPDGID  = ntupleSteps->GetVal(6);

   Double_t *vedepevent  = ntupleEvents->GetVal(0);
   Double_t *vtracklengthevent  = ntupleEvents->GetVal(1);
   Double_t *velectrons  = ntupleEvents->GetVal(2);
   // cout<<*vx<<endl;
   //for(int i=0;i<ntuple->GetSelectedRows();i++){
   //     std::cout<<*vx<<std::endl;
   //	  vx++;
   // }

   //number of events
   //int entries=ntuple->GetEntries();//200000
   int eventid=0;


   //From the pointer to a vector;                                                                                                             
   std::vector<double> Xpos, Ypos, Zpos, EdepStep, StepLengthStep;
   std::vector<int> EventID, PDGID;

   Xpos.clear();
   Ypos.clear();
   Zpos.clear();
   EdepStep.clear();
   StepLengthStep.clear();
   EventID.clear();
   PDGID.clear();

   std::vector<double> EdepEvent, TrackLengthEvent, electrons;

   EdepEvent.clear();
   TrackLengthEvent.clear();
   electrons.clear();


   for(int i=0;i<ntupleSteps->GetSelectedRows();i++){
     eventid=*veventid;
     EventID.push_back(eventid);
     //std::cout<<"Eventid: "<<EventID.at(i)<<std::endl;

     Xpos.push_back(*vx);
     Ypos.push_back(*vy);
     Zpos.push_back(*vz);
     EdepStep.push_back(*vedepstep);
     StepLengthStep.push_back(*vsteplengthstep);
     PDGID.push_back(*vPDGID);

     hx->Fill(*vx);
     hy->Fill(*vy);
     hz->Fill(*vz);
     hedepsteps->Fill(*vedepstep);
     heventid->Fill(eventid);
     hsteplengthstep->Fill(*vsteplengthstep);
     hPDGID->Fill(*vPDGID);
     
     hzx->Fill(*vz,*vx);
     hzy->Fill(*vz,*vy);
     hxy->Fill(*vx,*vy);

     vx++;
     vy++;
     vz++;
     vedepstep++;
     vsteplengthstep++;
     vPDGID++;

     veventid++;
    
   }        
  
   for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    
     EdepEvent.push_back(*vedepevent);
     TrackLengthEvent.push_back(*vtracklengthevent);
     electrons.push_back(*velectrons);

     hedepevent->Fill(*vedepevent);
     htracklengthevent->Fill(*vtracklengthevent);
     helectrons->Fill(*velectrons);

     vedepevent++;
     vtracklengthevent++;
     velectrons++;
   }

   //Doesnt work 
   //for (int i=0;i<ntupleSteps->GetSelectedRows();i++){
    //std::cout<<"z: "<<Zpos.at(i)<<" x: "<<Xpos.at(i)<<std::endl;
    //std::cout<<"Eventid: "<<EventID.at(i)<<std::endl;}
 
   //TGraph *g1 = new TGraph(ntupleSteps->GetSelectedRows(),&Zpos[0],&Xpos[0]);
   //g1->Draw("ap");
   //TGraph *gr = new TGraph(ntuple->GetSelectedRows(),ntuple->GetV2(), ntuple->GetV1());
   //gr->Draw("ap"); 


   //pad 1
  c1->cd(1);
  //ntupleSteps->Draw("Xpos:Zpos");
 
  hzx->GetYaxis()->SetLabelSize(0.03);
  hzx->GetXaxis()->SetLabelSize(0.03);

  hzx->GetYaxis()->SetTitleSize(0.025);
  hzx->GetXaxis()->SetTitleSize(0.025);
  hzx->GetYaxis()->SetTitleOffset(2);
  hzx->GetXaxis()->SetTitleOffset(2);

  hzx->SetFillStyle( 3001);
  hzx->SetFillColor( kBlue+1);
 
  hzx->Draw();
 
  //ntupleSteps->Draw("EdepEvent"); 
  /* hedepevent->GetYaxis()->SetLabelSize(0.03);
  hedepevent->GetXaxis()->SetLabelSize(0.03);

  hedepevent->GetYaxis()->SetTitleSize(0.025);
  hedepevent->GetXaxis()->SetTitleSize(0.025);
  hedepevent->GetYaxis()->SetTitleOffset(2);
  hedepevent->GetXaxis()->SetTitleOffset(2);
  hedepevent->SetFillStyle( 3001);
  hedepevent->SetFillColor( kBlue+1);
  hedepevent->Draw();  
  */
  //pad 2
  c1->cd(2);
  //ntupleSteps->Draw("Ypos:Zpos");

  hzy->GetYaxis()->SetLabelSize(0.03);
  hzy->GetXaxis()->SetLabelSize(0.03);

  hzy->GetYaxis()->SetTitleSize(0.025);
  hzy->GetXaxis()->SetTitleSize(0.025);
  hzy->GetYaxis()->SetTitleOffset(2);
  hzy->GetXaxis()->SetTitleOffset(2);

  hzy->SetFillStyle( 3001);
  hzy->SetFillColor( kOrange+1);

  hzy->Draw();
  

  //ntupleSteps->Draw("electrons");
  /*helectrons->GetYaxis()->SetLabelSize(0.03);
  helectrons->GetXaxis()->SetLabelSize(0.03);

  helectrons->GetYaxis()->SetTitleSize(0.025);
  helectrons->GetXaxis()->SetTitleSize(0.025);
  helectrons->GetYaxis()->SetTitleOffset(2);
  helectrons->GetXaxis()->SetTitleOffset(2);

  helectrons->SetFillStyle( 3001);
  helectrons->SetFillColor( kOrange+1);
  helectrons->Draw();
  */
 
  /*heventid->GetYaxis()->SetLabelSize(0.03);                                                                               
 heventid->GetXaxis()->SetLabelSize(0.03);                                                                                   
                                                                                                                              
 heventid->GetYaxis()->SetTitleSize(0.025);                                                                                  
 heventid->GetXaxis()->SetTitleSize(0.025);                                                                                  
 heventid->GetYaxis()->SetTitleOffset(2);                                                                                    
 heventid->GetXaxis()->SetTitleOffset(2);         
                                                                                                                               
 heventid->SetFillStyle( 3001);                                                                                              
 heventid->SetFillColor( kOrange+1);                                                                                         
 heventid->Draw();
  */
   //pad 3
   c1->cd(3);
   //ntupleSteps->Draw("Xpos");
   hx->GetYaxis()->SetLabelSize(0.03);
   hx->GetXaxis()->SetLabelSize(0.03);

   hx->GetYaxis()->SetTitleSize(0.025);
   hx->GetXaxis()->SetTitleSize(0.025);
   hx->GetYaxis()->SetTitleOffset(2);
   hx->GetXaxis()->SetTitleOffset(2);
   hx->SetFillStyle( 3001);
   hx->SetFillColor( kOrange-8);

   hx->Draw();
   

   //ntupleSteps->Draw("EdepStep");   
   /*   hedepsteps->GetYaxis()->SetLabelSize(0.03);
   hedepsteps->GetXaxis()->SetLabelSize(0.03);

   hedepsteps->GetYaxis()->SetTitleSize(0.025);
   hedepsteps->GetXaxis()->SetTitleSize(0.025);
   hedepsteps->GetYaxis()->SetTitleOffset(2);
   hedepsteps->GetXaxis()->SetTitleOffset(2);
   hedepsteps->SetFillStyle( 3001);
   hedepsteps->SetFillColor( kBlue+4);
   hedepsteps->Draw();
   */
  //pad 4
   c1->cd(4);
   //ntupleSteps->Draw("Ypos");
   hy->GetYaxis()->SetLabelSize(0.03);
   hy->GetXaxis()->SetLabelSize(0.03);

   hy->GetYaxis()->SetTitleSize(0.025);
   hy->GetXaxis()->SetTitleSize(0.025);
   hy->GetYaxis()->SetTitleOffset(2);
   hy->GetXaxis()->SetTitleOffset(2);
   hy->SetFillStyle( 3001);
   hy->SetFillColor( kOrange-8);

   hy->Draw();

   //ntupleSteps->Draw("Zpos");  

   /*hz->GetYaxis()->SetLabelSize(0.03);
   hz->GetXaxis()->SetLabelSize(0.03);
   hz->GetYaxis()->SetTitleSize(0.025);
   hz->GetXaxis()->SetTitleSize(0.025);
   hz->GetYaxis()->SetTitleOffset(2);
   hz->GetXaxis()->SetTitleOffset(2);
   hz->SetFillStyle( 3001);
   hz->SetFillColor( kOrange-8);
   hz->Draw();
   */

   //gPad->SetLogy(1); 
 //ntuple->SetBranchAddress("Xpos",&Xpos);
  //int entries=ntuple->GetEntries();
    //for(int i=0;i<entries;i++){
    //ntuple->GetEntry(i);
    //  }

   //   c1->SaveAs("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_Geomfillhist.png"); 
   //c1->SaveAs("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_variablesfillhist.png");
   // c1->SaveAs("100000eventid_fillhisto.png");
   //   c1->SaveAs("200000photons_Geomfillhist.png");
}  
