{
  gROOT->Reset();
  //gROOT->SetStyle("Plain");
  gROOT->SetStyle("ATLAS");
  //TH1F* hEH = new TH1F("hEH", "", 640,0,32000);//un bin son 50 keV 
  TH1F* hEH = new TH1F("hEH", "", 33650,0,3365000);//un bin son 100 keV


  hEH->GetXaxis()->SetTitle("N_{e-h}^{reco}");

  TH1F*hedepevent = new TH1F("hedepevent", "Edep by electrons in each event", 100, 288.89, 288.9);

  // Draw histos filled by Geant4 simulation
  // Open file filled by Geant4 simulation
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/40keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/100keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/300keV_Fano_01_EHpairs.root");  
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/500keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/662keV_Fano_01_EHpairs.root");  
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/800keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/1000keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/2000keV_Fano_01_EHpairs.root");
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/4000keV_Fano_01_EHpairs.root"); 
  //TFile f("../../../DATA/geant4/Resolution_Efficiency/8000keV_Fano_01_EHpairs.root");
  TFile f("../../../DATA/geant4/Resolution_Efficiency/10000keV_Fano_01_EHpairs.root"); 

  
  //TCanvas* c1 = new TCanvas("c1", "", 200, 10, 700, 600);
  

   TNtuple* ntupleEvents = (TNtuple*)f.Get("Events");
  ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons:NehEvent","electrons>-1","goff");

  Double_t *vedepevent  = ntupleEvents->GetVal(0);
  Double_t *veh  = ntupleEvents->GetVal(3);

  std::vector<int> eh;
  std::vector<double> EdepEvent;
  eh.clear();
  EdepEvent.clear();

  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    eh.push_back(*veh);
    EdepEvent.push_back(*vedepevent);

    //std::cout<<"eh in each event: "<<*veh<<std::endl;

    veh++;
    vedepevent++;
    }


  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    hEH->Fill(eh.at(i));
    hedepevent->Fill(EdepEvent.at(i));
  }

  //hEH->GetYaxis()->SetLabelSize(0.03);
  //hEH->GetXaxis()->SetLabelSize(0.03);
  //hEH->GetYaxis()->SetTitleSize(0.025);
  //hEH->GetXaxis()->SetTitleSize(0.025);
  //hEH->GetYaxis()->SetTitleOffset(2);
  //hEH->GetXaxis()->SetTitleOffset(2);
  //hEH->SetFillStyle( 3001);
  hEH->SetFillColor(kViolet+8);
  //hEH->SetStats(0);
  std::cout<<"Entries: "<<hEH->GetEntries()<<std::endl;
  //Turn off these ranges to plot the normalized histograms
  // hEH->GetXaxis()->SetRangeUser(3361000,3366000);
  //hEH->GetXaxis()->SetRangeUser(28000,32000);
  //DRAW NO NORMALIZED HISTOGRAM
  //hEH->Draw("HIST");


  //Normalize the y axis to the number of entries (50000, photons launched) 
  //Clone the histogram hEH to get the normalized one hnorm
  TH1*hnorm = (TH1*)(hEH->Clone("hnorm"));
  hnorm->Scale(1./hnorm->Integral());
  hnorm->GetXaxis()->SetRangeUser(3361000,3366000);
  //hnorm->GetXaxis()->SetRangeUser(28000,32000); 
  //DRAW NORMALIZED HISTOGRAM
  hnorm->Draw("HIST");  


  //100keV
  //FIT NO NORMALIZED HISTOGRAM hEH
  /*TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",29700,30200); 
  Fit1->SetParameters(1,29932,56.03);
  hEH->Fit(Fit1,"0","",29700,30200);
  Fit1->SetLineColor(kBlue);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same"); 
  */
  //FIT NORMALIZED HISTOGRAM hnorm
  /*TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",29700,30200);                                           
  Fit1->SetParameters(0.1,29932,56.03);
  hnorm->Fit(Fit1,"0","",29700,30200);
  Fit1->SetLineColor(kBlue);
  Fit1->SetLineStyle(2); 
  Fit1->Draw("same");   
  */

  //10MeV
  //FIT NO NORMALIZED HISTOGRAM hEH
  /*TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",3361000,3365000);                      
  Fit1->SetParameters(1,3363240,567.34);
  hEH->Fit(Fit1,"0","",3361000,3365000);                                                                            
  Fit1->SetLineColor(kBlue);                                                                                 
  Fit1->SetLineStyle(2);                                                                                           
  Fit1->Draw("same");  
  */
  //FIT NORMALIZED HISTOGRAM hnorm
  TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",3361000,3365000);                                   
  Fit1->SetParameters(0.001,3363240,567.34);
  hnorm->Fit(Fit1,"0","",3361000,3365000);
  Fit1->SetLineColor(kBlue);                                                                                    
  Fit1->SetLineStyle(2);                                                                                     
  Fit1->Draw("same");



  //Number of counts energy deposit by electrons in the photopeak (photoelectric effect)
  double rate_photoelectric = hEH->Integral(hEH->FindFixBin(3361500), hEH->FindFixBin(3365000), "");
  std::cout<<"Number of eh pairs (counts) in the photopeak: "<<rate_photoelectric<<endl;
  

  /*hedepevent->GetYaxis()->SetLabelSize(0.03);
  hedepevent->GetXaxis()->SetLabelSize(0.03);
  hedepevent->GetYaxis()->SetTitleSize(0.025);
  hedepevent->GetXaxis()->SetTitleSize(0.025);
  hedepevent->SetFillStyle( 3001);
  hedepevent->SetFillColor( kBlue+1);
  hedepevent->Draw();

  TF1*Fit1 = new TF1("Fit1", "[0]*TMath::Gaus(x,[1],[2])",288.88, 288.9);
  Fit1->SetParameters(1,288.9,0.004);
  hedepevent->Fit(Fit1,"0","",288.88, 288.9);
  Fit1->SetLineColor(kBlue);                                                                                            
  Fit1->SetLineStyle(2);                                                                                                            
  Fit1->Draw("same");
  */


  /*  //TH1D* hist1 = (TH1D*)f.Get("NehEvent");
  TH1D* hist1 = (TH1D*)f.Get("Edep-Event");  
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
  */
  /*auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(hEh,"simulated data","p");
  legend->SetBorderSize(0);
  legend->Draw("same");
  */
}
