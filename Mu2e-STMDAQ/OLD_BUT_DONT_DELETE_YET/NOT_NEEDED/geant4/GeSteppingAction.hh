#ifndef GeSteppingAction_h
#define GeSteppingAction_h 1

#include "G4UserSteppingAction.hh"

class GeUserDetectorConstruction;
class GeEventAction;

/// Stepping action class.
///
/// In UserSteppingAction() there are collected the energy deposit and track 
/// lengths of charged particles in Absober and Gap layers and
/// updated in B4aEventAction.

class GeSteppingAction : public G4UserSteppingAction
{
public:
  GeSteppingAction(const GeUserDetectorConstruction* detectorConstruction,
                    GeEventAction* eventAction);
  virtual ~GeSteppingAction();

  virtual void UserSteppingAction(const G4Step* step);
    
private:
  const GeUserDetectorConstruction* fDetConstruction;
  GeEventAction*  fEventAction;  
};


#endif
