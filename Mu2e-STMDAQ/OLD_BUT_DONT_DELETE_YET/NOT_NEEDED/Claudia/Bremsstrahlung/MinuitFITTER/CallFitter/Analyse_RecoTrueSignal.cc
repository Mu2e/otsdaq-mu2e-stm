#include "TApplication.h"
#include "TRootCanvas.h"

//#include "/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
//#include "/work/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

int main (int argc, char *argv[]) {

  std::string input_path = argv[1];

  bool store_ROOTfile = true;
  bool plot_1stCovmatrix_element = true;

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);

  TCanvas* c = new TCanvas("c");
  
  int Njobs = 20;
  double mean_E = 0.347;
  double sigma = 0.002;
  double frange[2] = { 0.04, 2 };

  int backsize = 3;
  //double back_rangesigma[backsize] = { 2.5, 3.5, 4.5 };
  double back_rangesigma[backsize] = { 1.5, 1.75, 2 };

  double TrueSignal_, RecoSignal_;
  double TrueBack_[backsize], RecoBack_[backsize];

  //Integral ranges
  double recobrems[backsize];
  double trueSB[backsize];
  double truebrems[backsize];
  double frange_sigma[backsize][2];
  std::stringstream sigmarange[backsize];
  
  for( int i = 0 ; i < backsize ; i++) {
    frange_sigma[i][0] = mean_E-(back_rangesigma[i]*sigma);
    frange_sigma[i][1] = mean_E+(back_rangesigma[i]*sigma);

    sigmarange[i] << std::fixed << std::setprecision(2) << back_rangesigma[i];
  }


  //Output File
  std::string outrootname;
  TFile* outfile;
  TTree* SBtree;

  if(store_ROOTfile==true){
    outrootname = "Signal_BackgroundTrueReco_"+sigmarange[0].str()+"_"+sigmarange[1].str()+"_"+sigmarange[2].str()+"sigma.root"; 
    outfile = new TFile(outrootname.c_str(),"recreate");
  
    //Output tree
    SBtree = new TTree("SBtree", "SBtree");
    SBtree->Branch("TrueSignal_", &TrueSignal_);
    SBtree->Branch("RecoSignal_", &RecoSignal_);
 

    for( int k = 0; k < backsize; k++ ) {

      std::string branch_nameTrue = "TrueBack_"+sigmarange[k].str()+"sigma";
      std::string branch_nameReco = "RecoBack_"+sigmarange[k].str()+"sigma";
      char* char_branch_nameTrue = const_cast<char*>(branch_nameTrue.c_str());
      char* char_branch_nameReco = const_cast<char*>(branch_nameReco.c_str());
      SBtree->Branch(char_branch_nameTrue, &TrueBack_[k]);
      SBtree->Branch(char_branch_nameReco, &RecoBack_[k]);
    }
  }
  
  //Plot cov matrix
  double sigmax[2] = {750, 1100};
  double sigmay[2] = {0, 80};
  TGraph *graphsigma = new TGraph (2,sigmax,sigmay);
  graphsigma->SetMarkerStyle(1);
  TH1D* covMatrixHisto;

  if(plot_1stCovmatrix_element==true){
     covMatrixHisto = new TH1D ("Sigma histogram", "", 100, sigmax[0], sigmax[1]);
  }




  //Input File
  std::string rootname;
  std::vector<double> *TrueSignal=0, *RecoSignal=0;
  
  MinuitFitter *fitter = new MinuitFitter();
  int NP = fitter->return_NP();
  int NPbrems = fitter->return_NPbrems();
  int NPsignal = fitter->return_NPsignal();
  TH1D *hSTM;
  TMatrixD *BestFitPars;

  std::string  Bfunc_name = "Bfunc";
  char* char_Bfunc_name = const_cast<char*>(Bfunc_name.c_str());
  fitter->Init_Bremsfunc(char_Bfunc_name, frange);

  for(int i = 0 ; i < Njobs ; i++) {

    //rootname = input_path+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+".root";
    //rootname = input_path+"/BinnedLoglike_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root"; 
    //rootname = input_path+"/BinnedChi2_NOIntegral_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    //rootname = input_path+"/UnbinnedLogLike_1.00kHz_TimeSim_2000s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    //rootname = input_path+"/BinnedLoglike_NOIntegral_50.00kHz_TimeSim_2000s_seed_0_0.0020MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root";
    rootname = input_path+"/UnbinnedLogLike_1.00kHz_TimeSim_10s_seed_0_0.0010MeV_50Runs_Job_"+std::to_string(i)+"_Background_ShapeNominal.root"; 
    
    std::cout<<"Input file: "<<rootname<<std::endl;
    
    TFile *infile = new TFile(rootname.c_str());

    //Read Tree
    TTree *treeread=(TTree*)infile->Get("Signaltree");
    treeread->SetBranchAddress("TrueSignal",&TrueSignal);
    treeread->SetBranchAddress("RecoSignal",&RecoSignal);

    treeread->GetEntry(0);
    int nexperiments = TrueSignal->size();
    std::cout<<"1 vector: "<<treeread->GetEntries()<<" of entries: "<<nexperiments<<std::endl;    
    
    for( int j = 0 ; j <  nexperiments; j++) {

      std::cout<<""<<std::endl;
      std::cout<< "Experiment: " << j+1 << std::endl;

      //Covariance Matrix
      std::string str_covmatrix = "Covmatrix"+std::to_string(j+1);
      char* char_covmatrix = const_cast<char*>(str_covmatrix.c_str());
      TMatrixD *Covmatrix = (TMatrixD*)infile->Get(char_covmatrix);
      TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
      
      //Get S+B histogram
      std::string str_hSTM = "hSTM"+std::to_string(j+1);
      char* char_hSTM = const_cast<char*>(str_hSTM.c_str());
      hSTM = (TH1D*)infile->Get(char_hSTM);
      double bin_width = hSTM->GetBinWidth(0);

      //Best Fit Parameters
      std::string str_BestFitPars = "BestFitPars"+std::to_string(j+1);
      char* char_BestFitPars = const_cast<char*>(str_BestFitPars.c_str());
      BestFitPars = (TMatrixD*)infile->Get(char_BestFitPars);
      TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
      std::cout<<"Opened: "<<str_hSTM<<" "<<str_BestFitPars<<std::endl;
      
      double *best_fitpar = new double[NPbrems];

      for( int k = 0 ; k < NPbrems ; k++ ){
	best_fitpar[k] = _BestFitPars[0][k+NPsignal];
	//std::cout<<k<<" "<<k+NPsignal<<" "<<_BestFitPars[0][k+NPsignal]<<std::endl;
      }

      //Set B function parameters and return it
      fitter->SetPar_Bremsfunc(best_fitpar);
      TF1* fbrems = fitter->return_fbrems();

      for( int k = 0; k < backsize; k++ ) {
	//Integrate this function in range to get Reco background
	recobrems[k] = fbrems->Integral( frange_sigma[k][0], frange_sigma[k][1] ) / bin_width;
	//Integrate this histogram in range to get True background + signal
	trueSB[k] = hSTM->Integral(hSTM->FindFixBin(frange_sigma[k][0]), hSTM->FindFixBin(frange_sigma[k][1]), "");
	truebrems[k] = trueSB[k] - TrueSignal->at(j);
      
	//Fill Branches
	TrueBack_[k] = truebrems[k];
	RecoBack_[k] = recobrems[k]; 

	std::cout<<"---- True Background: " << sigmarange[k].str() << "sigma: " << TrueBack_[k] <<std::endl;
	std::cout<<"Reco Background: " << sigmarange[k].str() << "sigma: " << RecoBack_[k] <<std::endl;

      }

      //Fill signal branch
      TrueSignal_ = TrueSignal->at(j);
      //Fill reco branch
      RecoSignal_ = RecoSignal->at(j);

      if(store_ROOTfile==true){
	SBtree->Fill();
      }

      if(plot_1stCovmatrix_element==true){
	double reco_sigma = sqrt(_Covmatrix[0][0]) / bin_width;
	double accuracy = reco_sigma / RecoSignal_;
	std::cout<<"Cov matrix element: "<<_Covmatrix[0][0]<<" Reco sigma: "<<reco_sigma<<" Accuracy: "<<accuracy<<std::endl;
	covMatrixHisto->Fill(reco_sigma);
      }


      std::cout<< "---- True Signal: " << TrueSignal_ << ", Reco Signal: " << RecoSignal_ << std::endl;
    }//nexperiments

    infile->Close();

  }//njobs

  
  if(store_ROOTfile==true){
    outfile->Write();
    outfile->Close();
  }

  
  if(plot_1stCovmatrix_element==true){
    TPaveStats *stat[1];
    covMatrixHisto->SetFillColor(kRed-3);
    covMatrixHisto->SetLineColor(kBlack);
    covMatrixHisto->SetFillStyle(3001);
    covMatrixHisto->Draw("HIST");

    gPad->Update();
    stat[0] = (TPaveStats*)covMatrixHisto->FindObject("stats");
    stat[0]->SetY1NDC(.74);
    stat[0]->SetY2NDC(.91);
    stat[0]->SetX1NDC(0.2);
    stat[0]->SetX2NDC(0.45);
    stat[0]->SetTextSize(0.043);
    stat[0]->SetTextColor(kRed-3);

    graphsigma->Draw("ap");
    graphsigma->GetXaxis()->SetTitle("#sigma_{Reco Signal}");
    graphsigma->GetYaxis()->SetTitle("Entries");
    covMatrixHisto->Draw("HIST,same");
    stat[0]->Draw("same");
  }


#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif

}
