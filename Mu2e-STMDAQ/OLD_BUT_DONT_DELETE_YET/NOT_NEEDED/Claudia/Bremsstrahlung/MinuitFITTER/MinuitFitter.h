#ifndef MINUITFITTER_H
#define MINUITFITTER_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#include <vector>
#include <algorithm>
#include <string>
#include <utility> // std::pair 
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream 
#include <iomanip>
#include <math.h>

#include "TH1F.h"
#include "TGraph.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"
#include "TMinuit.h"
#include "TRandom3.h"
#include "TRandom.h"
#include "TMatrixD.h"
#include "Math/Util.h"  // for safe log(x)
#include "TStyle.h"
#include "TPad.h"
#include "TPaveStats.h"
#include "TTree.h"
#include "TBranch.h"
#include "TSystem.h"

//#include "STMDAQ-TestBeam/utils/Random.hh"
//#include "STMDAQ-TestBeam/utils/Logger.hh"

#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/utils/Random.hh"
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/utils/Logger.hh"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// Define MinuitFitter clas
class MinuitFitter {

 public:

  // Default constructor - shouldn't be used
  MinuitFitter();
  MinuitFitter(int _Option, unsigned long int _seed);

  void MinuitfcnLogLike(int &, double *, double &, double *, int);
  void MinuitfcnBinnedChisq(int &, double *, double &, double *, int);
  void MinuitfcnUnBinnedLogLike(int &, double *, double &, double *, int);
  
  void Init_Bremsfunc(char* name, double *range);
  void SetPar_Bremsfunc(double *par);

  void Init_Signalfunc(char* name, double *range);
  void SetPar_Signalfunc(double *par);  
  
  void Init_UnBinnedData(unsigned long int data_size, double* dataset);
  
  void Init_SBfunc(char* name, double *range);
  void SetPar_SBfunc(double *par);
  
  void InitBrems_Data(unsigned long int pulseNumBrems);
  void GenBrems_Data(unsigned long int pulseNumBrems);
  
  void InitSignal_Data(unsigned long int pulseNumSignal);
  void GenSignal_Data(unsigned long int pulseNumSignal);
  
  void InitSB_Data(unsigned long int pulses);
  void GenSB_Data(unsigned long int pulses); 
  
  void Histo_Data(double *fit_range, TH1D* histo);
  void fit(double* par, int NP);
  TF1* plotMinuit_NORM(char* name, double *par, double *range, double scale);
  void corset(const TMatrixD& V, TMatrixD& C); //cov matrix, return sqrt(cov matrix)
  void corgen(const TMatrixD& C, double *x, int NP_); //sqrt(cov matrix), sampled values

  double* return_dataBrems() { return dataBrems; }
  double* return_dataSignal() { return dataSignal; }
  double* return_dataSB() { return dataSB; }
  double* return_bincontent() { return bincontent; }
  double* return_bincontent_errors() { return bincontent_error; }
  double* return_bincenter() { return bincenter; }

  unsigned long int return_nbins() { return nbins; }
  unsigned long int return_nbinsfit() { return nbins_fit; }
  int return_NPbrems() { return NPbrems; }
  int return_NPsignal() { return NPsignal; }
  int return_NP() { return NP; }
  double return_binning() { return _binning; }
  double return_frange_min() { return frange[0]; }
  double return_frange_max() { return frange[1]; }
  unsigned long int return_dof() { dof = nbins_fit - NP; return dof; }

  bool return_useintegral() { return useintegral;}
  bool return_usebin_loglike(){ return usebin_loglike;}
  bool return_usebin_chi2(){ return usebin_chi2;}
  bool return_useunbin_loglike(){ return useunbin_loglike;}

  TF1* return_fbrems(){ return fbrems; }
  TF1* return_fsignal(){ return fsignal; }
  TF1* return_fSB(){ return fSB; }

  //Return fit parameters
  double return_amin(){ return amin; }
  double return_edm(){ return edm; }
  double return_errdef(){ return errdef; }
  int return_nvpar(){ return nvpar; }
  int return_nparx(){ return nparx; }
  int return_icstat(){ return icstat; }
  double* return_p_minuit(){ return p_minuit; }
  double* return_perr_minuit(){ return perr_minuit; }
  TMatrixD* return_Covmatrix(){ return Covmatrix; }

  //Constructor
  bool usebin_chi2; // use least squared method to minimise
  bool usebin_loglike; // use binned log likelihood method to minimise 
  bool useunbin_loglike; // use unbinned log likelihood method to minimise
  bool useintegral; // evaluate FCN using integral instead of evaluating the function in bin center, used just for binned methods
  bool defaultminuit; // use default minuit
  bool fixparameters_fit; // fix parameters to do the fit
  TRandom3* rndm_ptr;

 private:

  //Data
  double* bincontent; //z[]
  double* bincontent_error; //zerror[]
  double* bincenter; //x[]

  double* dataBrems;
  double* dataSignal;
  double* dataSB;
  double* _dataset; //r[] for unbinned (signal+background)
  unsigned long int _data_size; //data size of signal+ back

  //Initial parameters
  unsigned long int nbins_fit, dof;
  double _binning;
  unsigned long int nbins = 20000;
  int NPbrems = 4; //fit parameters for brems func
  int NPsignal = 3; //fit parameters for gauss func
  int NP = 7; //fit parameters for brems+gaus func
  double frange[2]={0.04,2};
  //If fix parameters (provide number of parameters to fix and which ones)
  static const int Npar_fix = 3;
  int fixpar[Npar_fix]={1,2,3};

  //ROOT objects
  TF1* fbrems;
  TF1* fsignal;
  TF1* fSB;

  //Minuit result
  double amin,edm,errdef;
  int nvpar,nparx,icstat;
  double* p_minuit;
  double* perr_minuit;
  TMatrixD* Covmatrix;

};

#endif
