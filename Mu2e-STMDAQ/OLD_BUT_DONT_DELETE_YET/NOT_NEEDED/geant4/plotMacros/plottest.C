{
  gROOT->Reset();
  gROOT->SetStyle("Plain");
  
  // Draw histos filled by Geant4 simulation 
  //   

  // Open file filled by Geant4 simulation 
  TFile f("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_new.root");

  // Create a canvas and divide it into 2x2 pads
  //  TCanvas* c1 = new TCanvas("c1", "", 20, 20, 1000, 1000);
  //c1->Divide(2,2);
  
  // Get ntuple
  TNtuple* ntupleSteps = (TNtuple*)f.Get("Steps");
  TNtuple* ntupleEvents = (TNtuple*)f.Get("Events");
 
   TH1* heventid = new TH1F("heventid", "EventID each step", 200, -1., 100000.);
   heventid->GetXaxis()->SetTitle("EventID");
   TH1* hedepsteps = new TH1F("hedepsteps", "Edep by electrons in each step", 100, 0., 0.5);
   hedepsteps->GetXaxis()->SetTitle("Edep Step e- (MeV)");
  
  //Define the Ntuples
   //electrons
   //ntupleSteps->Draw("Xpos:Ypos:Zpos:EventID:EdepStep:StepLengthStep:PDGID","PDGID==11","goff");
    ntupleEvents->Draw("EdepEvent:TrackLengthEvent:electrons","electrons>-1","goff");
    //photons and electrons
    ntupleSteps->Draw("Xpos:Ypos:Zpos:EventID:EdepStep:StepLengthStep:PDGID","PDGID>1","goff"); 
    //Long64_t numberofpoints= ntuple->GetEstimate();
    //Double_t rows=ntuple-> GetSelectedRows();

  //Number of total entries in all the events
  std::cout<<ntupleSteps->GetSelectedRows()<<std::endl;
  std::cout<<ntupleEvents->GetSelectedRows()<<std::endl;

  //From the tuple to a pointer
   Double_t *vx  = ntupleSteps->GetVal(0);
   Double_t *vy  = ntupleSteps->GetVal(1);
   Double_t *vz  = ntupleSteps->GetVal(2);
   Double_t *veventid  = ntupleSteps->GetVal(3);
   Double_t *vedepstep  = ntupleSteps->GetVal(4);
   Double_t *vsteplengthstep  = ntupleSteps->GetVal(5);
   Double_t *vPDGID  = ntupleSteps->GetVal(6);
   Double_t *vedepstepgammae  = ntupleSteps->GetVal(7);


   Double_t *vedepevent  = ntupleEvents->GetVal(0);
   Double_t *vtracklengthevent  = ntupleEvents->GetVal(1);
   Double_t *velectrons  = ntupleEvents->GetVal(2);
  
   int eventid=0;

   //uncomment for filling the histogram with the tuple
   for(int i=0;i<ntupleSteps->GetSelectedRows();i++){
     std::cout<<i<<" EventID: "<<*veventid<<std::endl;
      heventid->Fill(*veventid);
      veventid++;
    
     }        

      heventid->Draw("");
   

   //uncomment for plotting the g4 the tuple
   //   ntupleSteps->Draw("EventID");

   /* std::vector<double> EdepStep;
   EdepStep.clear();
      //Number of electrons                                                                        
   for(int i=0;i<ntupleSteps->GetSelectedRows();i++){
     EdepStep.push_back(*vedepstep);
     //	std::cout<<*vedepstep<<std::endl;
     	hedepsteps->Fill(*vedepstep);
	vedepstep++;
      }
    
      hedepsteps->GetYaxis()->SetLabelSize(0.03);
      hedepsteps->GetXaxis()->SetLabelSize(0.03);

      hedepsteps->GetYaxis()->SetTitleSize(0.025);
      hedepsteps->GetXaxis()->SetTitleSize(0.025);
      hedepsteps->GetYaxis()->SetTitleOffset(2);
      hedepsteps->GetXaxis()->SetTitleOffset(2);
      hedepsteps->SetFillStyle( 3001);
      hedepsteps->SetFillColor( kBlue+4);
      hedepsteps->Draw(); 
     */
      //uncomment for plotting the g4 the tuple
   //ntupleSteps->Draw("EdepStep","PDGID>1","goff");    
   // ntupleSteps->Draw("EdepStep");

      c1->SaveAs("3eventid.png");   
   // c1->SaveAs("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_fillhisto.png");
   //c1->SaveAs("100000photons_347keV_0.5600_x-0.6_0.6_y-0.01_0.01_z5_15_0010_Target_tuple.png");
 
}  
