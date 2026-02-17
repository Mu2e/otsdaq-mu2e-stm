#include "GeUserDetectorConstruction.hh"
#include "GePhysicsList.hh"
#include "GePrimaryGeneratorAction.hh"
#include "GeRunAction.hh"
#include "GeEventAction.hh"
#include "GeSteppingAction.hh"
//#include "GeSteppingVerbose.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"

#include "G4Timer.hh"
#include "ctime"
#include "cstdio"

//#define debug

int main(int argc,char** argv)
{
  G4Timer myTimer;
  myTimer.Start(); 
  time_t lt = time(NULL);
  #ifdef debug
  G4cout<<"main: beginning time is"<<ctime(&lt)<<G4endl;
  #endif
 
  //Initial seed
  //By default it uses MixMax for the seed
  //Add these lines to get a different seed each time we run geant4
  G4long seed = time(NULL);
  CLHEP::HepRandom::setTheSeed(seed);

  //Add these lines to change from MixMax seed to RanecuEngine
  //  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine());
  //  CLHEP::HepRandom::setTheSeed(124);
  //Add this line to show the initial seed of geant4, it changes for each event
  #ifdef debug
  G4cout << "***********************" << G4endl;
  G4cout << "*** Seed: " << G4Random::getTheSeed() << " ***" << G4endl;
  G4cout << "***********************" << G4endl;
  #endif


  // User Verbose output class
  //
  //G4VSteppingVerbose* verbosity = new GeSteppingVerbose;
  //G4VSteppingVerbose::SetInstance(verbosity);

  // Run manager
  //
  #ifdef debug
  G4cout<<"main: init G4RunManager"<<G4endl;
  #endif
  G4RunManager * runManager = new G4RunManager;

 

  // User Initialization classes (mandatory)
  //
  G4cout<<"main: detector init "<<G4endl;
  GeUserDetectorConstruction* detector = new GeUserDetectorConstruction;
  G4cout<<"main: detector user init"<<G4endl;
  runManager->SetUserInitialization(detector);
  //
  G4cout<<"main: physics loading"<<G4endl;
  G4VUserPhysicsList* physics = new GePhysicsList;
  G4cout<<"main: physics user init"<<G4endl;
  runManager->SetUserInitialization(physics);

  // User Action classes
  //
  G4cout<<"main: generator init "<<G4endl;
  G4VUserPrimaryGeneratorAction* gen_action = new GePrimaryGeneratorAction(detector);
  G4cout<<"main: generator set action "<<G4endl;
  runManager->SetUserAction(gen_action);
  //
  G4cout<<"main: run action init"<<G4endl;
  G4UserRunAction* run_action = new GeRunAction;
  G4cout<<"main: run action user init"<<G4endl;
  runManager->SetUserAction(run_action);
  //
  G4UserEventAction* event_action = new GeEventAction;
  runManager->SetUserAction(event_action);
  //
  GeEventAction* eaction = new GeEventAction;
  runManager->SetUserAction(eaction);

  G4UserSteppingAction* stepping_action = new GeSteppingAction(detector,eaction);
  runManager->SetUserAction(stepping_action);

  // Initialize G4 kernel
  //
  G4cout<<G4endl;
  G4cout<<"main: runManager init"<<G4endl;
  runManager->Initialize();

  //One way of simulating 10 events in 1 run
  // Get the pointer to the User Interface manager
  // G4UImanager * UI = G4UImanager::GetUIpointer();  
  //UI->ApplyCommand("/run/beamOn 10");

  //Another way of simulating n numberOfEvent events in 1 run
  //Number of events in each run
  int numberOfEvent = 1000000;
  //number of runs
  int numberOfRuns = 1;
  for(int i=0;i<numberOfRuns;i++){
    runManager->BeamOn(numberOfEvent);
  }

  /*if (argc!=1) {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    G4cout<<G4endl;
    G4cout<<"main: executing ..."<<G4endl;
    UI->ApplyCommand(command+fileName);
    G4cout<<"main: executing done"<<G4endl;
    } else {
    G4UIsession * session = 0;
    session = new G4UIterminal();
    session->SessionStart();
    delete session;
    }*/
  
  delete runManager;
  //delete verbosity;

  lt = time(NULL);
  printf( "the end time is %s", ctime(&lt));
  myTimer.Stop();
  G4cout<<"Spend time:"<<myTimer<<G4endl;

  return 0;
}
