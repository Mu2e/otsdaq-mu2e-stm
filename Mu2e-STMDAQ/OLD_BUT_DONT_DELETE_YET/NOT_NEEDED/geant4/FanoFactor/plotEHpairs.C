{
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  // Draw histos filled by Geant4 simulation
  // Open file filled by Geant4 simulation
  //TFile f("../../../DATA/geant4/FanoFactor/prueba662eIoni.root");
  //TFile f("../../../DATA/geant4/FanoFactor/prueba662keV_FanoFactor0_1.root"); 
  //TFile f("../../../DATA/geant4/FanoFactor/prueba662keV_FanoFactor0_08.root");
  //TFile f("../../../DATA/geant4/FanoFactor/prueba662keV_FanoFactor0_08_allprocess.root");    
  //TFile f("../../../DATA/geant4/FanoFactor/prueba662keV_FanoFactor0_1_allprocess.root");  
  TFile f("../../../DATA/geant4/FanoFactor/prueba662keV_FanoFactor0_7_allprocess.root"); 


  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 200, 10, 700, 600);
  
  TH1D* hist1 = (TH1D*)f.Get("NehEvent");
  //TH1D* hist1 = (TH1D*)f.Get("Edep-Event");  
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);
  hist1->SetFillStyle( 3001);
  hist1->SetFillColor(kBlue+2);
  hist1->SetStats(0);
  hist1->Draw("HIST");

  //Fit
  TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])", 100000, 200000);
  Fit1->SetParameters(1,1e+05,5e+04);

  hist1->Fit(Fit1,"0","",140000, 180000);
  Fit1->SetLineColor(kBlue);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same");
}
