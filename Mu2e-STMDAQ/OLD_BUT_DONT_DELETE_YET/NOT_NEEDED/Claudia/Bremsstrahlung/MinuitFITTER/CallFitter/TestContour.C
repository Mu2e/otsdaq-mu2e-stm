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
  int NP = 2;
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
  int NP = 2;
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




void TestContour()
{
  TFile *infile = new TFile("lines2.root");
  TH1F *h0=(TH1F*)infile->Get("h0");
  TF1 *line0 = new TF1("line0","[0]*x+[1]",0,5);
  line0->SetParameters(-1.0,10.0);
  double fixparam = 8.57422e+01;
  //line0->FixParameter(1,fixparam); //fix parameter 1 to fixparam
  h0->Fit("line0");

  TF1 *fitresult = h0->GetFunction("line0");
  double chi2min = fitresult->GetChisquare();
  double ndf = fitresult->GetNDF();
  double prob = fitresult->GetProb();
  double p0 = fitresult->GetParameter(0);
  double p0_error = p0 + fitresult->GetParError(0);
  double p0_error_nocorr = p0 + 1.15380;
  std::cout<<"p0: "<<p0<<" p0 error: "<<fitresult->GetParError(0)<<std::endl;

  // TMinuit *gMinuit = new TMinuit(); wrong just call gminuit
  TMatrixD matrix0(2,2);
  gMinuit->mnemat(matrix0.GetMatrixArray(),2);
  std::cout<<"Covariance Matrix: "<<std::endl;
  matrix0.Print();

  std::cout<<"The correlation matrix: "<<std::endl;
  gMinuit->mnmatu(1);

  //Draw the contour
  gMinuit->SetErrorDef(1);
  //Minuit generates the contour
  /*TGraph *gr0 = (TGraph *)gMinuit->Contour(100,0,1); //(number of points, parameter 1, parameter 2)*/

  ///Generate the contour myself///
  TGraph *gr0 = new TGraph();
  int nrand = 10000;
  TF1 *fitresult_[nrand];

  int NP = 2;
  TMatrixD sqrtCov( NP, NP );
  double fitpar[2] = {fitresult->GetParameter(0), fitresult->GetParameter(1)};
  corset( matrix0 , sqrtCov );
  std::cout<<"sqrt(Cov): "<<std::endl;
  sqrtCov.Print();

  for (int loop = 0; loop < nrand; ++loop){
    double *values = new double[NP];
    corgen (sqrtCov, values);
    //std::cout<<values[0]<<" "<<values[1]<<std::endl;
    //--sum the mean to the parameters
    for (int i=0;i<NP;i++) {
      values[i]+=fitpar[i];
    }

    //Fit and get chi2
    line0->FixParameter(0, values[0]);
    line0->FixParameter(1, values[1]);
    h0->Fit("line0","Q");
    fitresult_[loop] = h0->GetFunction("line0");
    double chi2 = fitresult_[loop]->GetChisquare();

    //Just fill the contour if the error is in 1sigma, i.e. chi2 is in [chi2min-1, chi2min+1]
    if(abs(chi2)<(chi2min+1)){
      //std::cout<<"chi2: "<<chi2<<std::endl;
      gr0->SetPoint(loop, values[0], values[1]);
    }
    //std::cout<<"loop: "<<loop<<": "<<values[0]<<" "<<values[1]<<" "<<values[2]<<std::endl;
  }
  ///Generate the contour myself///

  gr0->SetLineColor(kRed);
  //gr0->Draw("alp");
  gr0->Draw("ap");
  gr0->GetYaxis()->SetRangeUser(79,92);
  gr0->GetXaxis()->SetRangeUser(-10,-4);
  gr0->GetXaxis()->SetTitle("parameter 0 (slope)");
  gr0->GetYaxis()->SetTitle("parameter 1 (intercept)");
  gr0->SetTitle("1-sigma uncertainties on fit parameters");


  TLine* l1 = new TLine(p0,79.5,p0,92);
  l1->SetLineColor(kBlack);
  l1->Draw("same");

  TLine* l2 = new TLine(p0_error,79.5,p0_error,92);
  l2->SetLineColor(kGreen);
  l2->SetLineStyle(2);
  l2->Draw("same");


  TLine* l3 = new TLine(-10, fixparam, -4.5, fixparam);
  l3->SetLineColor(kBlack);
  l3->Draw("same");


  TLine* l4 = new TLine(p0_error_nocorr,79.5,p0_error_nocorr,92);
  l4->SetLineColor(kOrange);
  l4->SetLineStyle(2);
  l4->Draw("same");

  auto legend1 = new TLegend(0.35,0.6,0.7,0.8);
  legend1->AddEntry(l2,"p0 1#sigma fit error (=2.41)","l");
  legend1->AddEntry(l4,"p0 1#sigma fit error (=1.15) for p1 = 85.74","l");
  legend1->Draw("same");

}
