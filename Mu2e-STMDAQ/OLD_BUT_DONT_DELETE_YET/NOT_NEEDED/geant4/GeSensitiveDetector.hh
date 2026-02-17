#ifndef GeSensitiveDetector_h
#define GeSensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;

class GeSensitiveDetector : public G4VSensitiveDetector
{
public:
  GeSensitiveDetector(G4String);
  ~GeSensitiveDetector();

  void Initialize(G4HCofThisEvent*);
  G4bool ProcessHits(G4Step*, G4TouchableHistory*);
  void EndOfEvent(G4HCofThisEvent*);

};
#endif
