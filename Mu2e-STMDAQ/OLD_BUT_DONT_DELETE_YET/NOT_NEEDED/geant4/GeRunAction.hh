#ifndef GeRunAction_h
#define GeRunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

class GeRunAction : public G4UserRunAction
{
public:
  GeRunAction();
  ~GeRunAction();

public:
  void BeginOfRunAction(const G4Run*);
  void EndOfRunAction(const G4Run*);
};

#endif
