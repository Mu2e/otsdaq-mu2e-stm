{
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  TH1F* hEH = new TH1F("hEH", "", 100,210000,230000);
  hEH->GetXaxis()->SetTitle("#e^{-}-hole pairs created in each event");

  // Draw histos filled by Geant4 simulation
  // Open file filled by Geant4 simulation
  //TFile f("../../../DATA/geant4/FanoFactorValid/662keV_FanoFactor0_7_allprocess.root");
  TFile f("../../../DATA/geant4/FanoFactorValid/662keV_FanoFactor0_5_allprocess.root"); 
  //TFile f("../../../DATA/geant4/FanoFactorValid/662keV_FanoFactor0_3_allprocess.root"); 
  //TFile f("../../../DATA/geant4/FanoFactorValid/662keV_defaultFano_allprocess.root"); 
  //TFile f("../../../DATA/geant4/FanoFactorValid/662keV_FanoFactor0_1_allprocess.root");
  //TFile f("../../../DATA/geant4/FanoFactorValid/662keV_FanoFactor0_08_allprocess.root");

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 200, 10, 700, 600);
  

   TNtuple* ntupleEvents = (TNtuple*)f.Get("Events");
  ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons:NehEvent","electrons>-1","goff");

  Double_t *veh  = ntupleEvents->GetVal(3);
  std::vector<int> eh;
  eh.clear();

  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    eh.push_back(*veh);
   
    std::cout<<"Event: "<<i<<", eh in each event: "<<*veh<<std::endl;

    veh++;
    }


  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    hEH->Fill(eh.at(i)); 
  }

  hEH->GetYaxis()->SetLabelSize(0.03);
  hEH->GetXaxis()->SetLabelSize(0.03);
  hEH->GetYaxis()->SetTitleSize(0.025);
  hEH->GetXaxis()->SetTitleSize(0.025);
  hEH->GetYaxis()->SetTitleOffset(2);
  hEH->GetXaxis()->SetTitleOffset(2);
  hEH->SetFillStyle( 3001);
  hEH->SetFillColor(kBlue+2);
  hEH->SetStats(0);                                                                                                               
  hEH->Draw();  

  TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",217000,221000); 
  Fit1->SetParameters(1,219157,509);
  hEH->Fit(Fit1,"0","",217000,221000);
  Fit1->SetLineColor(kBlue);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same"); 
  
  /*TH1D* hist1 = (TH1D*)f.Get("NehEvent");
  //TH1D* hist1 = (TH1D*)f.Get("Edep-Event");  
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);
  //hist1->GetXaxis()->SetRangeUser(0,1000);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);
  hist1->SetFillStyle( 3001);
  hist1->SetFillColor(kBlue+2);
  hist1->SetStats(0);
  hist1->Draw("HIST");
 
  //Fit
  TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])", 9500, 9900);
  Fit1->SetParameters(1,9700,509);

  hist1->Fit(Fit1,"0","",9500, 9900);
  Fit1->SetLineColor(kBlue);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same");
  
  */

}
