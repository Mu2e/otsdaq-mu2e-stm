#include "GeSteppingAction.hh"
#include "GeEventAction.hh"
#include "GeUserDetectorConstruction.hh"

#include "G4Step.hh"
#include "G4RunManager.hh"

//#include "g4root.hh" --> mu2edaq1
// mu2edaq2
#include "G4RootAnalysisManager.hh" 

//add
#include "G4UnitsTable.hh"
#include "G4EmCalculator.hh"
#include "G4NistManager.hh"
#include "G4LossTableManager.hh"
#include "G4ElectronIonPair.hh"
#include "G4NIELCalculator.hh"

//#define debug

GeSteppingAction::GeSteppingAction(const GeUserDetectorConstruction* detectorConstruction,
				     GeEventAction* eventAction)
  : G4UserSteppingAction(),
    fDetConstruction(detectorConstruction),
    fEventAction(eventAction)
{}


GeSteppingAction::~GeSteppingAction()
{}


void GeSteppingAction::UserSteppingAction(const G4Step* step)
{
  // Collect energy and track length step by step
  #ifdef debug
  std::cout<<"---GeSteppingAction.cc-Analyse"<<std::endl;
  #endif
  auto PreSteppos=step->GetPreStepPoint()->GetPosition();
  auto PostSteppos=step->GetPostStepPoint()->GetPosition();
  #ifdef debug
  std::cout<<"PreSteppos: "<<PreSteppos/CLHEP::cm<<"cm  PostSteppos: "<<PostSteppos/CLHEP::cm<<" cm"<<std::endl;
  #endif
  // get volume of the current step
  auto volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();

#ifdef debug
  std::cout<<"Volume: "<<volume->GetName()<<std::endl;
  #endif
  
  auto pre_px=step->GetPreStepPoint()->GetMomentum().x();
  auto pre_py=step->GetPreStepPoint()->GetMomentum().y();
  auto pre_pz=step->GetPreStepPoint()->GetMomentum().z();

  auto post_px=step->GetPostStepPoint()->GetMomentum().x();
  auto post_py=step->GetPostStepPoint()->GetMomentum().y();
  auto post_pz=step->GetPostStepPoint()->GetMomentum().z();
  
#ifdef debug
  std::cout<<"Pre Momentum vector: ("<<pre_px/CLHEP::keV<<","<<pre_py/CLHEP::keV<<","<<pre_pz/CLHEP::keV<<") keV"<<std::endl;
  std::cout<<"Post Momentum vector: ("<<post_px/CLHEP::keV<<","<<post_py/CLHEP::keV<<","<<post_pz/CLHEP::keV<<") keV"<<std::endl;
#endif
  //Particle Step
  G4Track* track=step->GetTrack();
  G4ParticleDefinition* particle = track->GetDefinition();

  //  auto analysisManager = G4AnalysisManager::Instance();
  auto analysisManager = G4RootAnalysisManager::Instance();
   





  //If we are inside sensitive Ge volume fill histograms
   if(volume->GetName()=="Target"){

   //Event ID
   auto eventid = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
   #ifdef debug
   std::cout<<"EventID: "<<eventid<<std::endl;
   #endif
   //Name of the process
   std::string procName=step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
   #ifdef debug
   std::cout<<"Process: "<<procName<<std::endl;
   #endif
   //position
   auto prex=PreSteppos.getX()/CLHEP::cm;
   auto prey=PreSteppos.getY()/CLHEP::cm;
   auto prez=PreSteppos.getZ()/CLHEP::cm;
   auto postx=PostSteppos.getX()/CLHEP::cm;
   auto posty=PostSteppos.getY()/CLHEP::cm;
   auto postz=PostSteppos.getZ()/CLHEP::cm;
   #ifdef debug
   std::cout<<"Pre Position: ("<<prex<<","<<prey<<","<<prez<<") cm"<<std::endl;
   std::cout<<"Post Position: ("<<postx<<","<<posty<<","<<postz<<") cm"<<std::endl;
   #endif
   //Momentum direction
   auto predir_px=step->GetPreStepPoint()->GetMomentumDirection().x();
   auto predir_py=step->GetPreStepPoint()->GetMomentumDirection().y();
   auto predir_pz=step->GetPreStepPoint()->GetMomentumDirection().z();
   #ifdef debug
   std::cout<<"Pre Momentum direction: ("<<predir_px<<","<<predir_py<<","<<predir_pz<<"), normalised momentum vector"<<std::endl;
   #endif
   auto postdir_px=step->GetPostStepPoint()->GetMomentumDirection().x();
   auto postdir_py=step->GetPostStepPoint()->GetMomentumDirection().y();
   auto postdir_pz=step->GetPostStepPoint()->GetMomentumDirection().z();
   #ifdef debug
   std::cout<<"Post Momentum direction: ("<<postdir_px<<","<<postdir_py<<","<<postdir_pz<<"), normalised momentum vector"<<std::endl;
   #endif
   //Momentum value
   auto pre_px=step->GetPreStepPoint()->GetMomentum().x();
   auto pre_py=step->GetPreStepPoint()->GetMomentum().y();
   auto pre_pz=step->GetPreStepPoint()->GetMomentum().z();
   #ifdef debug
   std::cout<<"Pre Momentum vector: ("<<pre_px/CLHEP::keV<<","<<pre_py/CLHEP::keV<<","<<pre_pz/CLHEP::keV<<") keV"<<std::endl;
   #endif
   auto post_px=step->GetPostStepPoint()->GetMomentum().x();
   auto post_py=step->GetPostStepPoint()->GetMomentum().y();
   auto post_pz=step->GetPostStepPoint()->GetMomentum().z();
   #ifdef debug
   std::cout<<"Post Momentum vector: ("<<post_px/CLHEP::keV<<","<<post_py/CLHEP::keV<<","<<post_pz/CLHEP::keV<<") keV"<<std::endl;
   #endif
   // energy deposit
   auto edep = (step->GetTotalEnergyDeposit())/CLHEP::keV;
   #ifdef debug
   std::cout<<"Edeposit: "<<edep<<" keV"<<std::endl;
   #endif
   //A la funcion G4BestUnit hay que pasarle el valor de energia en las unidades en la que la devuelve geant4 (MeV) y hace la conversion de los MeV, por eso si le paso 11 kev devuelve 0.011keV=11MeV porque los esta tomando como 11 MeV
   //auto edep1 = (step->GetTotalEnergyDeposit());
   //G4cout<< "Edeposit1: " << G4BestUnit(edep1,"Energy")<<G4endl;


  // step length
  G4double stepLength = 0.;
  //  if ( step->GetTrack()->GetDefinition()->GetPDGCharge() != 0. ) {
  //stepLength = step->GetStepLength();
  //std::cout<<"StepLength: "<<stepLength<<std::endl;
  //}
  stepLength = step->GetStepLength()/CLHEP::cm;
  #ifdef debug
  std::cout<<"StepLength: "<<stepLength<<" cm"<<std::endl;
  #endif

  int PDGID =particle->GetPDGEncoding();
  int particlenum=step->GetTrack()->GetTrackID();
  #ifdef debug
  std::cout<<"PDGID: "<<PDGID<<std::endl;
  std::cout<<"Particle generated number: "<<particlenum<<std::endl;
  #endif
  
  G4double step_initialtime= step->GetPreStepPoint()->GetGlobalTime(); 
  G4double step_finaltime= step->GetPostStepPoint()->GetGlobalTime();
  #ifdef debug
  std::cout<<"PreStep Time: "<<step_initialtime/CLHEP::ns<<" ns"<<std::endl;
  std::cout<<"PostStep Time: "<<step_finaltime/CLHEP::ns<<" ns"<<std::endl;
  #endif
  //electrons or positrons
  if(PDGID==11||PDGID==-11){ 
  fEventAction->Counting();
  fEventAction->Edepositelectrons(edep,stepLength);
  }
  //Photons steplength per event 
  if(PDGID==22){
    fEventAction->Steplength_photons(stepLength);
  }



  //if((procName=="phot")||(procName=="compt")||((procName=="eBrem"))){fEventAction->Counting();}
  


  //Cross section of photon processes
  G4EmCalculator emCalculator;
  #ifdef debug
  emCalculator.SetVerbose(1);
  #endif
  //kinetic energy at the beginning of the step to see the probability the particle has to suffer the process 
  G4double fEkin = step->GetPreStepPoint()->GetKineticEnergy();
  G4String fParticle = particle->GetParticleName();
  G4NistManager* man = G4NistManager::Instance();
  G4Material* material = man->FindOrBuildMaterial("G4_Ge");
  G4double density = material->GetDensity();
  //#ifdef debug
  //std::cout<<material<<"Density: "<<G4BestUnit(density,"Volumic Mass")<<std::endl;
  //#endif
 
  G4double massSigmacalc,massSigma;
  if (PDGID==22){
    massSigmacalc = 1./((density/(CLHEP::g/CLHEP::cm3))*stepLength);
    massSigma = emCalculator.ComputeCrossSectionPerVolume(fEkin,particle,procName,material)/density;
    #ifdef debug
    G4cout << "Process: " << procName << " with geant4 Cross Section= "<< G4BestUnit(massSigma, "Surface/Mass")<<G4endl;
    G4cout << "Calculated Cross Section= "<< massSigmacalc<<" cm2/g"<<G4endl;
    #endif
    //Compare the simulated result of the cross section  with the 'input' data (massSigmacalc), i.e. with the cross sections stored in the PhysicsTables and used by Geant4 (massSigma).
    G4double diffcrosssect = massSigma/(CLHEP::cm2/CLHEP::g)-massSigmacalc;
    #ifdef debug
    G4cout<<"Difference between cross-section as an input(geant4) and simulated(calculated): "<<diffcrosssect<<std::endl;
    #endif
    analysisManager->FillH1(9, diffcrosssect);
  }


  //Number of electron holes pair
  G4double meaneion = 0.0;
  G4int samplingeion = 0.0;
  G4ElectronIonPair* fElIonPair = G4LossTableManager::Instance()->ElectronIonPair();
  //Fano Factor
  //std::cout<<"Fano Factor: "<<fElIonPair->invFanoFactor<<std::endl;
  meaneion = fElIonPair->MeanNumberOfIonsAlongStep(step);
  //sampling e-h with gamma distribution inv fano factor=1/0.2 DEFAULT
  //samplingeion=fElIonPair->SampleNumberOfIonsAlongStep(step);
  //sampling e-h with gamma distribution changing inv fano factor 
  G4double invFanoFactor = 1.0/0.1;
  samplingeion = G4lrint(G4RandGamma::shoot(meaneion*invFanoFactor,invFanoFactor));
  //non ionizing energy loss
  G4double niel = step->GetNonIonizingEnergyDeposit();

  #ifdef debug
  std::cout<<"Mean e-h generated: "<<meaneion<<std::endl;
  std::cout<<"Sampling e-h with Fano factor: "<<samplingeion<<std::endl;
  std::cout<<"NIEL: "<<G4BestUnit(niel, "Energy")<<std::endl;
  #endif
  
  //Storing the number of electron hole pairs created after the energy of an electron or positron is deposited (by eIoni, eBrem and msc processes)
   // if((meaneion!=0)&&(procName=="eIoni")){
   if(meaneion!=0){ 
    fEventAction->CountingEHpairs(samplingeion);
  }


   //refraction index //seg fault
   // If the material doesn't have a RINDEX property vector then return nullptr
   //G4MaterialPropertiesTable* MPT = material->GetMaterialPropertiesTable();
   //G4MaterialPropertyVector* rIndex = MPT->GetProperty(kRINDEX);
   //Min and max refraction indices
   //G4double nmin= rIndex->GetMinValue();
   //G4double nmax= rIndex->GetMaxValue();
   //std::cout<<"Min and max refraction index: "<<nmin<<" "<<nmax<<std::endl;





   analysisManager->FillH1(2, edep);                             
   analysisManager->FillH1(3, prex);
   analysisManager->FillH1(4, prey);
   analysisManager->FillH1(5, prez);
   analysisManager->FillH1(6, stepLength);

   analysisManager->FillH1(8, eventid);

   analysisManager->FillH2(0, prez, prex);
   analysisManager->FillH2(1, prez, prey);


  // fill ntuple STEP in Ge
  analysisManager->FillNtupleDColumn(1,0,edep);
  analysisManager->FillNtupleDColumn(1,1,prex);
  analysisManager->FillNtupleDColumn(1,2,prey);
  analysisManager->FillNtupleDColumn(1,3,prez);
  analysisManager->FillNtupleDColumn(1,4,postx);
  analysisManager->FillNtupleDColumn(1,5,posty);
  analysisManager->FillNtupleDColumn(1,6,postz);
  analysisManager->FillNtupleDColumn(1,7,stepLength);
  analysisManager->FillNtupleIColumn(1,8,PDGID);
  analysisManager->FillNtupleIColumn(1,9,eventid);
  analysisManager->FillNtupleDColumn(1,10,pre_px);
  analysisManager->FillNtupleDColumn(1,11,pre_py);
  analysisManager->FillNtupleDColumn(1,12,pre_pz);
  analysisManager->FillNtupleDColumn(1,13,post_px);
  analysisManager->FillNtupleDColumn(1,14,post_py);
  analysisManager->FillNtupleDColumn(1,15,post_pz);
  analysisManager->FillNtupleIColumn(1,16,particlenum);
  analysisManager->FillNtupleDColumn(1,17,step_initialtime);
  analysisManager->FillNtupleDColumn(1,18,step_finaltime);

  analysisManager->AddNtupleRow(1);


  //kill event after first interaction
  /*std::cout<<"Stepkilled"<<std::endl;
  if(step->GetTrack()->GetTrackID()>=1){
  G4RunManager::GetRunManager()->AbortEvent();}*/

  //  fEventAction->variables(x,y,z,edep,stepLength,PDGID);
  }//volume target


















   //If we are inside sensitive KCl, polyethylene volume, Vacuum volume (VD plane) or Al window fill tree
   if((volume->GetName()=="KClTarget")||(volume->GetName()=="PolyAbsorber")||(volume->GetName()=="VacuumTarget")||(volume->GetName()=="AlWindow")){

     //Event ID   
     auto eventid = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
     #ifdef debug
     std::cout<<"EventID: "<<eventid<<std::endl;
     #endif
     int PDGID = particle->GetPDGEncoding();
     #ifdef debug
     std::cout<<"PDGID: "<<PDGID<<std::endl;
     #endif
     auto edep = (step->GetTotalEnergyDeposit())/CLHEP::keV;
     #ifdef debug
     std::cout<<"Edeposit: "<<edep<<" keV"<<std::endl;
     #endif
     //Name of the process
     std::string procName=step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
     #ifdef debug
     std::cout<<"Process: "<<procName<<std::endl;
     #endif
     
     //Position
     auto prex=PreSteppos.getX()/CLHEP::cm;
     auto prey=PreSteppos.getY()/CLHEP::cm;
     auto prez=PreSteppos.getZ()/CLHEP::cm;
     auto postx=PostSteppos.getX()/CLHEP::cm;
     auto posty=PostSteppos.getY()/CLHEP::cm;
     auto postz=PostSteppos.getZ()/CLHEP::cm;
     
     G4double stepLength = step->GetStepLength()/CLHEP::cm;
     #ifdef debug
     std::cout<<"step length: "<<stepLength<<std::endl;
     #endif
     
     auto pre_px=step->GetPreStepPoint()->GetMomentum().x();
     auto pre_py=step->GetPreStepPoint()->GetMomentum().y();
     auto pre_pz=step->GetPreStepPoint()->GetMomentum().z();
     
     if(volume->GetName()=="KClTarget"){
       analysisManager->FillNtupleIColumn(2,0,eventid); 
       analysisManager->FillNtupleDColumn(2,1,edep);
       analysisManager->FillNtupleDColumn(2,2,prex);
       analysisManager->FillNtupleDColumn(2,3,prey);
       analysisManager->FillNtupleDColumn(2,4,prez);
       analysisManager->FillNtupleDColumn(2,5,postx);
       analysisManager->FillNtupleDColumn(2,6,posty);
       analysisManager->FillNtupleDColumn(2,7,postz);
       analysisManager->FillNtupleDColumn(2,8,stepLength);
       analysisManager->FillNtupleIColumn(2,9,PDGID);
       analysisManager->FillNtupleDColumn(2,10,pre_px);
       analysisManager->FillNtupleDColumn(2,11,pre_py);
       analysisManager->FillNtupleDColumn(2,12,pre_pz);

       analysisManager->AddNtupleRow(2);
     }

     if(volume->GetName()=="VacuumTarget"){
       analysisManager->FillNtupleIColumn(3,0,eventid);
       analysisManager->FillNtupleDColumn(3,1,edep);
       analysisManager->FillNtupleDColumn(3,2,prex);
       analysisManager->FillNtupleDColumn(3,3,prey);
       analysisManager->FillNtupleDColumn(3,4,prez);
       analysisManager->FillNtupleDColumn(3,5,stepLength);
       analysisManager->FillNtupleIColumn(3,6,PDGID);
       analysisManager->FillNtupleDColumn(3,7,pre_px);
       analysisManager->FillNtupleDColumn(3,8,pre_py);
       analysisManager->FillNtupleDColumn(3,9,pre_pz);

       analysisManager->AddNtupleRow(3);
     }

      if(volume->GetName()=="PolyAbsorber"){
       analysisManager->FillNtupleIColumn(4,0,eventid);
       analysisManager->FillNtupleDColumn(4,1,edep);
       analysisManager->FillNtupleDColumn(4,2,prex);
       analysisManager->FillNtupleDColumn(4,3,prey);
       analysisManager->FillNtupleDColumn(4,4,prez);
       analysisManager->FillNtupleDColumn(4,5,postx);
       analysisManager->FillNtupleDColumn(4,6,posty);
       analysisManager->FillNtupleDColumn(4,7,postz);
       analysisManager->FillNtupleDColumn(4,8,stepLength);
       analysisManager->FillNtupleIColumn(4,9,PDGID);
       analysisManager->FillNtupleDColumn(4,10,pre_px);
       analysisManager->FillNtupleDColumn(4,11,pre_py);
       analysisManager->FillNtupleDColumn(4,12,pre_pz);

       analysisManager->AddNtupleRow(4);
     }

      if(volume->GetName()=="AlWindow"){
  
	analysisManager->FillNtupleIColumn(5,0,eventid);
	analysisManager->FillNtupleDColumn(5,1,edep);
	analysisManager->FillNtupleDColumn(5,2,prex);
	analysisManager->FillNtupleDColumn(5,3,prey);
	analysisManager->FillNtupleDColumn(5,4,prez);
	analysisManager->FillNtupleDColumn(5,5,postx);
	analysisManager->FillNtupleDColumn(5,6,posty);
	analysisManager->FillNtupleDColumn(5,7,postz);
	analysisManager->FillNtupleDColumn(5,8,stepLength);
	analysisManager->FillNtupleIColumn(5,9,PDGID);
	analysisManager->FillNtupleDColumn(5,10,pre_px);
	analysisManager->FillNtupleDColumn(5,11,pre_py);
	analysisManager->FillNtupleDColumn(5,12,pre_pz);

	analysisManager->AddNtupleRow(5);
     }


   }



}
