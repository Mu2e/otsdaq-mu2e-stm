using namespace RooFit;

//Importing from histogram
void RooFitHisto() {

  TFile *fin = new TFile("/data1/cgarcia/MinuitFit_1kHz_100000s_347keV_1keVres/1000JOBS/BinnedLoglike_NOIntegral_1kHz_TimeSim_100000s_seed_0_50Runs_Job_0.root");
  TH1D* hist = (TH1D *)fin->Get("hSTM1");

  RooRealVar mass("mass","mass",0.,2.);
  RooDataHist bindata("bindata","bindata",RooArgList(mass),hist);
  bindata.Print("v");

  //Fit data
  RooRealVar mu("mu","mu",0.347,0.3,0.4);
  RooRealVar sigma("sigma","sigma",0.001,0.0005,0.0015);
  RooGaussian gaus("gaus","gaus",mass,mu,sigma);

  RooRealVar slope("slope","slope",-0.3,0.1,0.5);
  RooPolynomial linear("linear","linear",mass,RooArgSet(slope));

  RooRealVar frac("frac","frac",0.2,0.,1.);
  RooAddPdf model("model","model",RooArgList(gaus,linear),RooArgList(frac));

  //Plot data and model before fitting
  RooPlot* frame_mass = mass.frame();
  bindata.plotOn(frame_mass);
  model.plotOn(frame_mass, LineColor(kRed));

  model.fitTo(bindata,Minos(true));

  //Plot fitting
  model.plotOn(frame_mass, LineColor(kBlue));
  frame_mass->Draw("");

}
