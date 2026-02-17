{
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  // Draw histos filled by Geant4 simulation                                                                                                                      
  //                                                                                                                                                              

  // Open file filled by Geant4 simulation                                                                                                                        
  // TFile f("../../../DATA/geant4/RateNISTvsCounts/40keV.root");
  // TFile f("../../../DATA/geant4/RateNISTvsCounts/100keV.root");
  // TFile f("../../../DATA/geant4/RateNISTvsCounts/300keV.root");
  // TFile f("../../../DATA/geant4/RateNISTvsCounts/400keV.root");
  // TFile f("../../../DATA/geant4/RateNISTvsCounts/800keV.root");
  TFile f("../../../DATA/geant4/RateNISTvsCounts/1000keV.root");
  // Create a canvas and divide it into 2x2 pads                                                                                                                  
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);

 
  c1->cd(1);
  TH1D* hist1 = (TH1D*)f.Get("Edep-Step");
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);

  hist1->SetFillStyle( 3001);
  hist1->SetFillColor( kBlue+1);
  hist1->Draw("HIST");

}
