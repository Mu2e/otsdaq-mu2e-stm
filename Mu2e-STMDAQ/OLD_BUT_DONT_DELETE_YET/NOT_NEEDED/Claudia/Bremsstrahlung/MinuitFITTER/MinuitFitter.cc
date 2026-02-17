//#include "/work/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
//#include "/home/stm_mu2e/claudiaa/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"
#include "/work/mu2e/cgarcia/STMDAQ-TestBeam/Claudia/Bremsstrahlung/MinuitFITTER/MinuitFitter.h"

double Bremsfunc_fit(double x, double *par)
{  
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  double value = A/(exp(B*x)+C)+D;
  return value;
};

//Just to initialise functions
double Bremsfunc(double* x, double* par)
{
  return Bremsfunc_fit(x[0],par);
};

double BremsfuncNORM_fit(double x, double *par)
{
  double A = par[0];
  double B = par[1];
  double C = par[2];
  double D = par[3];

  //Normalising range 0.04 - 2 
  double frange[2]={0.04,2};

  double I_0 = ( (-1)*par[0]*TMath::Log(1+par[2]*TMath::Exp((-1)*par[1]*frange[0])) )/(par[1]*par[2]) + (par[3] * frange[0]);
  double I_1 = ( (-1)*par[0]*TMath::Log(1+par[2]*TMath::Exp((-1)*par[1]*frange[1])) )/(par[1]*par[2]) + (par[3] * frange[1]);
  double norm = I_1 - I_0;
  //std::cout<<"NORMALISATION: "<<norm<<std::endl;

  double value = (A/(exp(B*x)+C)+D) / norm;
  return value;
};

double Signalfunc_fit(double x, double *par)
{
  double mu    = par[1];
  double sigma = par[2];
  double norm  = 1./sqrt(2.*TMath::Pi())/sigma;
  double G     = norm*exp(-0.5 *pow((x-mu)/sigma,2));

  return par[0] * G;
};

double Signalfunc(double* x, double *par)
{
  return Signalfunc_fit(x[0],par);
};

double SignalfuncNORM_fit(double x, double *par)
{
  double mu    = par[0];
  double sigma = par[1];
  double norm  = 1./sqrt(2.*TMath::Pi())/sigma;
  double G     = norm*exp(-0.5 *pow((x-mu)/sigma,2));

  return G;
};

//Binned LogLikelihood and Chi2
double SBfunc_fit(double x, double *par)
{
  double* parS = new double[3];
  parS[0] = par[0];
  parS[1] = par[1];
  parS[2] = par[2];

  double* parB = new double[4];
  parB[0] = par[3];
  parB[1] = par[4];
  parB[2] = par[5];
  parB[3] = par[6];

  double f = Signalfunc_fit(x,parS) + Bremsfunc_fit(x,parB);
  delete parS; delete parB;
  
  return f;
};

double SBfunc(double *x, double *par)
{
  return SBfunc_fit(x[0],par);
};

//Unbinned LogLikelihood 
double SBfuncNORM_fit(double x, double *par) 
{
  double* parS = new double[2];
  parS[0] = par[0];
  parS[1] = par[1];
  
  double* parB = new double[4];
  parB[0] = par[2];
  parB[1] = par[3];
  parB[2] = par[4];
  parB[3] = par[5];

  double fs = par[6];

  double f = (fs * SignalfuncNORM_fit(x,parS) )+ ((1.-fs) *  BremsfuncNORM_fit(x,parB));
  delete parS; delete parB;
  
  return f;
};

//double SBfuncNORM(double* x, double *par)
//{
//return SBfuncNORM_fit(x[0],par);
//};

//JUST USEFUL FOR PLOTTING UNBINNED (Need to scale fitted function from minuit)

double SBfuncNORM_cte_fit(double x, double *par)
{
  double* parS = new double[2];
  parS[0] = par[0];
  parS[1] = par[1];

  double* parB = new double[4];
  parB[0] = par[2];
  parB[1] = par[3];
  parB[2] = par[4];
  parB[3] = par[5];

  double fs = par[6];

  //Rescale with a cte
  double f = par[7]*((fs * SignalfuncNORM_fit(x,parS)) + ((1.-fs) *  BremsfuncNORM_fit(x,parB)));
  delete parS; delete parB;
  return f;
};

double SBfuncNORM_cte(double* x, double *par)
{
  return SBfuncNORM_cte_fit(x[0],par);
};


//////////******Class functions******//////////

static MinuitFitter* static_fitter_pointer = NULL;

void fitter_fcn_wrapper(int &npar, double *gin, double &f, double *par, int iflag) {
  if (static_fitter_pointer==NULL) {
    Logger::Instance()->write(Logger::ERROR,"Unitialised Minuit function pointer - EXITING..");
    exit(-1);
  }

  if(static_fitter_pointer->usebin_loglike) {
    static_fitter_pointer->MinuitfcnLogLike(npar, gin, f, par, iflag);
  }
  if(static_fitter_pointer->usebin_chi2) {
    static_fitter_pointer->MinuitfcnBinnedChisq(npar, gin, f, par, iflag);
  }
  if(static_fitter_pointer->useunbin_loglike) {
    static_fitter_pointer->MinuitfcnUnBinnedLogLike(npar, gin, f, par, iflag);
  }
}

MinuitFitter::MinuitFitter(){
  
  //Chi2 by default
  usebin_loglike = false;
  usebin_chi2 = true;
  useunbin_loglike = false;
  useintegral = true;
  fixparameters_fit = false;
  defaultminuit = true;
  
  //Random seed
  unsigned long _seed = 0;
  rndm_ptr = new TRandom3(_seed);
  std::cout<<"Init rndm Seed: "<<rndm_ptr->GetSeed()<<std::endl;
}

MinuitFitter::MinuitFitter(int _Option, unsigned long _seed){
 
  // _seed
  // Generate random pointer
  rndm_ptr = new TRandom3(_seed);
  std::cout<<"Init rndm Seed: "<<rndm_ptr->GetSeed()<<std::endl;
  // _Option
  // 1st num -> 1-Chi2, 2-BinnedLog, 3-UnbinnedLog
  // 2nd num -> 0-NoIntegral, 1-Integral
  // 3rd num -> 0-DefaultMinuit, 1-Set Minuit
  // 4th num -> 0-NoFixParameters, 1-FixParameters

  usebin_chi2 = false;
  usebin_loglike = false;
  useunbin_loglike = false;
  useintegral = false;
  defaultminuit = true;
  fixparameters_fit = false;

  //Get number
  int a4 = _Option % 10; //4th
  int a3_aux = _Option / 10;
  int a3 = a3_aux % 10; //3rd
  int a2_aux = a3_aux / 10;
  int a2 = a2_aux % 10; //2nd
  int a1_aux = a2_aux / 10;
  int a1 = a1_aux % 10; //1st

  if(a1==1){    
    std::cout<<"****************************************************"<<std::endl;
    std::cout<<"**MINUIT: FITTING WITH BINNED LEAST SQUARES - CHI2**"<<std::endl;
    std::cout<<"****************************************************"<<std::endl;

    usebin_chi2 = true;
  }
  else if(a1==2){
    std::cout<<"****************************************************"<<std::endl;
    std::cout<<"*****MINUIT: FITTING WITH BINNED LOG-LIKELIHOOD*****"<<std::endl;
    std::cout<<"****************************************************"<<std::endl;

    usebin_loglike = true;
  }
  else if(a1==3){
    std::cout<<"****************************************************"<<std::endl;
    std::cout<<"****MINUIT: FITTING WITH UNBINNED LOG-LIKELIHOOD****"<<std::endl;
    std::cout<<"****************************************************"<<std::endl;

    useunbin_loglike = true;
  }
  else{ std::cout<<"ERROR::: bad constructor FIT MODE"<<std::endl; exit(0); }

  if(a2==1){
    std::cout<<"*Using INTEGRAL instead of bin center****************"<<std::endl;
    useintegral = true;
  }
  else{ std::cout<<"*Using BIN CENTER********************************"<<std::endl; }

  if(a3==1){
    std::cout<<"*Using Improved Minuit SETS**************************"<<std::endl;
    defaultminuit = false;
  }
  else{ std::cout<<"*Using Default Minuit SETS***********************"<<std::endl; }
  
  if(a3==4){
    std::cout<<"*Fixing fit parameters*******************************"<<std::endl;
    fixparameters_fit = true;
  }
  else{ std::cout<<" "<<std::endl; }

}


void MinuitFitter::Init_Bremsfunc(char* name, double *range) {

  fbrems = new TF1(name,Bremsfunc,range[0],range[1],NPbrems);
};

void MinuitFitter::SetPar_Bremsfunc(double *par) {
  fbrems->SetNpx(300000);
  for(int i = 0 ; i < NPbrems; i++) {
    fbrems->SetParameter(i,par[i]);
  }
};

void MinuitFitter::Init_Signalfunc(char* name, double *range) {

  fsignal = new TF1(name,Signalfunc,range[0],range[1],NPsignal);
};

void MinuitFitter::SetPar_Signalfunc(double *par) {
  fsignal->SetNpx(300000);
  for(int i = 0 ; i < NPsignal; i++) {
    fsignal->SetParameter(i,par[i]);
  }
};

void MinuitFitter::Init_SBfunc(char* name, double *range) {

  fSB = new TF1(name,SBfunc,range[0],range[1],NP);
};

void MinuitFitter::SetPar_SBfunc(double *par) {

  fSB->SetNpx(300000);
  for(int i = 0 ; i < NP; i++) {
    fSB->SetParameter(i,par[i]);
  }
};

//Multiply the funtion by 'scale'

TF1* MinuitFitter::plotMinuit_NORM(char* name, double *par, double *range, double scale) {
  double NPscaled = NP+1;
  TF1* fSB_scaled = new TF1(name,SBfuncNORM_cte,range[0],range[1],NPscaled);

  fSB_scaled->SetNpx(300000);
  for(int i = 0 ; i < NP; i++){
    fSB_scaled->SetParameter(i,par[i]);
  }

  fSB_scaled->SetParameter(NP,scale);

  return fSB_scaled;
};

void MinuitFitter::InitBrems_Data(unsigned long int pulseNumBrems) {

  dataBrems = new double[pulseNumBrems];
};

void MinuitFitter::GenBrems_Data(unsigned long int pulseNumBrems) {
  for(unsigned long int i =0; i < pulseNumBrems; i++) {
    dataBrems[i] = fbrems->GetRandom(rndm_ptr,"");
  }
};

void MinuitFitter::InitSignal_Data(unsigned long int pulseNumSignal) {

  dataSignal = new double[pulseNumSignal];
};

void MinuitFitter::GenSignal_Data(unsigned long int pulseNumSignal) {
  for(unsigned long int i =0; i < pulseNumSignal; i++) {
    dataSignal[i] = fsignal->GetRandom(rndm_ptr,"");
  }
};


void MinuitFitter::InitSB_Data(unsigned long int pulses) {

  dataSB = new double[pulses];
};

void MinuitFitter::GenSB_Data(unsigned long int pulses) {

  for(unsigned long int i =0; i < pulses; i++) {
    dataSB[i] = fSB->GetRandom(rndm_ptr,"");
  }

};

//Fill the histogram with data and recover data

void MinuitFitter::Histo_Data(double *fit_range, TH1D *histo) {
  nbins_fit = 0;
  _binning = histo->GetBinWidth(1);
  unsigned long int bin_min = histo->FindFixBin(fit_range[0]);
  unsigned long int bin_max = histo->FindFixBin(fit_range[1]);

  unsigned long int size = bin_max - bin_min;

  std::cout<<"Fit range: ["<<fit_range[0]<<", "<<fit_range[1]<<"]"<<std::endl;
  std::cout<<"histo->GetNbinsX(): "<<histo->GetNbinsX()<<" "<<(int)nbins<<std::endl;
  std::cout<<"bin_min: "<<bin_min<<" bin_max: "<<bin_max<<" size: "<<size<<std::endl;
  bincontent = new double[size];
  bincontent_error = new double[size];
  bincenter = new double[size];

  //Return variables from histogram
   if(histo->GetNbinsX() != (int)nbins) { std::cout<<"ERROR::: bad Nbins in histogram"<<std::endl; exit(0); }
   else {
    for(unsigned long int i = bin_min; i < bin_max; i++) {
      bincontent[nbins_fit] = histo->GetBinContent(i);
      bincontent_error[nbins_fit] = histo->GetBinError(i);
      bincenter[nbins_fit] = histo->GetBinCenter(i);
      nbins_fit++;
    }
   }
};

void MinuitFitter::MinuitfcnLogLike(int &npar, double *gin, double &f, double *par, int iflag) {
 
  double LogLike = 0;
  double log;

  for (unsigned long int i = 0; i < nbins_fit; i++) {

    if(bincontent[i] > 0) {

      //Don't use integral - x[]=bincenter
      double y_fit1 = SBfunc_fit(bincenter[i],par);
     
      //Use integral
      if(useintegral==true) {

	double xlo = bincenter[i] - 0.5*_binning;
	double xhi = bincenter[i] + 0.5*_binning;

	fSB->SetNpx(300000);
	for(int k = 0; k < NP; k++) { fSB->SetParameter(k,par[k]); }

        double integral = fSB->Integral(xlo,xhi);
	double y_fit2 = integral / _binning;

	//log  = (y_fit2-bincontent[i])+bincontent[i]*(ROOT::Math::Util::EvalLog(bincontent[i])-ROOT::Math::Util::EvalLog(y_fit2));
	log  = (y_fit2-bincontent[i])+bincontent[i]*(TMath::Log(bincontent[i])-TMath::Log(y_fit2));
      }
      else {
	//log  = (y_fit1-bincontent[i])+bincontent[i]*(ROOT::Math::Util::EvalLog(bincontent[i])-ROOT::Math::Util::EvalLog(y_fit1));
	log  = (y_fit1-bincontent[i])+bincontent[i]*(TMath::Log(bincontent[i])-TMath::Log(y_fit1));
      }

      LogLike += log;
    }
  }//nbins

  f = LogLike;
};

void MinuitFitter::MinuitfcnBinnedChisq(int &npar, double *gin, double &f, double *par, int iflag) {
 
  double chisq = 0;
  double delta;

  for (unsigned long int i = 0; i < nbins_fit; i++) {
    
    if(bincontent_error[i] != 0) {

      //Don't use integral - x[]=bincenter
      double y_fit1 = SBfunc_fit(bincenter[i],par);

      //Use integral
      if(useintegral==true){

	double xlo = bincenter[i] - 0.5*_binning;
	double xhi = bincenter[i] + 0.5*_binning;

	fSB->SetNpx(300000);
	for(int k = 0; k < NP; k++) { fSB->SetParameter(k,par[k]); }

        double integral = fSB->Integral(xlo,xhi);
	double y_fit2 = integral / _binning;

	delta  = (bincontent[i]-y_fit2)/bincontent_error[i];
      }
      else{
	delta  = (bincontent[i]-y_fit1)/bincontent_error[i];
      }
      //std::cout<<"Chi2: bin: "<<i<<" f(bincenter[i]): "<<y_fit1<<" data[i]: "<<bincontent[i]<<" error data[i]: "<<bincontent_error[i]<<" delta: "<<delta<<std::endl;
 
      chisq += delta * delta;
    }
  }//nbins

  //std::cout<<"chisq---: "<<chisq<<std::endl;
  f = chisq;
};

void MinuitFitter::Init_UnBinnedData(unsigned long int data_size, double* dataset) {
  
  _data_size = data_size;

  _dataset = new double[data_size];

  for (unsigned long int i = 0; i < data_size; i++) {
    _dataset[i] = dataset[i];
  }
}

void MinuitFitter::MinuitfcnUnBinnedLogLike(int &npar, double *gin, double &f, double *par, int iflag) {

  f = 0;
  for(unsigned long int i = 0; i < _data_size; i++) {

    double L = SBfuncNORM_fit(_dataset[i],par);
    //double L = BremsfuncNORM_fit(_dataset[i],par); 

    if (L>0.) f -= TMath::Log(L); //f = ROOT::Math::Util::EvalLog(L); 
    else { f = HUGE_VAL; return;}
  }
};

//Minuit fitting
void MinuitFitter::fit(double *par, int Ninput_par) {

  TMinuit* _minuit = new TMinuit();

  // Ensure that minimisation fcn function can be associated with this Fitter object
  static_fitter_pointer      = this;
  _minuit->SetFCN(fitter_fcn_wrapper);

  double val;
  int    ierr = 0;

  if(defaultminuit==false) {
    // Width of print-out - viva punch cards
    //val = 80.0;  _minuit->mnexcm( "SET WIDTH", &val, 1, ierr );
    // Strategy for calculating first and second derivatives : 2 is most accurate
    val = 2.0;   _minuit->mnexcm( "SET STR", &val, 1, ierr );
    // Accuracy of floating point in function calculation (we use doubles..)
    val = 1.e-12; _minuit->mnexcm( "SET EPS", &val, 1, ierr );
  }

  //Least-Squares
  if(usebin_chi2) {
    val = 1;
  }
  //Lhood
  else {
    val = 0.5; 
  }
  _minuit->mnexcm( "SET ERROR", &val, 1, ierr );

  //Set initial parameters 
  double step = 0.1;

  for (int i = 0; i <  Ninput_par; i++) {
    std::string par_name = std::to_string(i);
    _minuit->mnparm(i, par_name, par[i], step, 0,0,ierr);
  }

  // If fix parameters
  if(fixparameters_fit) {
    for(int i = 0 ; i < Npar_fix ; i++) {
      int _fixpar  = fixpar[i];
      _minuit->FixParameter(_fixpar);
    }
  }

  // Now all setup - do the fit...
  val = 1480; // max # of iterations
  _minuit->mnexcm( "MIGRAD", &val ,1, ierr);
  if (ierr) { 
    stringstream s2; s2 << "Fitter(): Minuit::MIGRAD failed : error code = " << ierr;
    Logger::Instance()->write(Logger::WARNING,s2.str());
  }

  // Now get the proper errors if binned / too slow for unbinned
  if(!useunbin_loglike) {
    val = 10000.0; 
    _minuit->mnexcm( "MINOS", &val, 1, ierr );
    if (ierr) { 
      stringstream s2; s2 << "Fitter(): Minuit::MINOS failed : error code = " << ierr;
      Logger::Instance()->write(Logger::WARNING,s2.str());
    }
  }

  // Get results
  p_minuit = new double[Ninput_par];
  perr_minuit = new double[Ninput_par];

  _minuit->mnstat(amin,edm,errdef,nvpar,nparx,icstat);
  //Get pars
  for (int i = 0; i <  Ninput_par; i++) {
    _minuit->GetParameter(i,p_minuit[i],perr_minuit[i]);
  }
  //Get Cov matrix
  Covmatrix = new TMatrixD(Ninput_par,Ninput_par);
  _minuit->mnemat(Covmatrix->GetMatrixArray(),Ninput_par);

  return;
};

//Sample from Cov Matrix

void MinuitFitter::corset( const TMatrixD& V, TMatrixD& C ){
  // calculate sqrt(V) as lower diagonal matrix
  // if we dont know the number of params
  int NP_ = V.GetNcols();

  for( int i = 0; i < NP_; ++i ) {
    for( int j = 0; j < NP_; ++j ) {
      C[i][j] = 0;
    }
  }
  for( int j = 0; j < NP_; ++j ) {
    // diagonal terms first
    double Ck = 0;
    for( int k = 0; k < j; ++k ) {
      Ck += C[j][k] * C[j][k];
    } // k
    C[j][j] = sqrt( fabs( V[j][j] - Ck ) );
    // off-diagonal terms
    for( int i = j+1; i < NP_; ++i ) {
      Ck = 0;
      for( int k = 0; k < j; ++k ) {
	Ck += C[i][k] * C[j][k];
      } //k
      C[i][j] = ( V[i][j] - Ck ) / C[j][j];
    }// i
  } // j
}

void MinuitFitter::corgen(const TMatrixD& C, double *x, int NP_){

  double *z =new double[NP_];
  // np random numbers from unit Gaussian
  for( int i = 0; i < NP_; ++i )
    z[i] = rndm_ptr->Gaus( 0.0, 1.0 );
  // fill values
  for( int i = 0; i < NP_; ++i ) {
    x[i] = 0;
    for( int j = 0; j <= i; ++j ) {
      x[i] += C[i][j] * z[j];
    } // j
  } // i
  delete [] z; //free the array
}

