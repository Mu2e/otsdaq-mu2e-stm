#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

//#include "/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
//#include "/work/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

//1- This code reads the Data, Cov Matrix and Best fit values from a root file
//2- Samples from the covariance matrix to get the new fit parameters as p= pbest fit value + sampling
//3- Calculates the Chi2 (as data(fixed) - f(sampled parameters)) in the whole histogram range (fit_range)
//4- Puts the sampled parameters and the chi2 in histograms
//This is done using the covariance matrices from a chi2 fit and a log likelihood fit 

void CheckFitMethods(int argc, char *argv[], std::string rootname, int FitOption, int nsample, double seed)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  std::cout<<"Seed: "<<seed<<std::endl;
  
  bool binnedloglike = false;
  bool binnedchi2 = true;
  bool store_dataROOT = false;

  std::string name_method;

  if( binnedloglike==true ){
    name_method = "Binned_LogLikelihood";
  }
  if( binnedchi2==true ){
    name_method= "Binned_Chi2";
  }
  
  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  TCanvas* c = new TCanvas("c","c",0,0,1500,800);

  //boost::chrono::high_resolution_clock::time_point t1 ;
  //boost::chrono::high_resolution_clock::time_point t2 ;

  //***********READ DATA***********//

  TFile *infile = new TFile(rootname.c_str());

  //TH1D *hSTM = (TH1D*)infile->Get("hSTM");
  //TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix");  
  //TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars");

  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix1");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars1");
  TH1D *hSTM1 = (TH1D*)infile->Get("hSTM1");

  
  //hSTM1->Draw("");
  //infile->Close();

  int Nentries = hSTM1->GetEntries();
  int Nbins = hSTM1->GetNbinsX();
  std::cout<<"Entries: "<<Nentries<<" Bins: "<<Nbins<<std::endl;



    
  //FitOption doesn't matter we're not going to call minuit minimization here
  //Initialise Minuit Class
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed);

  //Get histogram bin content and errors in fitrange to form the data
  double fit_range[2] = {0.04,2};
  fitter->Histo_Data(fit_range, hSTM1); //Get nbins_fit, _binning, bincontent, bincontent_error, bincenter
  //double binning = fitter->return_binning();

  //Return dof
  unsigned long int dof = fitter->return_dof();
  std::cout<<"Degrees of freedom (ndof): "<<dof<<std::endl;
  //Return number of fit parameters NP
  int NP = fitter->return_NP();
  std::cout<<"Number of fit parameters, NP: "<<NP<<std::endl;
  //Calculate sqrt(cov matrix)
  TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
  TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
  _Covmatrix.Print();
  _BestFitPars.Print();
  TMatrixD sqrtCov( NP, NP );
  fitter->corset( _Covmatrix, sqrtCov );
  //std::cout<<"sqrt(Cov): "<<std::endl;
  //sqrtCov.Print();

  //Recover best fit parameters
  double best_fitpar[NP];

  for( int i = 0 ; i < NP ; i++ ){
    best_fitpar[i] = _BestFitPars[0][i];
  }


  
  //Open ROOT file
  TFile*output;

  std::string rootfile;

  if(store_dataROOT==true) {

    rootfile = "Genfx_seed_"+std::to_string(int(seed))+"_"+name_method+".root";

    output = new TFile(rootfile.c_str(),"recreate");
  }


  //***********PLOT DATA***********// 

  //Chisq histogram
  TH1D *hchisq_ndof;
  TH1D *hchisq;
 
  std::string XAxis_name1 = "#chi^{2}/ndof (" + name_method + ")";
  char* XAxis_name_char1 = const_cast<char*>(XAxis_name1.c_str());

  if(binnedloglike==true){
    hchisq_ndof = new TH1D("hchisq_ndof","", 100, 1.04, 1.09);
    hchisq = new TH1D("hchisq","", 100, 20900, 21800);
  }
  else{
    hchisq_ndof = new TH1D("hchisq_ndof","", 100, 1.0275, 1.0295);
    hchisq = new TH1D("hchisq","", 100, 20540, 20585);
  }

  hchisq_ndof->GetXaxis()->SetTitle(XAxis_name_char1);
  
  std::string XAxis_name2 = "#chi^{2} (" + name_method + ")";
  char* XAxis_name_char2 = const_cast<char*>(XAxis_name2.c_str());
  hchisq->GetXaxis()->SetTitle(XAxis_name_char2);
  

  TH1D *hp[7];

  int dimlimhisto = 14;
  double limhisto[dimlimhisto];
  int k =0;
  
  for(int i = 0 ; i < NP; i++){
    limhisto[k] = best_fitpar[i] - 0.05*best_fitpar[i];
    limhisto[k+1] = best_fitpar[i] + 0.05*best_fitpar[i];
    if(i==1){
      limhisto[k] = best_fitpar[i] - 0.0003*best_fitpar[i];
      limhisto[k+1] = best_fitpar[i] + 0.0003*best_fitpar[i];
    }
    if(i==5){
      limhisto[k] = best_fitpar[i] - 0.005*best_fitpar[i];
      limhisto[k+1] = best_fitpar[i] + 0.005*best_fitpar[i];
    }

    k=k+2;
  }
  

  hp[0] = new TH1D("hp0","", 100, limhisto[0], limhisto[1]);
  hp[1] = new TH1D("hp1","", 100, limhisto[2], limhisto[3]);
  hp[2] = new TH1D("hp2","", 100, limhisto[4], limhisto[5]);
  hp[3] = new TH1D("hp3","", 100, limhisto[6], limhisto[7]);
  hp[4] = new TH1D("hp4","", 100, limhisto[8], limhisto[9]);
  hp[5] = new TH1D("hp5","", 100, limhisto[11], limhisto[10]); //since negative parameters
  hp[6] = new TH1D("hp6","", 100, limhisto[12], limhisto[13]);


  for(int i = 0 ; i < 7 ; i++){
    std::string name = "p" + std::to_string(i);
    char* name_char = const_cast<char*>(name.c_str());
    hp[i]->GetXaxis()->SetTitle(name_char);
  }

  TPad *pad1 = new TPad("","",0,0.66,0.33,1);
  TPad *pad2 = new TPad("","",0.33,0.66,0.66,1);
  TPad *pad3 = new TPad("","",0.66,0.66,1,1);
  TPad *pad4 = new TPad("","",0,0.33,0.33,0.66);
  TPad *pad5 = new TPad("","",0.33,0.33,0.66,0.66);
  TPad *pad6 = new TPad("","",0.66,0.33,1,0.66);
  TPad *pad7 = new TPad("","",0,0,0.33,0.33);
  TPad *pad8 = new TPad("","",0.33,0,0.66,0.33);
  TPad *pad9 = new TPad("","",0.66,0,1,0.33);
  
  pad1->Draw();
  pad2->Draw();
  pad3->Draw();
  pad4->Draw();
  pad5->Draw();
  pad6->Draw();
  pad7->Draw();
  pad8->Draw();
  pad9->Draw();
  
 //***********START CODE***********//

  //t1 = boost::chrono::high_resolution_clock::now();
  //Initialise S+B function 
  //Next 3 lines are required just if we calculate the chi2 using the integral instead of the bin center (much slower)
  std::string  func_name = "fSB";
  char* char_func_name = const_cast<char*>(func_name.c_str());
  fitter->Init_SBfunc(char_func_name, fit_range);

  double *values = new double[NP];

  for (int loop = 0; loop < nsample; ++loop){

    std::cout<<"ITERATION: "<<loop<<std::endl;
    fitter->corgen( sqrtCov, values , NP);

    //--sum the mean to the parameters
    for (int i = 0; i < NP ; i++) {
      values[i]+=best_fitpar[i];

      hp[i]->Fill( values[i] );
    }
    std::cout<<"Sampled values: "<<values[0]<<" "<<values[1]<<" "<<values[2]<<" "<<values[3]<<" "<<values[4]<<" "<<values[5]<<" "<<values[6]<<std::endl;

    //Initialise S+B function 
    //Next line is required just if we calculate the chi2 using the integral instead of the bin center
    fitter->SetPar_SBfunc( values );

    //Call the chi2 function
    double chisq = 0;
    int npar = 0;
    double* gin = 0;
    
    //BE CAREFUL! If sampling from the unbinned covariance matrix the parameters have to be multiplied by: data_size * bin_width
    /*if(unbinned){
    values[2] = values[2]*datasize*binning;
    values[5] = values[5]*datasize*binning;
    values[6] = values[6]*datasize*binning;
    }*/

    fitter->MinuitfcnBinnedChisq(npar, gin, chisq, values, 0);

    std::cout<<"chi2: "<<chisq<<std::endl;
    double chisq_ndof = chisq / dof;
    std::cout<<"chi2/dof: "<<chisq_ndof<<std::endl;

    //Fill histogram with chi2 obtained
    hchisq->Fill( chisq ); 
    hchisq_ndof->Fill( chisq_ndof );
  }//loop

  //t2 = boost::chrono::high_resolution_clock::now();

  //std::cout<< "Computing time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;


  if(store_dataROOT==true) {
    for (int i = 0; i < NP ; i++) {
      std::string  hparameters_name = "hp"+std::to_string(i);
      char* char_hparameters_name = const_cast<char*>(hparameters_name.c_str());
      output->WriteObject(hp[i], char_hparameters_name);
    }
  }



  TPaveStats *stat[9];

  pad1->cd();
  hp[0]->Draw("HIST");
  hp[0]->SetFillColor(kViolet);
  hp[0]->SetLineColor(kBlack);
  hp[0]->SetFillStyle(3001);
  gPad->Update();
  stat[0] = (TPaveStats*)hp[0]->FindObject("stats");
  stat[0]->SetY1NDC(.6);
  stat[0]->SetY2NDC(.9);
  stat[0]->SetX1NDC(0.63);
  stat[0]->SetX2NDC(0.9);
  stat[0]->SetTextColor(kBlack);
  stat[0]->Draw("same");
  
  pad2->cd();
  TAxis *X = hp[1]->GetXaxis();
  X->SetNdivisions(5,3,0);
  
  hp[1]->SetFillColor(kGreen);
  hp[1]->SetLineColor(kBlack);
  hp[1]->SetFillStyle(3001);
  hp[1]->Draw("HIST");
  gPad->Update();
  stat[1] = (TPaveStats*)hp[1]->FindObject("stats");
  stat[1]->SetY1NDC(.6);
  stat[1]->SetY2NDC(.9);
  stat[1]->SetX1NDC(0.63);
  stat[1]->SetX2NDC(0.9);
  stat[1]->SetTextColor(kBlack);
  stat[1]->Draw("same");
  
  pad3->cd();
  hp[2]->SetFillColor(kRed);
  hp[2]->SetLineColor(kBlack);
  hp[2]->SetFillStyle(3001);
  hp[2]->Draw("HIST");
  gPad->Update();
  stat[2] = (TPaveStats*)hp[2]->FindObject("stats");
  stat[2]->SetY1NDC(.6);
  stat[2]->SetY2NDC(.9);
  stat[2]->SetX1NDC(0.63);
  stat[2]->SetX2NDC(0.9);
  stat[2]->SetTextColor(kBlack);
  stat[2]->Draw("same");

  pad4->cd();
  hp[3]->SetFillColor(kYellow);
  hp[3]->SetLineColor(kBlack);
  hp[3]->SetFillStyle(3001);
  hp[3]->Draw("HIST");
  gPad->Update();
  stat[3] = (TPaveStats*)hp[3]->FindObject("stats");
  stat[3]->SetY1NDC(.6);
  stat[3]->SetY2NDC(.9);
  stat[3]->SetX1NDC(0.63);
  stat[3]->SetX2NDC(0.9);
  stat[3]->SetTextColor(kBlack);
  stat[3]->Draw("same");


  pad5->cd();
  hp[4]->SetFillColor(kBlue);
  hp[4]->SetLineColor(kBlack);
  hp[4]->SetFillStyle(3001);
  hp[4]->Draw("HIST");
  gPad->Update();
  stat[4] = (TPaveStats*)hp[4]->FindObject("stats");
  stat[4]->SetY1NDC(.6);
  stat[4]->SetY2NDC(.9);
  stat[4]->SetX1NDC(0.63);
  stat[4]->SetX2NDC(0.9);
  stat[4]->SetTextColor(kBlack);
  stat[4]->Draw("same");

  
  pad6->cd();
  hp[5]->SetFillColor(kBlack);
  hp[5]->SetLineColor(kBlack);
  hp[5]->SetFillStyle(3001);
  hp[5]->Draw("HIST");
  gPad->Update();
  stat[5] = (TPaveStats*)hp[5]->FindObject("stats");
  stat[5]->SetY1NDC(.6);
  stat[5]->SetY2NDC(.9);
  stat[5]->SetX1NDC(0.63);
  stat[5]->SetX2NDC(0.9);
  stat[5]->SetTextColor(kBlack);
  stat[5]->Draw("same");


  pad7->cd();
  hp[6]->SetFillColor(kOrange-3);
  hp[6]->SetLineColor(kBlack);
  hp[6]->SetFillStyle(3001);
  hp[6]->Draw("HIST");
  gPad->Update();
  stat[6] = (TPaveStats*)hp[6]->FindObject("stats");
  stat[6]->SetY1NDC(.6);
  stat[6]->SetY2NDC(.9);
  stat[6]->SetX1NDC(0.63);
  stat[6]->SetX2NDC(0.9);
  stat[6]->SetTextColor(kBlack);
  stat[6]->Draw("same");

  pad8->cd();
  hchisq_ndof->SetFillColor(kPink-8);
  hchisq_ndof->SetLineColor(kBlack);
  hchisq_ndof->SetFillStyle(3001);
  hchisq_ndof->Draw("HIST");
  gPad->Update();
  stat[7] = (TPaveStats*)hchisq_ndof->FindObject("stats");
  stat[7]->SetY1NDC(.6);
  stat[7]->SetY2NDC(.9);
  stat[7]->SetX1NDC(0.63);
  stat[7]->SetX2NDC(0.9);
  stat[7]->SetTextColor(kBlack);
  stat[7]->Draw("same");


  pad9->cd();
  hchisq->SetFillColor(kGray);
  hchisq->SetLineColor(kBlack);
  hchisq->SetFillStyle(3001);
  hchisq->Draw("HIST");

  gPad->Update();
  stat[8] = (TPaveStats*)hchisq->FindObject("stats");
  stat[8]->SetY1NDC(.6);
  stat[8]->SetY2NDC(.9);
  stat[8]->SetX1NDC(0.63);
  stat[8]->SetX2NDC(0.9);
  stat[8]->SetTextColor(kBlack);
  stat[8]->Draw("same");

  std::string nameplot_png = "Genfx_seed_"+std::to_string(int(seed))+"_"+name_method+".png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
  std::string nameplot_pdf = "Genfx_seed_"+std::to_string(int(seed))+"_"+name_method+".pdf";
  char* char_nameplot_pdf = const_cast<char*>(nameplot_pdf.c_str());

  c->Print(char_nameplot_png);
  //c->Print(char_nameplot_pdf);

  if(store_dataROOT==true) { output->Close(); }

  //***********END CODE***********//

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
}




int main(int argc, char *argv[]) {

  std::string rootname = argv[1];
  int FitOption = std::atoi(argv[2]);
  //Sample the cov matrix nsample times 
  int nsample = std::atoi(argv[3]);
  double seed = std::stod(argv[4]);

  CheckFitMethods(argc, argv, rootname, FitOption, nsample, seed);

  exit(0);
  return 0;
}
