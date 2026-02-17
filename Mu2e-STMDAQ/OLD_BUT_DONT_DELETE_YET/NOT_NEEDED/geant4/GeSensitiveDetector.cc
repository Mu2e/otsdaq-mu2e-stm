#include "GeSensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

#include "G4RunManager.hh"
//#include "GeRunAction.hh"

//#define debug

GeSensitiveDetector::GeSensitiveDetector(G4String name) :
  G4VSensitiveDetector(name)
{
}

GeSensitiveDetector::~GeSensitiveDetector(){ }

void GeSensitiveDetector::Initialize(G4HCofThisEvent* HCE)
{
}

G4bool GeSensitiveDetector::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
  G4StepPoint* pointPre=aStep->GetPreStepPoint();
  G4StepPoint* pointPost=aStep->GetPostStepPoint(); 
  G4TouchableHandle touchCur=pointPre->GetTouchableHandle();
  G4VPhysicalVolume* volumeCur=touchCur->GetVolume();
  G4String volumeCurName=volumeCur->GetName();
  G4int copyNumofChamber=touchCur->GetCopyNumber();
  G4Track* trackCur=aStep->GetTrack();
  G4ParticleDefinition* particleCur = trackCur->GetDefinition();
  G4String particleCurName = particleCur->GetParticleName();
  G4double kineticEnergyCur = trackCur->GetKineticEnergy();
  G4RunManager* runManager= G4RunManager::GetRunManager();
  //GeRunAction* runActionCur=(GeRunAction *)runManager->GetUserRunAction();
  #ifdef debug
  std::cout<<" "<<std::endl;
  std::cout<<"--------------------Process----------------------"<<std::endl;
  std::cout<<"GeSensitiveDetector.cc "<<std::endl;
  G4cout << "Detector hit:" << pointPre->GetPosition()/CLHEP::cm << " cm  --> " << pointPost->GetPosition()/CLHEP::cm << 
    " cm by " << particleCurName << 
    " with Epost_step=" << kineticEnergyCur/CLHEP::keV << "keV inside " << volumeCurName << G4endl;
    

  G4cout << "Particle created:  "<<particleCur->GetParticleName()<<" PDGID("<<particleCur->GetPDGEncoding()<<") "<<trackCur->GetTrackID() << " Pos start: "<<pointPre->GetPosition()/CLHEP::cm<<"cm  Pos end: "<<pointPost->GetPosition()/CLHEP::cm<<"cm Step Length: "<<aStep->GetStepLength()/CLHEP::cm<<" cm"<<" Energy Deposit: "<<(aStep->GetTotalEnergyDeposit())/CLHEP::keV<<" keV"<<" Mass: " <<particleCur->GetPDGMass()  <<G4endl;
  G4cout<<"PreStep-----Ekin= "<<(pointPre->GetKineticEnergy())/CLHEP::keV<<" keV"<<" with Etot= " <<(pointPre->GetTotalEnergy())/CLHEP::keV<<" keV" << G4endl;
  G4cout<<"PostStep-----Ekin= "<<(pointPost->GetKineticEnergy())/CLHEP::keV<<" keV"<<" with Etot= " <<(pointPost->GetTotalEnergy())/CLHEP::keV<<" keV" << G4endl;
  #endif
  
  return true;
}

void GeSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
  #ifdef debug
  G4cout << "Event end!" << G4endl;
  #endif
}
