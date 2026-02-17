
{
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
    TFile f("200000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_new.root");
  // TFile f("B4_(0,0,34m).root"); 
  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  c1->Divide(2,2);
  
  // Draw pos xy for each hit in the detector histogram in the pad 1
  c1->cd(1);
  TH2D* hist1 = (TH2D*)f.Get("ZXpos");
  hist1->GetYaxis()->SetLabelSize(0.03);
  hist1->GetXaxis()->SetLabelSize(0.03);

  hist1->GetYaxis()->SetTitleSize(0.025);
  hist1->GetXaxis()->SetTitleSize(0.025);  
  hist1->GetYaxis()->SetTitleOffset(2);
  hist1->GetXaxis()->SetTitleOffset(2);

  //  hist1->SetFillStyle( 3001);
  //hist1->SetFillColor( kBlue+1);
  hist1->Draw("HIST");
  

  // Draw number of electrons in each event histogram in the pad 2
  c1->cd(2);
  TH2D* hist2 = (TH2D*)f.Get("ZYpos");
  hist2->GetYaxis()->SetLabelSize(0.03);
  hist2->GetXaxis()->SetLabelSize(0.03);

  hist2->GetYaxis()->SetTitleSize(0.025);
  hist2->GetXaxis()->SetTitleSize(0.025);
  hist2->GetYaxis()->SetTitleOffset(2);
  hist2->GetXaxis()->SetTitleOffset(2);

  //hist2->SetFillStyle( 3001);
  //hist2->SetFillColor( kOrange+1);  
  hist2->Draw("HIST");
      
  // Draw Xpos  histogram in the pad 3
  // with logaritmic scale for y
  c1->cd(3);
  TH1D* hist3 = (TH1D*)f.Get("Xpos");
  hist3->GetYaxis()->SetLabelSize(0.03);
  hist3->GetXaxis()->SetLabelSize(0.03);

  hist3->GetYaxis()->SetTitleSize(0.025);
  hist3->GetXaxis()->SetTitleSize(0.025);
  hist3->GetYaxis()->SetTitleOffset(2);
  hist3->GetXaxis()->SetTitleOffset(2);
  //gPad->SetLogy(1);
  hist3->SetFillStyle( 3001);
  hist3->SetFillColor( kOrange-8);
  hist3->Draw("HIST");
  
  // Draw Ypos of each step of the hits  histogram in the pad 4
  // with logaritmic scale for y
   c1->cd(4);
  //gPad->SetLogy(1);
  TH1D* hist4 = (TH1D*)f.Get("Ypos");
  hist4->GetYaxis()->SetLabelSize(0.03);
  hist4->GetXaxis()->SetLabelSize(0.03);

  hist4->GetYaxis()->SetTitleSize(0.025);
  hist4->GetXaxis()->SetTitleSize(0.025);
  hist4->GetYaxis()->SetTitleOffset(2);
  hist4->GetXaxis()->SetTitleOffset(2);
  hist4->SetFillStyle( 3001);
  hist4->SetFillColor( kOrange-8);
  hist4->Draw("HIST");


  gStyle->SetTitleFontSize(.04);
  //c1->SaveAs("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_Geomhist.png");
  c1->SaveAs("200000photons_Geomhist.png"); 
}
