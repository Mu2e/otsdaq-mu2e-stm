#ifndef BEAMVARS_hh
#define BEAMVARS_hh

static const double months_taking_data = 9;
static const double Mu2e_efficiency_year = 0.95;
static const double main_injectorcycle = 1.4; //sec 
static const double sec_taking_data = (months_taking_data/12)*365*24*3600*Mu2e_efficiency_year; //sec
static const int MICycles_year = sec_taking_data/main_injectorcycle;
static const double fADC = 370; //MHz
static const double tadc = 1.0/(fADC); //us
static const double spillsize = 43.1*0.001; //sec
static const double nspills = 8;
static const double ngaps = 7;
static const double spillgaps = 5*0.001; //sec
//static const double spillmaingap = 36*0.001; //sec
static const double beam_on_size = nspills * spillsize; //sec
static const double beam_on_spills_gaps = beam_on_size + ngaps * spillgaps; //sec 
static const double beam_on_period = 8*spillsize + 7*spillgaps; //sec
static const double gapsize = main_injectorcycle - beam_on_period; //sec

//At high rates we write all data in one spill
//Maximum data that we can write per spill
static const double limit_spilldata = beam_on_size*fADC*2/(1e6); //TB

//Maximum data that we can write per gap
static const double limit_gapdata = gapsize*fADC*2/(1e6); //TB

//Data stored per ZS peak
static const double ZPpeak_durationHPGe = 0.000003; //sec

//Data allowance in disk per year
static const double space_disk_TB = 500; //Tbytes
static const double space_disk_Gb = space_disk_TB*1000*8; //Gbit
static const double speedlimit = space_disk_Gb/sec_taking_data; //Gbit/s

//Percentage of 3d->2p transitions (66 keV Xrays)
static const double dp_muons = 62; //of stopped
//Percentage of 2p->1s transitions (347 keV Xrays)
static const double tot_ps_muons = 80;
static const double ps_muons = tot_ps_muons-dp_muons; //of stopped
//Percentage of DIO
static const double tot_DIO_muons = 39;
static const double DIO_muons = 7.8; //of stopped 
//Percentage of captured muons
static const double tot_captured_muons = 61; //of stopped
static const double captured_muons = 12.2; //of stopped  

//Percentage of 844 keV Xrays
static const double Mgbetadecay_muons = 9.2; //of captured
//Percentage of 1809 keV Xrays
static const double excitedAl_muons = 51; //of captured
//Percentage no x-ray
static const double noxrays = 100-Mgbetadecay_muons-excitedAl_muons;

//Muonic Al lifetime
static const double muAl_lifetime_s = 864e-9; //864ns in sec //in reality 1416ns
static const double muAl_lifetime_us = muAl_lifetime_s*1e6; //us 
//Excited Al to ground state lifetime
static const double groundstateAl_lifetime_s = 693e-15; //sec
static const double groundstateAl_lifetime_us = groundstateAl_lifetime_s*1e6; //us
//Beta decay time
static const double betadecay_time_min = 9.5; //min
static const double betadecay_time_s = 9.5*60; //sec
static const double betadecay_time_us = betadecay_time_s*1e6; //us
static const double betadecay_rate_Hz = log(2)/betadecay_time_s; //Hz
//Prompt Xray delay
static const double prompt_time_s = 1e-15; // 1fs in sec
static const double prompt_time_us = prompt_time_s*1e6; //us
//DIO lifetime
static const double DIO_lifetime_s = 2.2e-6; //2.2us in sec
static const double DIO_lifetime_us = DIO_lifetime_s*1e6; //us 
#endif
