#include "GePrimaryGeneratorAction.hh"
#include "GeUserDetectorConstruction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"

#include "Randomize.hh"
//Add
#include "G4UnitsTable.hh"

#include <iostream>
#include <cmath>
#include <utility>
#include <limits>
#include "G4INCLIRandomGenerator.hh"
#include "G4INCLThreeVector.hh"
#include "G4INCLGlobals.hh"
#include "G4INCLConfig.hh"
#include "G4INCLRandom.hh"
#include "G4RandomDirection.hh"

//#define debug

using namespace G4INCL;

GePrimaryGeneratorAction::GePrimaryGeneratorAction( GeUserDetectorConstruction* myDC) :
  myDetector(myDC)
{
  //#ifdef debug
  G4cout<<"GePrimaryGeneratorAction: creating photon "<<G4endl;
  //#endif
  /*  G4int n_particle = 1;
  particleGun = new G4ParticleGun(n_particle);

  // default particle
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  //G4ParticleDefinition* particle = particleTable->FindParticle("e-");
  G4ParticleDefinition* particle = particleTable->FindParticle("gamma"); 
  //G4ParticleDefinition* particle = particleTable->FindParticle("neutron");
  //G4ParticleDefinition* particle = particleTable->FindParticle("alpha");
  //G4ParticleDefinition* particle = particleTable->FindParticle("proton");

  particleGun->SetParticleDefinition(particle);
  //particleGun->SetParticlePosition(G4ThreeVector(0,0,0));
  //particleGun->SetParticlePosition(G4ThreeVector(0.56*CLHEP::m,0,0));
  */
}

GePrimaryGeneratorAction::~GePrimaryGeneratorAction()
{
  delete particleGun;
}

void GePrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{


  if(anEvent->GetEventID() == 0){
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

	  if(element.compare("pdgid:") == 0) ss >> pdgid;	  
	  if(element.compare("momMeV:") == 0) ss >> momMeV;
	  if(element.compare("thetamin:") == 0) ss >> thetamin;
	  if(element.compare("initpos_x_cm:") == 0) ss >> initpos_x_cm;
	  if(element.compare("initpos_y_cm:") == 0) ss >> initpos_y_cm;
	  if(element.compare("initpos_z_cm:") == 0) ss >> initpos_z_cm;

	  if(element.compare("bool_dodisk_pos:") == 0)
	    { ss >> bool_dodisk_pos;
	      if(bool_dodisk_pos==1){ dodisk_pos=true; std::cout<<"do disk initial pos"<<std::endl; }
	    }
	  
	  if(element.compare("init_rmax_cm:") == 0) ss >> init_rmax_cm;
	  
	  if(element.compare("bool_docyl_pos:") == 0)
            { ss >> bool_docyl_pos;
              if(bool_docyl_pos==1){ docyl_pos=true; std::cout<<"do cyl initial pos"<<std::endl; }
            }
	  
	  if(element.compare("bool_dofixed_pos:") == 0)
            { ss >> bool_dofixed_pos;
              if(bool_dofixed_pos==1){ dofixed_pos=true; std::cout<<"do fixed initial pos"<<std::endl; }
            }
	}
      }
    input_file.close();
    std::cout<<"Config file closed for primary particles"<<std::endl;
    std::cout<<"PDG id: "<<pdgid<<", Initial pos (x,y,z)cm: ("<<initpos_x_cm<<","<<initpos_y_cm<<","<<initpos_z_cm<<"), p="<<momMeV<<"MeV, thetamin: "<<thetamin<<std::endl;
  }

   
  G4int n_particle = 1;
  particleGun = new G4ParticleGun(n_particle);

  // default particle
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  //G4ParticleDefinition* particle = particleTable->FindParticle("gamma");
  G4ParticleDefinition* particle = particleTable->FindParticle(pdgid);
  double massMeV = particleTable->FindParticle(pdgid)->GetPDGMass();
  
  particleGun->SetParticleDefinition(particle);
  #ifdef debug
  G4cout<<"GePrimaryGeneratorAction: setting position "<<G4endl;
  #endif

  G4double x,y,z;
  
  if( docyl_pos==true ) {
    
    //Generate particles in a cylinder at origin 
    G4double cyl_radius = (6.35/2)*CLHEP::cm;//cm
    G4double h = 13.97*CLHEP::cm;//cm 
    G4double phi = G4UniformRand() * CLHEP::twopi;
    G4double rho = G4UniformRand() * cyl_radius;
    double x_center = initpos_x_cm*CLHEP::cm;
    double y_center = initpos_y_cm*CLHEP::cm;
    double z_center = initpos_z_cm*CLHEP::cm;
    x = x_center+rho * cos(phi);
    y = y_center+(G4UniformRand() * h) - (h/2);
    z = z_center+rho * sin(phi);
    particleGun->SetParticlePosition(G4ThreeVector(x,y,z));
  }

  if( dofixed_pos==true ) {
    
    //Generate particles in a fixed position
    x = initpos_x_cm*CLHEP::cm;
    y = initpos_y_cm*CLHEP::cm;
    z = initpos_z_cm*CLHEP::cm;   
    particleGun->SetParticlePosition(G4ThreeVector(x,y,z));
  }

  if( dodisk_pos==true ) {
  
    //centre of the disk and radius
    double _xc = initpos_x_cm*CLHEP::cm;
    double _yc = initpos_y_cm*CLHEP::cm;
    double _zc = initpos_z_cm*CLHEP::cm;
    double _rmax = init_rmax_cm*CLHEP::cm;
 
    //Generate particles in a disk
    double rho = sqrt(G4UniformRand());
    double theta = G4UniformRand() * CLHEP::twopi;
    x = _xc + _rmax * rho * cos(theta);
    y = _yc + _rmax * rho * sin(theta);
    z = _zc;
    
    particleGun->SetParticlePosition(G4ThreeVector(x,y,z));
  }

  //Kinetic Energy
  double EkeV = (sqrt( momMeV*momMeV + massMeV*massMeV ) - massMeV)*1000;
  
  particleGun->SetParticleEnergy(EkeV*CLHEP::keV);
  G4double energy=particleGun->GetParticleEnergy();

  //Momentum in a fixed direction
  //particleGun->SetParticleMomentumDirection(G4ThreeVector(0,0,1));
 
  //Creating a sphere in the initial configuration
  /*G4ThreeVector momentumUnitVector = G4RandomDirection();
  particleGun->SetParticleMomentumDirection(momentumUnitVector);
  */

  //Specify range of costheta in the sphere [min_cos_theta,1], all posible combinations for theta, phi is always between 0 and 2pi
  double min_cos_theta = thetamin;
  G4double cosTheta;
  G4double sinTheta2;

  if(min_cos_theta == 1 ){  cosTheta = min_cos_theta;}
  //Copied from G4RandomDirection.hh
  else {
    cosTheta  = G4UniformRand();
    while(cosTheta < min_cos_theta){ cosTheta = G4UniformRand(); }
  }
  
  sinTheta2 = 1. - cosTheta*cosTheta;  
  if( sinTheta2 < 0.)  sinTheta2 = 0.;
  G4double sinTheta  = std::sqrt(sinTheta2); 
  G4double phi       = CLHEP::twopi*G4UniformRand();
  particleGun->SetParticleMomentumDirection(G4ThreeVector(sinTheta*std::cos(phi),sinTheta*std::sin(phi), cosTheta).unit()); 


  particleGun->GeneratePrimaryVertex(anEvent);
  
  #ifdef debug
  std::cout<<"  "<<std::endl;
  std::cout<<"----------------------------Event: "<<anEvent->GetEventID() <<"-------------------------------------"<<std::endl;
  std::cout<<"PDG id: "<<pdgid<<" Mass: "<<massMeV<<" MeV"<<std::endl;
  std::cout<<"Generated Pos= "<<G4ThreeVector(x,y,z)/CLHEP::cm<<" cm"<<std::endl;
  std::cout<< "Initial Energy: " << G4BestUnit(energy,"Energy")<<std::endl;
  std::cout<<"Initial Momentum Vector: "<<particleGun->GetParticleMomentumDirection()<<" Initial Momentum Mag: "<<anEvent->GetPrimaryVertex()->GetPrimary()->GetTotalMomentum()/CLHEP::keV<<" keV Etot energy: "<<anEvent->GetPrimaryVertex()->GetPrimary()->GetTotalEnergy()<<std::endl;
  std::cout<<"cosTheta: "<<cosTheta<<std::endl;
  std::cout<<"Number of particle generated: "<<particleGun->GetNumberOfParticles()<<std::endl; 
  //std::cout<<"Energy: "<<(particleGun->GetParticleEnergy())/CLHEP::keV<<" keV"<<std::endl;
  std::cout<<"  "<<std::endl;
  #endif

}
