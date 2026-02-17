#include "TApplication.h"
#include "TRootCanvas.h"
#include <boost/chrono.hpp>

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

//1 - Read covariance matrix from calibration (grade 5 poly)
//2 - Sample cov matrix 1,000 times.
//3 - Generate new calibrations with sampled values.
//4 - Plot it.

void SampleCalibration(int argc, char *argv[], std::string rootname, int FitOption, double seed, int nsample, std::string outpath)
{

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  std::cout<<"Executing SampleCalibration.cc"<<std::endl;
  std::cout<<"Seed: "<<seed<<std::endl;

  bool store_dataROOT = false; //if this is true it doesn't plot the histogram in screen
  bool plot_data = true;
  
  gROOT->SetStyle("ATLAS");
  gStyle->SetOptStat(1111);
  TCanvas* c = new TCanvas("c");
  c->SetGrid();
  
  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;

  //***********PLOT************************************//
  double xx1[2]={-614, -604};
  double yy1[2]={344, 352};

  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->SetTitle("");
  graph1->GetYaxis()->SetTitle("E [keV]");
  graph1->GetXaxis()->SetTitle("ADC Counts");
  graph1->GetXaxis()->SetTitleOffset(0.9);
  graph1->SetMarkerStyle(1);
  graph1->SetMaximum(yy1[1]);
  graph1->SetMinimum(yy1[0]);
  graph1->Draw("ap");
  
  TF1*Lineardependence = new TF1("Lineardependence", "x*(-0.57)", xx1[0], xx1[1]);
  Lineardependence->SetLineColor(kBlue);

  //***********READ DATA********************************//
  
  std::cout<<rootname<<std::endl;

  TFile *infile = new TFile(rootname.c_str());
    
  //***************GET HISTOGRAM AND BEST FIT*****************//
  
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars");
  TMatrixD *BestFitParsErrors = (TMatrixD*)infile->Get("BestFitParsErrors");
  
  //***********CREATE ROOT OUTPUT*********************//   
    
  TFile*output;
  std::string rootoutfile;
  if(store_dataROOT==true) {
    
    rootoutfile = outpath+"/SampledCalibrations_.root";
    output = new TFile(rootoutfile.c_str(),"recreate");
  }

 //***********START CODE***************************//

  //Initialise Minuit Class
  MinuitFitter *fitter = new MinuitFitter(FitOption, seed); //doesn't matter

  //Number of fit parameters for a grade 5 poly
  int NP_ = 6;
  
  //Calculate sqrt(cov matrix)
  TMatrixD _Covmatrix(NP_,NP_,Covmatrix->GetMatrixArray(),"");
  TMatrixD _BestFitPars(1,NP_,BestFitPars->GetMatrixArray(),"");
  TMatrixD _BestFitParsErrors(1,NP_,BestFitParsErrors->GetMatrixArray(),"");
  std::cout<<"Covariance matrix to sample...:"<<std::endl;
  _Covmatrix.Print();
  std::cout<<"Best fit values...:"<<std::endl;
  _BestFitPars.Print();
  std::cout<<"Errors of the initial best fit values...:"<<std::endl;
  _BestFitParsErrors.Print();
    
  //Start the sampling
  TMatrixD sqrtCov( NP_, NP_ );
  
  fitter->corset( _Covmatrix, sqrtCov );
  //std::cout<<"sqrt(Cov): "<<std::endl;
  //sqrtCov.Print();


  //Recover best fit parameters
  double best_fitpar[NP_];
 
  for( int i = 0 ; i < NP_ ; i++ ){
    best_fitpar[i] = _BestFitPars[0][i];
  }
  
  //********************INIT LOOP OF SAMPLING***********//
    
  TF1* fgenerated[nsample];
  
  //Sampled values for Calibration
  double *sampvalues = new double[NP_];

  double* p_minuit;
  double* perr_minuit;

  for (int loop = 0; loop < nsample; loop++){

    std::cout<<""<<std::endl;
    std::cout<<""<<std::endl;
    std::cout<<"ITERATION: "<<loop<<std::endl;
    t1 = boost::chrono::high_resolution_clock::now();

    fitter->corgen( sqrtCov, sampvalues, NP_ );
    
    //--sum the mean to the parameters for all Sampled values
    for (int i = 0; i < NP_ ; i++) {
      sampvalues[i]+=best_fitpar[i];
      std::cout<<"Sampled values for calib "<<i<<": "<<sampvalues[i]<<std::endl;
    }
    
    //Initialise Background function
    std::string  func_name = "fC"+std::to_string(loop);
    char* char_func_name = const_cast<char*>(func_name.c_str());
    
    fgenerated[loop]= new TF1(char_func_name,"[0]+[1]*x+[2]*x*x+[3]*x*x*x+[4]*x*x*x*x+[5]*x*x*x*x*x",xx1[0],xx1[1]);
    //fgenerated[loop]->SetNpx(300000);

    for(int i = 0 ; i < NP_; i++){
      //subtract 1keV in the vertical axis
      if(i==0){ fgenerated[loop]->SetParameter(i,sampvalues[i]-1.03); }
      else{
	fgenerated[loop]->SetParameter(i,sampvalues[i]);
      }
    }
    /*
      if(store_dataROOT==true) {
      output->WriteObject(fgenerated[loop], char_func_name);
      }
    */
    
  }//loop
  
  
  
  t2 = boost::chrono::high_resolution_clock::now();
  std::cout<< "Computing time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;
  

  if( plot_data ==true ) {

    for(int i = 0 ; i <nsample ; i++){
      fgenerated[i]->SetLineColor(kYellow-9);
      fgenerated[i]->SetLineWidth(4);
      fgenerated[i]->Draw("same,l");
    }
  }

  Lineardependence->Draw("same,l");
  
  auto legend = new TLegend(0.5,0.65,0.9,0.9);
  legend->AddEntry(Lineardependence,"E=-0.57 ADC","l");
  legend->AddEntry(fgenerated[0],"Calibration Uncertainty","l");
  legend->Draw("same");
  
   std::string nameplot_png = "SampledPoly5Calibration.png";
   char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
   
   c->Print(char_nameplot_png);
   
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
  double seed = std::stod(argv[3]);
  int nsample = std::atoi(argv[4]);
  std::string outpath = argv[5];
  
  SampleCalibration(argc, argv, rootname, FitOption, seed, nsample, outpath);


  return 0;
}
