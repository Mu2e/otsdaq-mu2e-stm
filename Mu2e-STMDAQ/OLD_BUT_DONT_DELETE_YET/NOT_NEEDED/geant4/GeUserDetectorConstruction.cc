//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "GeUserDetectorConstruction.hh"
#include "GeSensitiveDetector.hh"

#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4MultiUnion.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4SDManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"

#include "G4UserLimits.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4ios.hh"

#include "G4NistManager.hh"

#include "G4UnitsTable.hh"

#define debug

GeUserDetectorConstruction::GeUserDetectorConstruction() :
  solidWorld(0),  logicWorld(0),  physiWorld(0),
  solidTarget(0), logicTarget(0), logicAlWindow(0), logicPoly(0), logicKCl(0), logicVacuum(0),
  physiTarget(0), stepLimit(0), //fMaterial(0), 
  worldLength(100*CLHEP::m), fTargetLength(0)
{
}

GeUserDetectorConstruction::~GeUserDetectorConstruction()
{
  delete stepLimit;
}

G4VPhysicalVolume* GeUserDetectorConstruction::Construct()
{
  //Activate detectors//
  bool Ge_detector=false;
  bool KCl_detector=false;
  bool Poly_detector=false;
  bool VDplane_STM=false;
  bool Al_Window=false;

  //read the txt file
  double bool_doHPGe, bool_doPoly, bool_doKClPot, bool_doVDplane, bool_doAlWindow;

  double HPGe_rot_angley, VD_rmax_cm, VD_halflength_cm, VD_xcenter_cm, VD_ycenter_cm, VD_zcenter_cm;

  double Poly_rmax_cm, Poly_halflength_cm, Poly_xcenter_cm, Poly_ycenter_cm, Poly_zcenter_cm;

  double AlWindow_rmax_cm, AlWindow_halflength_cm, AlWindow_xcenter_cm, AlWindow_ycenter_cm, AlWindow_zcenter_cm; 
    
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
        if(element.compare("bool_doHPGe:") == 0) {
	  ss >> bool_doHPGe;
	  if( bool_doHPGe == 1 ) {Ge_detector=true; std::cout<<"Build Ge detector"<<std::endl;}
	}

	if(element.compare("bool_doPoly:") == 0) {
          ss >> bool_doPoly;
          if( bool_doPoly == 1 ) {Poly_detector=true; std::cout<<"Build Poly detector"<<std::endl;}
        }

	if(element.compare("bool_doKClPot:") == 0) {
          ss >> bool_doKClPot;
          if( bool_doKClPot == 1 ) {KCl_detector=true; std::cout<<"Build KCl detector"<<std::endl;}
        }

	if(element.compare("bool_doVDplane:") == 0) {
          ss >> bool_doVDplane;
          if( bool_doVDplane == 1 ) {VDplane_STM=true; std::cout<<"Build VD detector"<<std::endl;}
        }

	if(element.compare("bool_doAlWindow:") == 0) {
          ss >> bool_doAlWindow;
          if( bool_doAlWindow == 1 ) {Al_Window=true; std::cout<<"Build Al Window"<<std::endl;}
        }
	
        if(element.compare("HPGe_rot_angley:") == 0) ss >> HPGe_rot_angley;

	if(element.compare("VD_rmax_cm:") == 0) ss >> VD_rmax_cm;
	if(element.compare("VD_halflength_cm:") == 0) ss >> VD_halflength_cm;
	if(element.compare("VD_xcenter_cm:") == 0) ss >> VD_xcenter_cm;
	if(element.compare("VD_ycenter_cm:") == 0) ss >> VD_ycenter_cm;
	if(element.compare("VD_zcenter_cm:") == 0) ss >> VD_zcenter_cm;

	if(element.compare("Poly_rmax_cm:") == 0) ss >> Poly_rmax_cm;
        if(element.compare("Poly_halflength_cm:") == 0) ss >> Poly_halflength_cm;
        if(element.compare("Poly_xcenter_cm:") == 0) ss >> Poly_xcenter_cm;
        if(element.compare("Poly_ycenter_cm:") == 0) ss >> Poly_ycenter_cm;
        if(element.compare("Poly_zcenter_cm:") == 0) ss >> Poly_zcenter_cm;

	if(element.compare("AlWindow_rmax_cm:") == 0) ss >> AlWindow_rmax_cm;
        if(element.compare("AlWindow_halflength_cm:") == 0) ss >> AlWindow_halflength_cm;
        if(element.compare("AlWindow_xcenter_cm:") == 0) ss >> AlWindow_xcenter_cm;
        if(element.compare("AlWindow_ycenter_cm:") == 0) ss >> AlWindow_ycenter_cm;
        if(element.compare("AlWindow_zcenter_cm:") == 0) ss >> AlWindow_zcenter_cm;
	

	
      }
    }
  input_file.close();
  
  #ifdef debug
  G4cout<<"GeUserDetectorConstruction::Construct: "<<G4endl;
  #endif
  G4NistManager* man=G4NistManager::Instance();
  //G4Material* Vacuum = new G4Material("Vacuum",1.0,1.01*g/mole,1.0E-25*g/cm3,kStateGas,2.73*kelvin,3.0E-18*pascal);
  G4Material* Vacuum    = man->FindOrBuildMaterial("G4_Galactic");


  //--------- Definitions of Solids, Logical Volumes, Physical Volumes ---------
  // World

  G4double worldboxside = worldLength / 2;

  G4GeometryManager::GetInstance()->SetWorldMaximumExtent(worldLength);
  solidWorld= new G4Box("World",worldboxside,worldboxside,worldboxside);
  logicWorld= new G4LogicalVolume( solidWorld, Vacuum, "World", 0, 0, 0);

  //  Must place the World Physical volume unrotated at (0,0,0).
  // 
  physiWorld = new G4PVPlacement(0,               // no rotation
				 G4ThreeVector(0, 0, 0), // at (0,0,0)
				 logicWorld,      // its logical volume
				 "World",         // its name
				 0,               // its mother  volume
				 false,           // no boolean operations
				 0);              // copy number

  //This places the logical volume logicWorld at the origin of the mother volume World, unrotated. The resulting physical volume is named “World” and has a copy number of 0.
  //------------------------------ 
  // Target
  //------------------------------
 
  G4Material* TargetMater=man->FindOrBuildMaterial("G4_Ge");
  G4Material* PolyMaterial=man->FindOrBuildMaterial("G4_POLYETHYLENE");
  G4Material* AlWindowMaterial=man->FindOrBuildMaterial("G4_Al");
  
  //G4Material* TargetMater=man->FindOrBuildMaterial("G4_Si");
  //G4Material* TargetMater=man->FindOrBuildMaterial("Vacuum");


  //SINGLE VOLUME
  /*    fTargetLength=5*CLHEP::cm;
    G4double fHalfTargetLength=0.5*fTargetLength;

  //This is a block detector
  //G4Box* solidTarget = new G4Box("Target", 5*CLHEP::cm, 5*CLHEP::cm, fHalfTargetLength);
 
  //This is a cylinder detector with a hole in the middle
  G4Tubs* solidTarget = new G4Tubs("Target",0.88*CLHEP::cm,4.5*CLHEP::cm,fHalfTargetLength,0,CLHEP::twopi);

  //G4LogicalVolume::G4LogicalVolume(G4VSolid*, G4Material*, const G4String&, G4FieldManager*, G4VSensitiveDetector*, G4UserLimits*, G4bool)’
  logicTarget = new G4LogicalVolume(solidTarget,TargetMater,"Target",0,0,0);
  physiTarget = new G4PVPlacement(0,               // no rotation
				  G4ThreeVector(0,0,25*CLHEP::cm),//0.2  // at (x,y,z)
				  logicTarget,     // its logical volume  
				  "Target",        // its name
				  logicWorld,      // its mother  volume
				  false,           // no boolean operations
				  0);              // copy number
  */
 


  //UNION OF TWO VOLUMES
  if(Ge_detector==true){

  G4cout<<"HPGe detector"<<std::endl;
  #ifdef debug
  std::cout<<TargetMater<<std::endl;
  #endif
  fTargetLength = 6.97*CLHEP::cm;
  G4double fHalfTargetLength = 0.5*fTargetLength;
  G4double frontcyl_TargetLength = 0.88*CLHEP::cm;
  G4double frontcyl_HalfTargetLength = 0.5*frontcyl_TargetLength;
  G4double total_length = fTargetLength + frontcyl_TargetLength;
  G4double R_det = 3.605*CLHEP::cm;
  G4double R_hole = 0.525*CLHEP::cm;
  
  G4Tubs* solidTarget1 = new G4Tubs("Cyl1",0,R_det,frontcyl_HalfTargetLength,0,CLHEP::twopi);
  G4Tubs* solidTarget2 = new G4Tubs("Cyl2",R_hole,R_det,fHalfTargetLength,0,CLHEP::twopi);
  
  //Center and rotation of cylinder with hole
  G4double center_cylhole = ((fTargetLength/2) + frontcyl_TargetLength );
  
  G4ThreeVector zTrans_(0, 0, center_cylhole);
  
  G4double center_nohole_cyl = center_cylhole - fHalfTargetLength - frontcyl_HalfTargetLength;    
  
  G4double centercyl2 = center_cylhole-center_nohole_cyl;

  G4ThreeVector zTrans(0, 0, centercyl2);
    
  solidTarget = new G4UnionSolid("Cylinder1+Cylinder2", solidTarget1, solidTarget2, 0, zTrans);
  
  logicTarget = new G4LogicalVolume(solidTarget,TargetMater,"Target",0,0,0);

  #ifdef debug
  std::cout<<"fTargetLength: "<<G4BestUnit(fTargetLength,"Length")<<" fHalfTargetLength: "<<G4BestUnit(fHalfTargetLength,"Length")<<" Rot y: "<<HPGe_rot_angley<<std::endl;
  std::cout<<"frontcyl_TargetLength: "<<G4BestUnit(frontcyl_TargetLength,"Length")<<" frontcyl_HalfTargetLength: "<<G4BestUnit(frontcyl_HalfTargetLength,"Length")<<std::endl;
  std::cout<<"total_length: "<<G4BestUnit(total_length,"Length")<<std::endl;
  std::cout<<"center_cylhole: "<<G4BestUnit(center_cylhole,"Length")<<std::endl;
  std::cout<<"center_nohole_cyl: "<<G4BestUnit(center_nohole_cyl,"Length")<<std::endl;
  std::cout<<"centercyl2: "<<G4BestUnit(centercyl2,"Length")<<std::endl;
  #endif

  //Create a Rotation Matrix to turn the cyl detector around the y axis
  G4RotationMatrix* Rotation = new G4RotationMatrix(); 
  Rotation->rotateX(0*CLHEP::deg);
  Rotation->rotateY(HPGe_rot_angley*CLHEP::deg); //positive angle is clockwise
  Rotation->rotateZ(0*CLHEP::deg);
  
  physiTarget = new G4PVPlacement(Rotation,               // rotation
				  G4ThreeVector(0,0,center_nohole_cyl),//0.2  // at (x,y,z) //center of cyl2
				  logicTarget,     // its logical volume  
				  "Target",        // its name
				  logicWorld,      // its mother  volume
				  false,           // no boolean operations
				  0);              // copy number

  }

  //******************************************************************//
  //Al Window before Ge Target
  if(Al_Window==true){

    std::cout<<"****** Al Window *****"<<std::endl;
    #ifdef debug
    std::cout<<AlWindowMaterial<<std::endl;
    #endif
    std::cout<<"Rot y: "<<HPGe_rot_angley<<" deg, rmax: "<<AlWindow_rmax_cm<<" cm, half length:  "<<AlWindow_halflength_cm<<" cm , (x,y,z)center cm: ("<<AlWindow_xcenter_cm<<", "<<AlWindow_ycenter_cm<<", "<<AlWindow_zcenter_cm<<")"<<std::endl;

    G4RotationMatrix* RotationAl_Window = new G4RotationMatrix();
    RotationAl_Window->rotateX(0*CLHEP::deg);
    RotationAl_Window->rotateY(HPGe_rot_angley*CLHEP::deg);
    RotationAl_Window->rotateZ(0*CLHEP::deg);
    
    G4Tubs* solidAlWindow = new G4Tubs("AlWindow", 0, AlWindow_rmax_cm*CLHEP::cm, AlWindow_halflength_cm*CLHEP::cm, 0, CLHEP::twopi); //R, half length
    logicAlWindow = new G4LogicalVolume(solidAlWindow,AlWindowMaterial,"AlWindow",0,0,0);
    new G4PVPlacement(RotationAl_Window,G4ThreeVector(AlWindow_xcenter_cm*CLHEP::cm,AlWindow_ycenter_cm*CLHEP::cm,AlWindow_zcenter_cm*CLHEP::cm),logicAlWindow,"AlWindow",logicWorld,false,0);

  }

  //******************************************************************//
  //Polyethylene detector between origin and Ge Target
  if(Poly_detector==true){

    std::cout<<"****** Poly *****"<<std::endl;
    #ifdef debug
    std::cout<<PolyMaterial<<std::endl;
    #endif
    std::cout<<"rmax: "<<Poly_rmax_cm<<" cm, half length:  "<<Poly_halflength_cm<<" cm , (x,y,z)center cm: ("<<Poly_xcenter_cm<<", "<<Poly_ycenter_cm<<", "<<Poly_zcenter_cm<<")"<<std::endl;

    //10cm poly
    G4Tubs* solidPoly = new G4Tubs("PolyAbsorber", 0, Poly_rmax_cm*CLHEP::cm, Poly_halflength_cm*CLHEP::cm, 0, CLHEP::twopi); //R, half length
    logicPoly = new G4LogicalVolume(solidPoly,PolyMaterial,"PolyAbsorber",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(Poly_xcenter_cm*CLHEP::cm,Poly_ycenter_cm*CLHEP::cm,Poly_zcenter_cm*CLHEP::cm),logicPoly,"PolyAbsorber",logicWorld,false,0);

    //18cm poly
    /*G4Tubs* solidPoly = new G4Tubs("PolyAbsorber", 0, 7.1*CLHEP::cm, 9*CLHEP::cm, 0, CLHEP::twopi); //R, half length
    logicPoly = new G4LogicalVolume(solidPoly,PolyMaterial,"PolyAbsorber",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(0,0,10.5*CLHEP::cm),logicPoly,"PolyAbsorber",logicWorld,false,0);*/
  }


  //******************************************************************//
  //KCl detector at origin
  if(KCl_detector==true){
    G4String name, symbol;    
    G4double  z;
    G4double  aK = 39.098*(CLHEP::g / CLHEP::mole); //g/mole
    G4Element* eleK = new G4Element(name="Potassium" ,symbol="K" , z= 19., aK);

    G4double  aCl = 35.453*(CLHEP::g / CLHEP::mole);
    G4Element* eleCl = new G4Element(name="Chlorine" ,symbol="Cl" , z= 17., aCl);
    std::cout<<"Build K and Cl: "<<eleK<<" "<<eleCl<<std::endl;

    G4double density = 1.98*(CLHEP::g / CLHEP::cm3); //g/cm3

    G4int ncomponents;
    G4Material* KCl = new G4Material("KCl",density,ncomponents=2); //2 is n components

    G4int natoms;
    KCl->AddElement(eleK, natoms=1); //atoms per molecule
    KCl->AddElement(eleCl, natoms=1);
    //std::cout<<"Check: "<<KCl<<std::endl;

    //Salt cylinder dimenssions
    G4double Saltcyl_Diameter = 6.35*CLHEP::cm;
    G4double Saltcyl_R = 0.5*Saltcyl_Diameter;
    G4double Saltcyl_TargetLength = 13.97*CLHEP::cm;
    G4double Saltcyl_HalfTargetLength = 0.5*Saltcyl_TargetLength;

    G4Tubs* solidKClTarget = new G4Tubs("KClTarget",0,Saltcyl_R,Saltcyl_HalfTargetLength,0,CLHEP::twopi);
    logicKCl = new G4LogicalVolume(solidKClTarget,KCl,"KClTarget",0,0,0);
    //Cyllinder axis defined by default along the z axis, we need it along y axis, rotate X 90deg
    G4RotationMatrix* rot = new G4RotationMatrix(); 
    rot->rotateX(90.*CLHEP::deg);
    new G4PVPlacement(rot,G4ThreeVector(0*CLHEP::cm,0*CLHEP::cm,0*CLHEP::cm),logicKCl,"KClTarget",logicWorld,false,0);
  }

    //******************************************************************//
    //Add a virtual detector in front of the Ge cylinder
  if(VDplane_STM==true){

    std::cout<<"****** VD *****"<<std::endl;
    std::cout<<"Rot y: "<<HPGe_rot_angley<<" deg, rmax: "<<VD_rmax_cm<<" cm, half length: "<<VD_halflength_cm<<"cm, (x,y,z)center cm: ("<<VD_xcenter_cm<<", "<<VD_ycenter_cm<<", "<<VD_zcenter_cm<<")"<<std::endl;
  
    G4RotationMatrix* RotationVD = new G4RotationMatrix();
    RotationVD->rotateX(0*CLHEP::deg);
    RotationVD->rotateY(HPGe_rot_angley*CLHEP::deg);
    RotationVD->rotateZ(0*CLHEP::deg);
  
    //Circle VD
    G4Tubs* solidplane = new G4Tubs("VacuumTarget", 0, VD_rmax_cm*CLHEP::cm, VD_halflength_cm*CLHEP::cm, 0, CLHEP::twopi); //radius, half z
    //Plane VD
    //G4Box* solidplane = new G4Box("VacuumTarget", 10.5*CLHEP::cm, 20.3*CLHEP::cm, 0.1*CLHEP::cm); //half x, half y, half z
    
    logicVacuum = new G4LogicalVolume(solidplane,Vacuum,"VacuumTarget",0,0,0);
    new G4PVPlacement(RotationVD,G4ThreeVector(VD_xcenter_cm*CLHEP::cm,VD_ycenter_cm*CLHEP::cm,VD_zcenter_cm*CLHEP::cm),logicVacuum,"VacuumTarget",logicWorld,false,0);

    /*
    G4Box* solidplane = new G4Box("VacuumTarget", 9.9*CLHEP::cm, 12.25*CLHEP::cm, 0.1*CLHEP::cm); //half x, half y, half z
    logicVacuum = new G4LogicalVolume(solidplane,Vacuum,"VacuumTarget",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(0,0,21.4*CLHEP::cm),logicVacuum,"VacuumTarget",logicWorld,false,0);
    */
    }

  //------------------------------------------------ 
  // Sensitive detectors
  //------------------------------------------------ 

    
  G4cout<<"construct: detector"<<G4endl;
  G4String trackerChamberSDname = "nipXray/GeSensitiveDetector";
  GeSensitiveDetector* aTrackerSD = new GeSensitiveDetector(trackerChamberSDname);
  G4cout<<"construct: adding detector to SDM"<<G4endl;
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  SDman->AddNewDetector( aTrackerSD );
  if(Ge_detector==true){ logicTarget->SetSensitiveDetector( aTrackerSD ); }
  if(KCl_detector==true){ logicKCl->SetSensitiveDetector( aTrackerSD ); }
  if(Poly_detector==true){ logicPoly->SetSensitiveDetector( aTrackerSD ); }
  if(VDplane_STM==true){ logicVacuum->SetSensitiveDetector( aTrackerSD ); }
  if(Al_Window==true){ logicAlWindow->SetSensitiveDetector( aTrackerSD ); }

  //G4cout<<"construct: setting user steps"<<G4endl;
  //G4double maxStep = 0.5*CLHEP::cm;
  //stepLimit = new G4UserLimits(maxStep);
  //logicTracker->SetUserLimits(stepLimit);
    
 
    
  G4cout<<"construct: returning constructed world"<<G4endl;
  return physiWorld;
}


