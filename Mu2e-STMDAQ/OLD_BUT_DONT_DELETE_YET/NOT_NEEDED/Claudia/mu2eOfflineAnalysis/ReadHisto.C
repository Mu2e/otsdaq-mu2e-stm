

void ReadHisto(){

  //TFile *input=new TFile("PhotonsElectronMuonBeamVD101_MDC2020.root");
  TFile *input=new TFile("eleElectronMuonBeamVD101_MDC2020.root");
  TH1D* h=(TH1D*)input->Get("h1");
  
  double integral = h->Integral();

  double entries = h->GetEntries();
  int Nbins = h->GetNbinsX();;
  double sum=0.0;
  
  for(int i=0; i<Nbins; i++){
    double bincenter = h->GetBinCenter(i);

    if(bincenter>0.1){
      double bincontent = h->GetBinContent(i);
      std::cout<<"bincontent: "<<bincontent<<" x bincenter "<<bincenter<<std::endl;
      sum=sum+bincontent*bincenter;
    }
  }

  std::cout<<"sum: "<<sum<<std::endl;
  h->Draw();
}
