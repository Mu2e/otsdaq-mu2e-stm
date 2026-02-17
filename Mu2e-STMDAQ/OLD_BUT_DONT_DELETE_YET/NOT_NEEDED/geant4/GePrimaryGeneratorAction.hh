#ifndef GePrimaryGeneratorAction_h
#define GePrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"

class GeUserDetectorConstruction;
class G4ParticleGun;
class G4Event;

class GePrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  GePrimaryGeneratorAction(GeUserDetectorConstruction*);    
  ~GePrimaryGeneratorAction();

public:
  void GeneratePrimaries(G4Event*);

private:
  G4ParticleGun* particleGun;
  GeUserDetectorConstruction* myDetector;

  int pdgid=22;
  double momMeV, thetamin, initpos_x_cm, initpos_y_cm, initpos_z_cm, init_rmax_cm, bool_dodisk_pos, bool_docyl_pos, bool_dofixed_pos;
  bool dodisk_pos=false;
  bool docyl_pos=false;
  bool dofixed_pos=false;
};

#endif
