
{
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
  TFile f("100000photons_100keV_000_x-10_10cm_y-10_10cm_z5_15cm_0010cm_Target.root");
  //TFile f("B4_(0,0,34m).root"); 

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);
  
  // Draw Edeposit in each event  histogram in the pad 1
  c1->cd(1);
  TH1D* hist1 = (TH1D*)f.Get("Edep-Event");
  //  hist1->SetLineColor(kRed);
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);

  hist1->SetFillStyle( 3001);                                                                                                                     
  hist1->SetFillColor( kBlue+1);    
  hist1->Draw("HIST");
  

  // Draw number of electrons in each event histogram in the pad 2
  c1->cd(2);
  TH1D* hist2 = (TH1D*)f.Get("electrons");
  //   TH1D* hist2 = (TH1D*)f.Get("EventID");
  hist2->GetYaxis()->SetLabelSize(0.03);
  hist2->GetXaxis()->SetLabelSize(0.03);

  hist2->GetYaxis()->SetTitleSize(0.025);
  hist2->GetXaxis()->SetTitleSize(0.025);
  hist2->GetYaxis()->SetTitleOffset(2);
  hist2->GetXaxis()->SetTitleOffset(2);

  hist2->SetFillStyle( 3001);                                                                                                                     
  hist2->SetFillColor( kOrange+1);       
  hist2->Draw("HIST");
      
  // Draw E deposit in each step  histogram in the pad 3
  // with logaritmic scale for y
  c1->cd(3);
  //  TH1D* hist3 = (TH1D*)f.Get("Edep-Step");
  TH2D* hist3 = (TH2D*)f.Get("ZXpos");   
  hist3->GetYaxis()->SetLabelSize(0.03);
  hist3->GetXaxis()->SetLabelSize(0.03);

  hist3->GetYaxis()->SetTitleSize(0.025);
  hist3->GetXaxis()->SetTitleSize(0.025);
  hist3->GetYaxis()->SetTitleOffset(2);
  hist3->GetXaxis()->SetTitleOffset(2);
  //gPad->SetLogy(1);                                                                                                                                 
  hist3->SetFillStyle( 3001);
  hist3->SetFillColor( kBlue+4);
  //gPad->SetLogy(1);
  hist3->Draw("HIST");
  
  // Draw Zposition of each step of the hits  histogram in the pad 4
  // with logaritmic scale for y
  c1->cd(4);
  //gPad->SetLogy(1);
  //TH1D* hist4 = (TH1D*)f.Get("Zpos");
  TH2D* hist4 = (TH2D*)f.Get("ZYpos");
  hist4->GetYaxis()->SetLabelSize(0.03);
  hist4->GetXaxis()->SetLabelSize(0.03);

  hist4->GetYaxis()->SetTitleSize(0.025);
  hist4->GetXaxis()->SetTitleSize(0.025);
  hist4->GetYaxis()->SetTitleOffset(2);
  hist4->GetXaxis()->SetTitleOffset(2);
  hist4->SetFillStyle( 3001);
  hist4->SetFillColor( kOrange-8);
  hist4->Draw("HIST");

  // c1->SaveAs("100000_100keV_histo.png");
}
