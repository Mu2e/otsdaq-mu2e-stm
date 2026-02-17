#ifndef GeEventAction_h
#define GeEventAction_h

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <iostream>
#include <vector>
using namespace std;
/// Event action class
///
/// It defines data members to hold the energy deposit and track lengths
/// of charged particles in Absober and Gap layers:
/// - fEnergyAbs, fEnergyGap, fTrackLAbs, fTrackLGap
/// which are collected step by step via the functions
/// - AddAbs(), AddGap()

class GeEventAction : public G4UserEventAction
{
public:
  GeEventAction();
  virtual ~GeEventAction();

  virtual void  BeginOfEventAction(const G4Event* event);
  virtual void    EndOfEventAction(const G4Event* event);
    
  void Edepositelectrons(G4double de, G4double dl);
  void Steplength_photons(G4double dlgamma);
  void Counting();
  void CountingEHpairs(G4int numbereh);

    

private:
  G4double  fEnergyAbs;
  G4double  fTrackLAbs;
  G4double  fTrackgamma;
  G4int  num;
  G4int  fEHpairs;
  G4int  ncompt;

};

// inline functions

inline void GeEventAction::Edepositelectrons(G4double de, G4double dl) {
  //Total energy deposited by electrons and total track length
  fEnergyAbs += de; 
  fTrackLAbs += dl;

}

inline void GeEventAction::Steplength_photons(G4double dlgamma) {
  //Total track length photons 
  fTrackgamma += dlgamma;

}



inline void GeEventAction::Counting() {
  num++;
}

inline void GeEventAction::CountingEHpairs(G4int numbereh){
  fEHpairs += numbereh;
}

//inline void GeEventAction::CountingComptons() {
//ncompt++;
//}


/*inline void GeEventAction::Edepositphotons(G4double dep) {
  //Total energy deposited by photons                                                                                            
  edepphotons += dep;
  }*/



/*inline void GeEventAction::variables(G4double dxvect,G4double dyvect,G4double dzvect,G4double dedepvect,G4double dsteplengthvect,int dPDGIDvect){
  xvect.push_back(dxvect);
  yvect.push_back(dyvect);
  zvect.push_back(dzvect);
  edepvect.push_back(dedepvect);
  steplengthvect.push_back(dsteplengthvect);
  PDGIDvect.push_back(dPDGIDvect);
  }*/
                     
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
