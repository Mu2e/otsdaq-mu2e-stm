TCanvas *statsEditing() {

  // Create and plot a test histogram with stats
  TCanvas *se = new TCanvas;
  TH1F *h = new TH1F("h","test",100,0,6000);
  TF1 *mygaus = new TF1("mygaus","TMath::Gaus(x,3254,.5)",0,600);

  h->FillRandom("mygaus",3000);
  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat();
  gStyle->SetOptStat(1110);
  //Three significant digits
  gStyle->SetStatFormat("6.4g");
  h->Draw();
  se->Update();

  // Retrieve the stat box
  TPaveStats *ps = (TPaveStats*)se->GetPrimitive("stats");
  ps->SetX1NDC(0.67); //new x start position
  ps->SetX2NDC(0.87); //new x end position
  ps->SetY1NDC(0.66); //new y start position
  ps->SetY2NDC(0.86); //new y end position
  ps->SetTextSize(0.04);
  ps->SetName("mystats");
  TList *listOfLines = ps->GetListOfLines();

  // Remove the RMS line
  //TText *tconst = ps->GetLineWith("RMS");
  TText *entries = ps->GetLineWith("Entries");
  listOfLines->Remove(entries);

  TText *tconst = ps->GetLineWith("h");
  listOfLines->Remove(tconst);



  // Add a new line in the stat box.
  // Note that "=" is a control character
  TLatex *myt = new TLatex(0,0,"Entries = 100");
  myt ->SetTextFont(42);
  myt ->SetTextSize(0.02);
  myt ->SetTextColor(kRed);
  listOfLines->Add(myt);

  // the following line is needed to avoid that the automatic redrawing of stats
  h->SetStats(0);

  se->Modified();
  return se;
}


void Statisticsboxroot(){

  TCanvas *canvas = statsEditing();


}
