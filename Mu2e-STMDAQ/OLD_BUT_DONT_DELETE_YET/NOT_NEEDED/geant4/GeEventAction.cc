#include "GeEventAction.hh"
#include "GeRunAction.hh"


#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4UnitsTable.hh"

#include "Randomize.hh"
#include <iomanip>

#ifdef mu2edaq2
#include "G4RootAnalysisManager.hh"
#endif
#ifdef mu2edaq1
#include "g4root.hh"
#endif


//#define debug

GeEventAction::GeEventAction()
  : G4UserEventAction(),
    fEnergyAbs(0.),
    fTrackLAbs(0.),
    fTrackgamma(0.),
    num(0.),
    fEHpairs(0.),
    ncompt(0)
{}



GeEventAction::~GeEventAction()
{}


void GeEventAction::BeginOfEventAction(const G4Event* /*event*/)
{  
  // initialisation per event
  fEnergyAbs = 0.;
  fTrackLAbs = 0.;
  fTrackgamma = 0.;
  num = 0.;
  fEHpairs = 0.;
  ncompt= 0.;


  //Show the current seed for each event
  #ifdef debug
  //G4Random::showEngineStatus();
  #endif
}


void GeEventAction::EndOfEventAction(const G4Event* event)
{
  // Accumulate statistics
  //

  // get analysis manager
#ifdef mu2edaq1  
  auto analysisManager = G4AnalysisManager::Instance();
#endif
#ifdef mu2edaq2  
  auto analysisManager = G4RootAnalysisManager::Instance();
#endif
  // fill histograms EVENT
  
   analysisManager->FillH1(0, fEnergyAbs);
   analysisManager->FillH1(1, fTrackLAbs);
   analysisManager->FillH1(7, num);
   analysisManager->FillH1(10,fEHpairs);

  // fill ntuple EVENT
  analysisManager->FillNtupleDColumn(0,0,fEnergyAbs);
  analysisManager->FillNtupleDColumn(0,1,fTrackLAbs);
  analysisManager->FillNtupleIColumn(0,2,num); 
  analysisManager->FillNtupleIColumn(0,3,fEHpairs);
  
  //Primary particle info: direccion del momento inicial, es decir es el vector unitario de la direccion, su modulo es 1, this is in MeV
  G4double Egen= event->GetPrimaryVertex()->GetPrimary()->GetTotalEnergy();
  G4ThreeVector momentumgen= event->GetPrimaryVertex()->GetPrimary()->GetMomentumDirection();
  G4ThreeVector mom= event->GetPrimaryVertex()->GetPrimary()->GetMomentum();
  G4double pxgen= mom.getX();
  G4double pygen= mom.getY();
  G4double pzgen= mom.getZ();


  G4ThreeVector positiongen= event->GetPrimaryVertex()->GetPosition();
  G4double xgen= positiongen.getX()/CLHEP::cm;
  G4double ygen= positiongen.getY()/CLHEP::cm;
  G4double zgen= positiongen.getZ()/CLHEP::cm;

  #ifdef debug
  std::cout<<"Egen: "<<Egen<<" pxgen: "<<pxgen<<" pygen: "<<pygen<<" pzgen: "<<pzgen<<" MeV"<<std::endl;  
  #endif
  
  analysisManager->FillNtupleDColumn(0,4,pxgen);
  analysisManager->FillNtupleDColumn(0,5,pygen);
  analysisManager->FillNtupleDColumn(0,6,pzgen);
  analysisManager->FillNtupleDColumn(0,7,Egen);
  analysisManager->FillNtupleDColumn(0,8,fTrackgamma);
  analysisManager->FillNtupleDColumn(0,9,xgen);
  analysisManager->FillNtupleDColumn(0,10,ygen);
  analysisManager->FillNtupleDColumn(0,11,zgen);

  /*
  std::cout<<"Number of comptons generated in the event: "<<num<<std::endl;
  std::cout<<"Number of e-h pairs generated in the event: "<<fEHpairs<<std::endl;
  std::cout<<"Cross-check momentum gen: "<<pxgen<<" "<<pygen<<" "<<pzgen<<" Egen: "<<Egen<<" MeV"<<std::endl;
  */
  analysisManager->AddNtupleRow(0);
  





  
  // Print per event (modulo n)
  //
  auto eventID = event->GetEventID();
  auto printModulo = G4RunManager::GetRunManager()->GetPrintProgress();

#ifdef debug
  //if ( ( printModulo > 0 ) && ( eventID % printModulo == 0 ) ) {
  /*G4cout << "---> End of event: " << eventID << G4endl;     
    G4cout
      << "   Total energy deposit: " << std::setw(7)
      << G4BestUnit(fEnergyAbs,"Energy")
      << "       total track length: " << std::setw(7)
      << G4BestUnit(fTrackLAbs,"Length")
      << G4endl;*/
  G4cout << "---> End of event: " << eventID  << "   Total energy deposit: " << std::setw(7) << fEnergyAbs<<" keV "  << "       Total track length by e+,e-: " << std::setw(7) << fTrackLAbs<<" cm       Total track length by phot: "<<fTrackgamma<<" cm" << G4endl;
 
  //}    
  #endif
}  
