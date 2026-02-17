#ifndef GePhysicsList_h
#define GePhysicsList_h 1

#include "G4VUserPhysicsList.hh"
#include "globals.hh"

class GePhysicsList: public G4VUserPhysicsList
{
public:
  GePhysicsList();
  ~GePhysicsList();

protected:
  // Construct particle and physics
  void ConstructParticle();
  void ConstructProcess();
  void SetCuts();


protected:
  // these methods Construct particles 
  void ConstructBosons();
  void ConstructLeptons();
  void ConstructBaryons();

protected:
  // these methods Construct physics processes and register them
  void ConstructEM();
};

#endif
