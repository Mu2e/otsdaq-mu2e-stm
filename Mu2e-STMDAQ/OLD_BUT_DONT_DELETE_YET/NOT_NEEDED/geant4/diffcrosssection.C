
{
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
  TFile f("diffcrosssectE_1MeVphotons.root");

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 200, 10, 700, 600);


  TH1D* hist1 = (TH1D*)f.Get("diffcrosssectE");   
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);
  hist1->SetFillStyle( 3001);
  hist1->SetFillColor(kOrange+2);
  hist1->Draw("HIST");
}
