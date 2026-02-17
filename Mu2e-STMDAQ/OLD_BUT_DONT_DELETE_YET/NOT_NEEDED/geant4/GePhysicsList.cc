#include "globals.hh"
#include "GePhysicsList.hh"

#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"

#include "G4PhysicsListHelper.hh"

GePhysicsList::GePhysicsList():  G4VUserPhysicsList()
{
  //  defaultCutValue = 1.0*cm;
  defaultCutValue = 1.0*CLHEP::cm;
  SetVerboseLevel(1);
}

GePhysicsList::~GePhysicsList()
{}

void GePhysicsList::ConstructParticle()
{
  // In this method, static member functions should be called
  // for all particles which you want to use.
  // This ensures that objects of these particle types will be
  // created in the program. 
  G4cout<<"GePhysicsList::ConstructParticle"<<G4endl;

  ConstructBosons();
  ConstructLeptons();
  ConstructBaryons();
  G4cout<<"GePhysicsList::ConstructParticle done"<<G4endl;
}

void GePhysicsList::ConstructBosons()
{
  G4cout<<"constructing bosons"<<G4endl;
  // pseudo-particles
  G4Geantino::GeantinoDefinition();
  G4ChargedGeantino::ChargedGeantinoDefinition();

  // gamma
  G4Gamma::GammaDefinition();

  // optical photon
  G4OpticalPhoton::OpticalPhotonDefinition();
}

#include "G4LeptonConstructor.hh"
void GePhysicsList::ConstructLeptons()
{
  G4cout<<"constructing leptons"<<G4endl;
  G4LeptonConstructor pConstructor;
  pConstructor.ConstructParticle();
}

#include "G4BaryonConstructor.hh"
void GePhysicsList::ConstructBaryons()
{
  G4cout<<"constructing baryons"<<G4endl;
  G4BaryonConstructor  pConstructor;
  pConstructor.ConstructParticle();  
}

void GePhysicsList::ConstructProcess()
{
  G4cout<<"GePhysicsList::ConstructProcess"<<G4endl;
  AddTransportation();
  ConstructEM();
  G4cout<<"GePhysicsList::ConstructProcess done"<<G4endl;
}

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4eMultipleScattering.hh"
#include "G4hMultipleScattering.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4hIonisation.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hPairProduction.hh"

#include "G4ionIonisation.hh"
//Default
void GePhysicsList::ConstructEM()
{
  G4cout<<"GePhysicsList::ConstructEM"<<G4endl;
  GetParticleIterator()->reset();
  while( (*GetParticleIterator())() ){
    G4ParticleDefinition* particle = GetParticleIterator()->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();

   
    //G4cout<<"for "<<particleName <<G4endl;
        if (particleName == "gamma") {
      // gamma         
      G4cout<<"adding photon processes "<<G4endl;
      pmanager->AddDiscreteProcess(new G4PhotoElectricEffect);
      pmanager->AddDiscreteProcess(new G4ComptonScattering);
      pmanager->AddDiscreteProcess(new G4GammaConversion);

    } else if (particleName == "e-") {
      //electron
      G4cout<<"adding electron processes "<<G4endl;
      pmanager->AddProcess(new G4eMultipleScattering, -1, 1, 1);
      pmanager->AddProcess(new G4eIonisation,         -1, 2, 2);
      pmanager->AddProcess(new G4eBremsstrahlung,     -1, 3, 3);      
	}
	
      //Adding positron interactions for photons E>1022 keV
      else if (particleName == "e+") {
      //positron
      pmanager->AddProcess(new G4eMultipleScattering, -1, 1, 1);
      pmanager->AddProcess(new G4eIonisation,         -1, 2, 2);
      pmanager->AddProcess(new G4eBremsstrahlung,     -1, 3, 3);
      pmanager->AddProcess(new G4eplusAnnihilation,    0,-1, 4);

	}
  }
}


/*void GePhysicsList::ConstructEM()                                                                         
{
 G4cout<<"GePhysicsList::ConstructEM"<<G4endl;                                                       
 G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

 auto myParticleIterator=GetParticleIterator();
 myParticleIterator->reset();

 while( (*myParticleIterator)() ){                                                                     
    G4ParticleDefinition* particle = myParticleIterator->value();                                       
    G4String particleName = particle->GetParticleName();
    if (particleName == "gamma") {
      std::cout<<"gamma process"<<std::endl;
      ph->RegisterProcess(new G4PhotoElectricEffect(), particle);
      ph->RegisterProcess(new G4ComptonScattering(), particle);
      ph->RegisterProcess(new G4GammaConversion(), particle);

    } else if (particleName == "e-") {
      std::cout<<"e- process"<<std::endl;
      ph->RegisterProcess(new G4eIonisation(), particle);
      ph->RegisterProcess(new G4eBremsstrahlung(), particle);

    } else if (particleName == "e+") {

      ph->RegisterProcess(new G4eIonisation(), particle);
      ph->RegisterProcess(new G4eBremsstrahlung(), particle);
      ph->RegisterProcess(new G4eplusAnnihilation(), particle);

    }
  }
}

*/


void GePhysicsList::SetCuts()
{
  G4cout<<"setting default cuts "<<G4endl;
  SetCutsWithDefault();
  G4cout<<"setting default cuts done"<<G4endl;
}
