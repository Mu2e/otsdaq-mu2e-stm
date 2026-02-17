#include "GeRunAction.hh"
#include "G4Run.hh"
//Added
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
//#include "g4root.hh" --> mu2edaq1

// mu2edaq2
#include "G4RootAnalysisManager.hh" 

//#define debug

GeRunAction::GeRunAction()
{
  // set printing event number per each event
  #ifdef debug
  G4RunManager::GetRunManager()->SetPrintProgress(1);  
  #endif
  
  // Create analysis manager
  //  auto analysisManager = G4AnalysisManager::Instance(); // mu2edaq1
  auto analysisManager = G4RootAnalysisManager::Instance(); // mu2edaq2
  G4cout << "Using " << analysisManager->GetType() << G4endl;
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetNtupleMerging(true);

  // Creating histograms

  // fill histograms H1 EVENT 
  analysisManager->CreateH1("Edep-Event","Total Edep by electrons generated in each event", 100, 0.,700);
  analysisManager->SetH1XAxisTitle(0,"Total Edep e- (keV)");
  analysisManager->SetH1YAxisTitle(0,"");
  analysisManager->CreateH1("TrackLength-Event","Total Track Length by e- in each event", 100, 0., 10);
  analysisManager->SetH1XAxisTitle(1,"Total Track Length (cm)");
  analysisManager->SetH1YAxisTitle(1,""); 
 // fill histograms H1 STEP
  analysisManager->CreateH1("Edep-Step","Edep by electrons+photons in each step", 100, 0., 400);
  analysisManager->SetH1XAxisTitle(2,"Edep Step e- (keV)");
  analysisManager->SetH1YAxisTitle(2,"");
  analysisManager->CreateH1("Xpos","x position of the hit in each step", 100, -20., 20);
  analysisManager->SetH1XAxisTitle(3,"x (cm)");
  analysisManager->SetH1YAxisTitle(3,"");
  analysisManager->CreateH1("Ypos","y position of the hit in each step", 100, -20., 20);
  analysisManager->SetH1XAxisTitle(4,"y (cm)");
  analysisManager->SetH1YAxisTitle(4,"");
  analysisManager->CreateH1("Zpos","z position of the hit in each step", 100, 0., 20);
  analysisManager->SetH1XAxisTitle(5,"z (cm)");
  analysisManager->SetH1YAxisTitle(5,"");  
  analysisManager->CreateH1("StepLength-Step","Step Length by electrons+photons  in each step", 100, 0., 1.5);
  analysisManager->SetH1XAxisTitle(6,"Step Length (cm)");
  analysisManager->SetH1YAxisTitle(6,"");
  // fill histograms H1 EVENT 
  analysisManager->CreateH1("num","#Electrons in each event", 20, 0., 20.);
  analysisManager->SetH1XAxisTitle(7,"#Electrons");
  analysisManager->SetH1YAxisTitle(7,"");
  // fill histograms H1 STEP
  analysisManager->CreateH1("EventID","EventID in each event", 100, 0., 100000.);
  analysisManager->SetH1XAxisTitle(8,"EventID each step");
  analysisManager->SetH1YAxisTitle(8,"");
  analysisManager->CreateH1("diffcrosssectE","Difference between Cross Sections in each step", 100,-10,10);
  analysisManager->SetH1XAxisTitle(9,"#sigma_{input Geant4}-(1/(#rho_{Ge}*steplength)) (g/cm^{2})");
  analysisManager->CreateH1("NehEvent","", 100,0,300000);
  analysisManager->SetH1XAxisTitle(10,"#e^{-}-hole pairs created in each event by eIoni+eBrem+msc"); 
  // fill histograms H2  STEP
  analysisManager->CreateH2("ZXpos","zx position of the hit in each step", 1000, 0.,20,1000,-20., 20);
  analysisManager->SetH2XAxisTitle(0,"z (cm)");
  analysisManager->SetH2YAxisTitle(0,"x (cm)");
  analysisManager->CreateH2("ZYpos","zy position of the hit in each step", 1000, 0.,20,1000,-20., 20);
  analysisManager->SetH2XAxisTitle(1,"z (cm)");
  analysisManager->SetH2YAxisTitle(1,"y (cm)");
  //  analysisManager->CreateH2("diffcrosssectE","1 MeV photons: Difference between Cross Sections (Intput Geant4-Calulated) in each step", 1000, 0.,0.1,1000,-0.1, 0.1);
  //analysisManager->SetH2XAxisTitle(2,"E (keV)");
  //analysisManager->SetH2YAxisTitle(2,"#sigma_{input Geant4}-(1/(#rho_{Ge}*steplength)) (g/cm^{2})");
  // Creating ntuple                                                      
   analysisManager->CreateNtuple("Events", "Variables");
   analysisManager->CreateNtupleDColumn("EdepEvent");
   analysisManager->CreateNtupleDColumn("TrackLengthEvent");
   analysisManager->CreateNtupleIColumn("num");
   analysisManager->CreateNtupleIColumn("NehEvent");
   analysisManager->CreateNtupleDColumn("pxgen");
   analysisManager->CreateNtupleDColumn("pygen");
   analysisManager->CreateNtupleDColumn("pzgen");
   analysisManager->CreateNtupleDColumn("Egen");
   analysisManager->CreateNtupleDColumn("TrackLengthEventgammas");
   analysisManager->CreateNtupleDColumn("xgen");
   analysisManager->CreateNtupleDColumn("ygen");
   analysisManager->CreateNtupleDColumn("zgen");

   analysisManager->FinishNtuple();

   analysisManager->CreateNtuple("StepsGe", "Variables");
   analysisManager->CreateNtupleDColumn("EdepStep");
   analysisManager->CreateNtupleDColumn("preXpos");
   analysisManager->CreateNtupleDColumn("preYpos");
   analysisManager->CreateNtupleDColumn("preZpos");
   analysisManager->CreateNtupleDColumn("postXpos");
   analysisManager->CreateNtupleDColumn("postYpos");
   analysisManager->CreateNtupleDColumn("postZpos");
   analysisManager->CreateNtupleDColumn("StepLengthStep");
   analysisManager->CreateNtupleIColumn("PDGID");
   analysisManager->CreateNtupleIColumn("EventID"); 
   analysisManager->CreateNtupleDColumn("pre_px");
   analysisManager->CreateNtupleDColumn("pre_py");
   analysisManager->CreateNtupleDColumn("pre_pz");
   analysisManager->CreateNtupleDColumn("post_px");
   analysisManager->CreateNtupleDColumn("post_py");
   analysisManager->CreateNtupleDColumn("post_pz");
   analysisManager->CreateNtupleIColumn("particlenum");
   analysisManager->CreateNtupleDColumn("step_initialtime");
   analysisManager->CreateNtupleDColumn("step_finaltime");  
   analysisManager->FinishNtuple();

   analysisManager->CreateNtuple("StepsKCl", "Variables");
   analysisManager->CreateNtupleIColumn("EventID");
   analysisManager->CreateNtupleDColumn("EdepStep");
   analysisManager->CreateNtupleDColumn("preXpos");
   analysisManager->CreateNtupleDColumn("preYpos");
   analysisManager->CreateNtupleDColumn("preZpos");
   analysisManager->CreateNtupleDColumn("postXpos");
   analysisManager->CreateNtupleDColumn("postYpos");
   analysisManager->CreateNtupleDColumn("postZpos");
   analysisManager->CreateNtupleDColumn("StepLengthStep");
   analysisManager->CreateNtupleIColumn("PDGID");
   analysisManager->CreateNtupleDColumn("pre_px");
   analysisManager->CreateNtupleDColumn("pre_py");
   analysisManager->CreateNtupleDColumn("pre_pz");
   analysisManager->FinishNtuple();


   analysisManager->CreateNtuple("StepsVacuumVirtualPlane", "Variables");
   analysisManager->CreateNtupleIColumn("EventID");
   analysisManager->CreateNtupleDColumn("EdepStep");
   analysisManager->CreateNtupleDColumn("preXpos");
   analysisManager->CreateNtupleDColumn("preYpos");
   analysisManager->CreateNtupleDColumn("preZpos");
   analysisManager->CreateNtupleDColumn("StepLengthStep");
   analysisManager->CreateNtupleIColumn("PDGID");
   analysisManager->CreateNtupleDColumn("pre_px");
   analysisManager->CreateNtupleDColumn("pre_py");
   analysisManager->CreateNtupleDColumn("pre_pz");
   analysisManager->FinishNtuple();

   analysisManager->CreateNtuple("StepsPoly", "Variables");
   analysisManager->CreateNtupleIColumn("EventID");
   analysisManager->CreateNtupleDColumn("EdepStep");
   analysisManager->CreateNtupleDColumn("preXpos");
   analysisManager->CreateNtupleDColumn("preYpos");
   analysisManager->CreateNtupleDColumn("preZpos");
   analysisManager->CreateNtupleDColumn("postXpos");
   analysisManager->CreateNtupleDColumn("postYpos");
   analysisManager->CreateNtupleDColumn("postZpos");
   analysisManager->CreateNtupleDColumn("StepLengthStep");
   analysisManager->CreateNtupleIColumn("PDGID");
   analysisManager->CreateNtupleDColumn("pre_px");
   analysisManager->CreateNtupleDColumn("pre_py");
   analysisManager->CreateNtupleDColumn("pre_pz");
   analysisManager->FinishNtuple();

   analysisManager->CreateNtuple("StepsAlWindow", "Variables");
   analysisManager->CreateNtupleIColumn("EventID");
   analysisManager->CreateNtupleDColumn("EdepStep");
   analysisManager->CreateNtupleDColumn("preXpos");
   analysisManager->CreateNtupleDColumn("preYpos");
   analysisManager->CreateNtupleDColumn("preZpos");
   analysisManager->CreateNtupleDColumn("postXpos");
   analysisManager->CreateNtupleDColumn("postYpos");
   analysisManager->CreateNtupleDColumn("postZpos");
   analysisManager->CreateNtupleDColumn("StepLengthStep");
   analysisManager->CreateNtupleIColumn("PDGID");
   analysisManager->CreateNtupleDColumn("pre_px");
   analysisManager->CreateNtupleDColumn("pre_py");
   analysisManager->CreateNtupleDColumn("pre_pz");
   analysisManager->FinishNtuple();
}

GeRunAction::~GeRunAction()
{
  //  delete G4AnalysisManager::Instance(); 
  delete G4RootAnalysisManager::Instance(); 
}

void GeRunAction::BeginOfRunAction(const G4Run* aRun)
{
  G4cout << "GeRunAction::BeginOfRunAction: Run #" << aRun->GetRunID() << " start." << G4endl;
  // Get analysis manager

  //  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();

  //Show the current seed for each event
  //G4Random::showEngineStatus();

  int runID=aRun->GetRunID();
  std::string srun = std::to_string(runID);
  // Open an output file
  //G4String fileName = "PolyAbsorber10cm_test100000photons_diskpos_694keV_pzdistr"+srun+".root";


  std::string rootname;
  std::ifstream input_file;
  input_file.open("geant_config.txt");
  if(!input_file.is_open()) throw std::runtime_error("Could not open file");

  std::string line;
  std::string element;
  while(std::getline(input_file, line))
    {
      // Create a stringstream of the current line
      std::stringstream ss(line);

      while(ss >> element){
        if(element.compare("rootname:") == 0) ss >> rootname;
        }
    }
  input_file.close();

  G4String fileName =  rootname;
  analysisManager->OpenFile(fileName);

}

void GeRunAction::EndOfRunAction(const G4Run*)
{ 

  // print histogram statistics
  //  auto analysisManager = G4AnalysisManager::Instance();
  auto analysisManager = G4RootAnalysisManager::Instance();
  if ( analysisManager->GetH1(1) ) {
    G4cout << G4endl << " ----> print histograms statistic ";
    if(isMaster) {
      G4cout << "for the entire run " << G4endl << G4endl; 
    }
    else {
      G4cout << "for the local thread " << G4endl << G4endl; 
    }
    
    /* G4cout << " EAbs : mean = " 
	   << G4BestUnit(analysisManager->GetH1(0)->mean(), "Energy") 
	   << " rms = " 
	   << G4BestUnit(analysisManager->GetH1(0)->rms(),  "Energy") << G4endl;
    
    G4cout << " EGap : mean = " 
	   << G4BestUnit(analysisManager->GetH1(1)->mean(), "Energy") 
	   << " rms = " 
	   << G4BestUnit(analysisManager->GetH1(1)->rms(),  "Energy") << G4endl;
    
    G4cout << " LAbs : mean = " 
	   << G4BestUnit(analysisManager->GetH1(2)->mean(), "Length") 
	   << " rms = " 
	   << G4BestUnit(analysisManager->GetH1(2)->rms(),  "Length") << G4endl;

     G4cout << " LGap : mean = " 
	   << G4BestUnit(analysisManager->GetH1(3)->mean(), "Length") 
	   << " rms = " 
	   << G4BestUnit(analysisManager->GetH1(3)->rms(),  "Length") << G4endl;
    */  }

  // save histograms & ntuple
  //
  analysisManager->Write();
  analysisManager->CloseFile();
  G4cout<<"GeRunAction::EndOfRunAction: end of run"<<G4endl;

}
