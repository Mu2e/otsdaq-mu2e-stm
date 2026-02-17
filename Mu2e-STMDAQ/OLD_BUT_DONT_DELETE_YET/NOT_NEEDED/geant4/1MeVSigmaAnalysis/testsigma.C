#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

void testsigma(){
  gROOT->Reset();
  gROOT->SetStyle("Plain");

  // Draw histos filled by Geant4 simulation
  //

  // Create a canvas and divide it into 2x2 pads
  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 900, 500);
  //TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  //c1->Divide(2,1);


  TH1F*hedepevent = new TH1F("hedepevent", "Edep by electrons in each event", 100, 985., 990);
  
  TH1F* htracklengthevent = new TH1F("htracklengthevent", "Total Track Length by electrons in each event", 100, 0., 10.);
  htracklengthevent->GetXaxis()->SetTitle("Total Track Length (cm)");
  TH1F* helectrons = new TH1F("helectrons", "#Electrons in each event", 20, 0., 20.);
  helectrons->GetXaxis()->SetTitle("#Electrons");



  //Histogram to store the sigmas of each file
  TH1F*hsigma = new TH1F("hsigma","", 50, 0, 0.005); //0.0001 kev binning 
  hsigma->GetXaxis()->SetTitle("#sigma_{E} (keV)");
  hsigma->SetTitle("1 MeV photons");

  //Histogram to store the meanenergy of each file
  TH1F*hmeanE = new TH1F("hmeanE","", 50, 985, 990); // 0.1 kev binning 
  hmeanE->GetXaxis()->SetTitle("Mean E_{Reco} (keV)");
  hmeanE->SetTitle("1 MeV photons");


  //Abrimos el txt
  fstream readfile;
  readfile.open("1MeVruns.txt",ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();
  //Lee cada fila del .txt que es cada uno de los nombres de los .csv
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    //cout<<name<<endl;
  }


  double meanenergy, meanenergyerror;
  double sigma, sigmaerror;
  std::vector<double> EdepEvent, TrackLengthEvent, electrons;








  for (int file=0;file<(file_name.size()-1);file++){
    hedepevent->Reset();
    htracklengthevent->Reset();
    helectrons->Reset();

    meanenergy=0.;
    meanenergyerror=0.;
    sigma=0.;
    sigmaerror=0.;



    string path;
    path=file_name[file];
    //cout<<file_name.size()<<endl;
    cout<<path.c_str()<<endl;

   
    TFile f(path.c_str());

  // Get ntuple
  TNtuple* ntupleSteps = (TNtuple*)f.Get("Steps");
  TNtuple* ntupleEvents = (TNtuple*)f.Get("Events");
  //Define the Ntuples
  ntupleSteps->Draw("Xpos:Ypos:Zpos:EventID:EdepStep:StepLengthStep:PDGID","EventID>-1","goff");
  ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons","electrons>-1","goff");



    //Long64_t numberofpoints= ntuple->GetEstimate();
    //Double_t rows=ntuple-> GetSelectedRows();

  //Number of total entries in all the events
  std::cout<<ntupleSteps->GetSelectedRows()<<std::endl;
  std::cout<<ntupleEvents->GetSelectedRows()<<std::endl;


  Double_t *vedepevent  = ntupleEvents->GetVal(0);
  Double_t *vtracklengthevent  = ntupleEvents->GetVal(1);
  Double_t *velectrons  = ntupleEvents->GetVal(2);



  EdepEvent.clear();
  TrackLengthEvent.clear();
  electrons.clear();



  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){

    EdepEvent.push_back(*vedepevent);
    TrackLengthEvent.push_back(*vtracklengthevent);
    electrons.push_back(*velectrons);
    
    // std::cout<<"Event: "<<i<<" Energy deposit in each event: "<<*vedepevent<<std::endl;
    vedepevent++;
    vtracklengthevent++;
    velectrons++;
  }

  //Filling the histograms

  for(int i=0;i<ntupleEvents->GetSelectedRows();i++){
    hedepevent->Fill(EdepEvent.at(i));
    htracklengthevent->Fill(TrackLengthEvent.at(i));
    helectrons->Fill(electrons.at(i));
    //  std::cout<<"Energy deposit in each event: "<<EdepEvent.at(i)<<std::endl;
  }

  //Energy deposit in each event by electrons stored in geant4
  //c1->cd(1);
   TCanvas *canvas0 = new TCanvas("canvas0", "",200,200,500,400);
  hedepevent->GetXaxis()->SetTitle("Total Edep e- (keV)");
  hedepevent->GetYaxis()->SetLabelSize(0.03);
  hedepevent->GetXaxis()->SetLabelSize(0.03);
  hedepevent->GetYaxis()->SetTitleSize(0.025);
  hedepevent->GetXaxis()->SetTitleSize(0.025);
  hedepevent->SetFillStyle( 3001);
  hedepevent->SetFillColor( kBlue+1); 
  hedepevent->Draw();
  canvas0->Update();
  canvas0->Modified();
  canvas0->WaitPrimitive();
  

  meanenergy=hedepevent->GetMean();
  sigma=hedepevent->GetRMS();

  meanenergyerror=hedepevent->GetMeanError();
  sigmaerror=hedepevent->GetRMSError();

  hmeanE->Fill(meanenergy);
  hsigma->Fill(sigma);

  std::cout<<"La Energia media del pico: "<<meanenergy<<" +/- "<<meanenergyerror<<std::endl;
  std::cout<<"La resolucion: "<<sigma<<" +/- "<<sigmaerror<<std::endl;


  //Number of counts energy deposit by electrons in the photopeak (photoelectric effect)
  double rate_photoelectric = hedepevent->Integral(hedepevent->FindFixBin(985), hedepevent->FindFixBin(990), "");
  std::cout<<"Number of counts in the photopeak with Edeposit by e-: "<<rate_photoelectric<<endl;
  //Number of counts energy deposit by electrons (Compton effect)
  double rate_compton = hedepevent->Integral(hedepevent->FindFixBin(0), hedepevent->FindFixBin(985), "");
  std::cout<<"Number of counts in the compton background with Edeposit by e-: "<<rate_compton<<endl;


  }//for int file

  /*  hsigma->SetFillStyle(3001);
  hsigma->SetFillColor(kAzure+2); 
  hsigma->Draw();
  */

  hmeanE->SetFillStyle(3001);
  hmeanE->SetFillColor(kGreen-3);
  hmeanE->Draw();
}
