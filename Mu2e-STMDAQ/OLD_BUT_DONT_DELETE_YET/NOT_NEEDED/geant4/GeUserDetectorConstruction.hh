#ifndef GeUserDetectorConstruction_h
#define GeUserDetectorConstruction_h 

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"

class G4Box;
class G4Tubs;
class G4MultiUnion;
class G4UnionSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4VPVParameterisation;
class G4UserLimits;

class GeUserDetectorConstruction : public G4VUserDetectorConstruction
{
public:

  GeUserDetectorConstruction();
  ~GeUserDetectorConstruction();

public:

  G4VPhysicalVolume* Construct();
  G4double GetWorldFullLength()   {return worldLength;};

  //G4Material* GetMaterial() const  {return fMaterial;};

private:

  G4Box*             solidWorld;    // pointer to the solid envelope 
  G4LogicalVolume*   logicWorld;    // pointer to the logical envelope
  G4VPhysicalVolume* physiWorld;    // pointer to the physical envelope

  //Use this for a squared or rectangular block detector
  //G4Box*             solidTarget;   // pointer to the solid Target
  //Use this for a cylindrical detector
  //G4Tubs*             solidTarget;   // pointer to the solid Target
  //Use this for combined detectors
  //G4MultiUnion*         solidTarget;
  G4UnionSolid*        solidTarget;

  
  G4LogicalVolume*   logicTarget; // pointer to the logical Target
  G4LogicalVolume*   logicAlWindow;
  G4LogicalVolume*   logicPoly;
  G4LogicalVolume*   logicKCl;
  G4LogicalVolume*   logicVacuum;
  
  G4VPhysicalVolume* physiTarget;   // pointer to the physical Target

  G4UserLimits* stepLimit;             // pointer to user step limits

  G4double worldLength;            // Full length of the world volume
  G4double fTargetLength;

  //  G4Material* fMaterial;
};

#endif
