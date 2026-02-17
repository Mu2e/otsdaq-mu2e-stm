#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <iomanip>

#include "TGraph.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TSpectrum.h"
#include "TLegend.h"
#include "TRandom3.h"
#include "TLine.h"
#include "TPaveStats.h"
#include "TMinuit.h"
#include "TF2.h"

void corset( const TMatrixD& V, TMatrixD& C ){
  // calculate sqrt(V) as lower diagonal matrix
  // if we dont know the number of params
  //int NP = V.GetNcols();
  // if we know number of params
  int NP = 4;
  for( int i = 0; i < NP; ++i ) {
    for( int j = 0; j < NP; ++j ) {
      C[i][j] = 0;
    }
  }
  for( int j = 0; j < NP; ++j ) {
    // diagonal terms first
    double Ck = 0;
    for( int k = 0; k < j; ++k ) {
      Ck += C[j][k] * C[j][k];
    } // k
    C[j][j] = sqrt( fabs( V[j][j] - Ck ) );
    // off-diagonal terms
    for( int i = j+1; i < NP; ++i ) {
      Ck = 0;
      for( int k = 0; k < j; ++k ) {
	Ck += C[i][k] * C[j][k];
      } //k
      C[i][j] = ( V[i][j] - Ck ) / C[j][j];
    }// i
  } // j
}

void corgen( const TMatrixD& C, double *x){
  //gRandom->SetSeed(0);
  //std::cout<<"seed: "<<gRandom->GetSeed()<<std::endl;
  int NP = 4;
  double *z =new double[NP];
  // np random numbers from unit Gaussian
  for( int i = 0; i < NP; ++i )
    z[i] = gRandom->Gaus( 0.0, 1.0 );
  // fill values
  for( int i = 0; i < NP; ++i ) {
    x[i] = 0;
    for( int j = 0; j <= i; ++j ) {
      x[i] += C[i][j] * z[j];
    } // j
  } // i
  delete [] z; //free the array
}




void SampleCovMatrix(std::string rootname, double meanE_MeV, double xmin, double xmax)
{

  gROOT->SetStyle("ATLAS");

  gStyle->SetOptStat(1110);
  gStyle->SetPadRightMargin(0.1);
  gStyle->SetPadLeftMargin(0.15);
  
  TCanvas *c1 = new TCanvas("");
  
  TFile *infile = new TFile(rootname.c_str());
  TMatrixD *Covmatrix = (TMatrixD*)infile->Get("Covmatrix");
  TMatrixD *BestFitPars = (TMatrixD*)infile->Get("BestFitPars");
  TMatrixD *BestFitParsErrors = (TMatrixD*)infile->Get("BestFitParsErrors");

  int NP = 4;
  TMatrixD _Covmatrix(NP,NP,Covmatrix->GetMatrixArray(),"");
  TMatrixD _BestFitPars(1,NP,BestFitPars->GetMatrixArray(),"");
  double best_fitpar[NP];
  
  for( int i = 0 ; i < NP ; i++ ){
    best_fitpar[i] = _BestFitPars[0][i];
    std::cout<<"Best fit parameters "<<i<<": "<<best_fitpar[i]<<std::endl;
  }
  
  TF1 *Acc_function = new TF1("Acc_function", "[0] + (1./([1]*x)) - sqrt( (1./([2]*x)) + (1./([3]*x*x)) )", xmin, xmax);
  Acc_function->SetParameters(best_fitpar[0],best_fitpar[1],best_fitpar[2],best_fitpar[3]);
  double mean_acc = Acc_function->Eval(meanE_MeV);
  std::cout<<"Eval Mean Acceptance: "<<mean_acc<<std::endl;

  //double mean_hist = 7.005e-8;
  //double mean_hist = 1;  
  double mean_hist = 1.19634e-7;
  
  //Sample cov matrix
  std::cout<<"Covariance Matrix: "<<std::endl;
  Covmatrix->Print();

  BestFitPars->Print();


  //double low_x = mean_acc - mean_acc*0.25;
  //double high_x = mean_acc + mean_acc*0.25;

  double low_x = (mean_acc - mean_acc*0.25)/mean_acc;
  double high_x = (mean_acc + mean_acc*0.25)/mean_acc;
   
  TH1D *hAccError = new TH1D("hAccError","", 100, low_x, high_x);
  //hAccError->GetXaxis()->SetTitle("Mean Flash Acceptance VD10 - VD89,90");
hAccError->GetXaxis()->SetTitle("Sampled / Mean Flash Acceptance VD10 - VD89,90");
  hAccError->GetYaxis()->SetTitle("# of Cov Matrix sampling");
  int nrand = 10000;

  TMatrixD sqrtCov( NP, NP );
  corset( _Covmatrix , sqrtCov );
  std::cout<<"sqrt(Cov): "<<std::endl;
  sqrtCov.Print();
  

  for (int loop = 0; loop < nrand; ++loop){
    double *values = new double[NP];
    corgen (sqrtCov, values);
    //std::cout<<values[0]<<" "<<values[1]<<std::endl;
    //--sum the mean to the parameters
    for (int i=0;i<NP;i++) {
      values[i]+= best_fitpar[i];
    }

    //Evaluate Acceptance
    Acc_function->SetParameters(values[0],values[1],values[2],values[3]);
    double sampled_mean_acc = Acc_function->Eval(meanE_MeV);
    //std::cout<<loop<<" sampled acceptance at "<<meanE_MeV<<" MeV: "<<mean_acc<<std::endl;
    hAccError->Fill(sampled_mean_acc/mean_hist);
  }

  double sampled_mean = hAccError->GetMean();

  std::cout<<"Sampled mean: "<<sampled_mean<<std::endl;
  
  hAccError->Draw("");

  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.66);
  ps->SetX2NDC(0.86);
  ps->SetY1NDC(0.77);
  ps->SetY2NDC(0.9);
  ps->SetTextSize(0.032);
  ps->SetName("mystats");
    
  hAccError->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  
  c1->Modified();

  gPad->RedrawAxis();

  
  hAccError->SetFillColor(kBlue-9);
  hAccError->SetLineColor(kBlue-9);
  hAccError->SetFillStyle(3001);
  hAccError->Draw();

  std::cout<<"Mean acceptance: "<<hAccError->GetMean()<<" +- "<<hAccError->GetMeanError()<<std::endl;
  std::stringstream stream_meanE;
  stream_meanE << std::fixed << std::setprecision(4) << meanE_MeV;

  std::string str_latex1 = "Mean E = "+stream_meanE.str()+" MeV";
  char* char_latex1 = const_cast<char*>(str_latex1.c_str());
  TLatex latex1;
  latex1.DrawLatexNDC(.17,.85,char_latex1);

  
  int pos1 = rootname.find("Bestfits_")+9;
  int pos2 = rootname.find("cut");
  int diff = pos2-pos1;
  std::string nameplot_png = "MeanFlashaccSampledCovMatrix_VD10cut_"+rootname.substr(pos1, diff)+"_range"+std::to_string(int(xmin))+"_"+std::to_string(int(xmax))+"_divmean.png";
  char* char_nameplot_png = const_cast<char*>(nameplot_png.c_str());
  std::cout<<"Name plot: "<<nameplot_png<<std::endl;
  c1->Print(char_nameplot_png);


}
