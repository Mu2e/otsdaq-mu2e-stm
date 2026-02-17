#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include <boost/chrono.hpp>
#include<cmath>
#include <sys/stat.h>
#include <cstring>

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

#include "TApplication.h"
#include "TRootCanvas.h"


//using namespace std;


int main(int argc, char* argv[]){

  std::string  filename  = std::string(argv[1]);
  std::cout << "filename = " << filename << std::endl;


#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  boost::chrono::high_resolution_clock::time_point t1 ;
  boost::chrono::high_resolution_clock::time_point t2 ;
 



  struct stat st;
  stat(filename.c_str(), &st);
  unsigned long int  n = st.st_size/2;  // get size of file (in bytes) and set number of ADC values (each vlaue is 2 bytes)
  std::ifstream myFile;

  t1 = boost::chrono::high_resolution_clock::now();
  
  //Open the binary file with data to suppress
  int16_t* ADC = new int16_t[n];
  myFile.open(filename, std::ios::in | std::ios::binary);
  myFile.read( (char*) ADC, n*sizeof(ADC[0]));
  myFile.close();

  myFile.close();
  t2 = boost::chrono::high_resolution_clock::now();

  std::cout << "Read data ... " << std::endl;

  std::cout<< "Number of elements: "<<n<< " in time " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  t1 = boost::chrono::high_resolution_clock::now();




  //----------------------- PEAKS SUPPRESSION ALGORITHM-----------------------------------------
 
  //Calculate the gradient for ADC values separated a distance window (in time would be window * tadc) 
  unsigned long int window=100;
  //gradient vector
  int16_t* gradient = new int16_t[n-window];
  //averaged gradient vector
  double* avgradient = new double[n-window];
  //time in clocks ticks for each raw ADC value
  double* time = new double[n];
  //time in us for each raw ADC value
  double* timeus = new double[n];
  //averaged time for each averaged gradient
  double* avADCtime = new double[n-window];
  //store suppressed data
  int16_t* suppressed_data = new int16_t[n-window];
  //vector with times for each ADC value suppressed
  double* suppressed_time = new double[n];
  //vector with each trigger time in clock ticks
  unsigned long int* trigger_vect = new unsigned long int[n-window];
  


  //t adc in microsec //Use the same as in the simulation
  // ADC samling frequency (MHz)
  //const double fADC = 320.0520833313;
  const double fADC = 370;
  //Sampling time of ADC (microsec)
  const double tadc = 1.0/(fADC);

  //Calculate the time and gradient vectors
  for(unsigned long int i=0;i<(n-window);i++){

    gradient[i]=ADC[i+window]-ADC[i];
  }

  for(unsigned long int i=0;i<n;i++){
    
    //time in microsec
    timeus[i]=i*tadc;
    //trigger time in clock ticks
    time[i]=i;
    //std::cout<<i<<" time[i]: "<<timeus[i]<<std::endl;
    
  }


  //Calculate the average of the gradient each n_average ADC values to avoid fluctuations
  int n_average=5;
  unsigned long int j=0;
  double av_gradient=0;
  double av_ADCtime=0;
  unsigned long int h=0;

  #ifdef PRINT_COUT
  std::cout<<"Average of gradient taken with: "<<n_average<<" values"<<std::endl;
  #endif 

  while(j<(n-window)){
    //Each point of the gradient and ADCtime averaged with the (n_average-1) following points, each point is the mean of n_average points of the gradient
    if((j+n_average)>(n-window)){n_average= (n-window)-j;}
    av_gradient=0;
    av_ADCtime=0;
    for(int k=0;k<n_average;k++){
      av_gradient= av_gradient+gradient[j+k];
      av_ADCtime=av_ADCtime+time[j+k];
    }
    avgradient[h]=av_gradient/n_average;
    avADCtime[h]=av_ADCtime/n_average;

    j=j+n_average;
    h++;
  }





  //Initial values
  bool peak=false;
  int counter=0;
  int trigger=0;
  int triggercounter=0;

  //----Variable parameters----
  //Find the trigger
  int threshold=-100;


  //store tbefore microseconds of data to the left of the trigger
  double tbefore=1;
  int prenumADCstored=int(tbefore/tadc);

  //store tafter microseconds of data to the right of the trigger
  double tafter=2;
  int postnumADCstored=int(tafter/tadc);

  //total number of ADC values stored per trigger
  int totalnumADCstored=prenumADCstored+postnumADCstored;
  //---------------------------

  unsigned long int size_sup;
  
  #ifdef PRINT_COUT
  std::cout<<"fadc= "<<fADC<<" tadc= "<<tadc<<" MHz, window: "<<window<<" threshold "<<threshold<<" tbefore: "<<tbefore<<" us ("<<prenumADCstored<<" ADC Counts)"<<" tafter "<<tafter<<" us ("<<postnumADCstored<<" ADC Counts)"<<std::endl;
  #endif

  //Store positions in clock ticks for the triggers found, fill trigger_vect[]
  //h is the number of elements in the averaged gradient vector
  for(unsigned long int i=0;i<h;i++){
   
    if(avgradient[i]>threshold){peak=false;
      continue;}
    //skip the rest indexes of the peak after the trigger that has already been stored
    if((avgradient[i]<threshold)&&(peak==true)){
      continue;}

    
    if((avgradient[i]<threshold)&&(peak==false)){
      peak=true;
      trigger=avADCtime[i];
      trigger_vect[triggercounter]=trigger;


      //std::cout<<"avgrad"<<avgradient[i]<<" Trigger number: "<<triggercounter<<": "<<trigger<<" Triggertime: "<<trigger*tadc<<std::endl;
      triggercounter++;

      //std::cout<<trigger<<std::endl;
    }
  }



  //We have (prenumADCstored+postnumADCstored) ADC values per trigger
  //Index of last trigger stored in suppressed_data[] array
  int last_triggerstored_index=0;
  //Difference between 2 following triggers
  int trig_dif=0;

  //Copy data around triggers found, create suppressed_data array and  suppressed_time array i.e. the timings in clock ticks of each ADC value stored (easy to plot sup and raw data tpgether)
  for(int i=0;i<triggercounter;i++){
    //first trigger
    if(i==0){
      //check that there are at least prenumADCstored ADC values before the trigger, if not, the number of ADC values is: trigger_vect[i]
      if(trigger_vect[i]-prenumADCstored<0){memcpy(&suppressed_data[0],&ADC[0],2*(trigger_vect[i]+postnumADCstored));
	
        #ifdef PRINT_COUT
        std::cout<<"Not "<<prenumADCstored<<" ADC values before 1st trigger"<<std::endl;
	#endif

	 memcpy(&suppressed_time[0],&time[0],8*(trigger_vect[i]+postnumADCstored)); //be careful they are unsigned long int so 8 bytes 

        #ifdef PRINT_COUT      
	std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	std::cout<<"Copied values from ADC data from index: 0 to "<<trigger_vect[i]+postnumADCstored<<std::endl;
	std::cout<<"Copied to suppressed data array from index: 0 to "<<trigger_vect[i]+postnumADCstored<<std::endl;
	std::cout<<"Time in ADC counts from: 0 to "<<time[trigger_vect[i]+postnumADCstored]<<std::endl;
	#endif
	
	//index of 1st trigger stored in sup array
	last_triggerstored_index=trigger_vect[i];}

      else{memcpy(&suppressed_data[0],&ADC[trigger_vect[i]-prenumADCstored],2*totalnumADCstored);
	memcpy(&suppressed_time[0],&time[trigger_vect[i]-prenumADCstored],8*totalnumADCstored);

	#ifdef PRINT_COUT
	std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	std::cout<<"Copied values from ADC data from index: "<<trigger_vect[i]-prenumADCstored<<" to "<<trigger_vect[i]+postnumADCstored<<std::endl;
	std::cout<<"Copied to suppressed data array from index: 0 to "<<totalnumADCstored<<std::endl;
	std::cout<<"Time in ADC counts from: "<<time[trigger_vect[i]-prenumADCstored]<<" to "<<time[trigger_vect[i]+postnumADCstored]<<std::endl;
	#endif

	//index of 1st trigger stored in sup array
	last_triggerstored_index=prenumADCstored;}
      
    }

    //if next triggers
    else{
      trig_dif=trigger_vect[i]-trigger_vect[i-1];
     
      //overlapped data to be stored
      if(trig_dif<totalnumADCstored){
	//check last trigger (not postnumADCstored after the last trigger)
	if(trigger_vect[i]+postnumADCstored>n){
	  unsigned long int remainADC=n-trigger_vect[i];
	  memcpy(&suppressed_data[last_triggerstored_index+(trig_dif-prenumADCstored)],&ADC[trigger_vect[i]-prenumADCstored],2*(prenumADCstored+remainADC));
	  
	  #ifdef PRINT_COUT
	  std::cout<<"Last trigger exceeds "<<postnumADCstored<<" ADC values"<<std::endl;
	  #endif
	  
	  memcpy(&suppressed_time[last_triggerstored_index+(trig_dif-prenumADCstored)],&time[trigger_vect[i]-prenumADCstored],8*(prenumADCstored+remainADC));

	  size_sup=last_triggerstored_index+trig_dif+remainADC; 

	  #ifdef PRINT_COUT
	  std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	  std::cout<<"Copied values from ADC data from index: "<<trigger_vect[i]-prenumADCstored<<" to "<<trigger_vect[i]+remainADC<<std::endl;
	  std::cout<<"Copied to suppressed data array from index:"<<last_triggerstored_index+(trig_dif-prenumADCstored)<<" to "<<last_triggerstored_index+trig_dif+remainADC<<std::endl;
	  std::cout<<"Time in ADC counts from: "<<time[trigger_vect[i]-prenumADCstored]<<" to "<<time[trigger_vect[i]+remainADC-1]<<std::endl; //Don't understand why I have to put this -1  
          #endif

	  break;}
	
	memcpy(&suppressed_data[last_triggerstored_index+(trig_dif-prenumADCstored)],&ADC[trigger_vect[i]-prenumADCstored],2*totalnumADCstored);
	memcpy(&suppressed_time[last_triggerstored_index+(trig_dif-prenumADCstored)],&time[trigger_vect[i]-prenumADCstored],8*totalnumADCstored);

	size_sup=last_triggerstored_index+trig_dif+postnumADCstored;

	#ifdef PRINT_COUT
	std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	std::cout<<"Copied values from ADC data from index: "<<trigger_vect[i]-prenumADCstored<<" to "<<trigger_vect[i]+postnumADCstored<<std::endl;
	std::cout<<"Copied to suppressed data array from index:"<<last_triggerstored_index+(trig_dif-prenumADCstored)<<" to "<<last_triggerstored_index+trig_dif+postnumADCstored<<std::endl;
	std::cout<<"Time in ADC counts from: "<<time[trigger_vect[i]-prenumADCstored]<<" to "<<time[trigger_vect[i]+postnumADCstored]<<std::endl;
	#endif

	last_triggerstored_index=last_triggerstored_index+trig_dif;
      }
    
      //new data to be stored
      if(trig_dif>=totalnumADCstored){
	//check last trigger (not postnumADCstored after the last trigger)
	if(trigger_vect[i]+postnumADCstored>n){
	  unsigned long int remainADC=n-trigger_vect[i];
	  memcpy(&suppressed_data[last_triggerstored_index+postnumADCstored],&ADC[trigger_vect[i]-prenumADCstored],2*(prenumADCstored+remainADC));
	  
	  #ifdef PRINT_COUT
	  std::cout<<"Last trigger exceeds "<<postnumADCstored<<" ADC values"<<std::endl;
	  #endif
	  
	  memcpy(&suppressed_time[last_triggerstored_index+postnumADCstored],&time[trigger_vect[i]-prenumADCstored],8*(prenumADCstored+remainADC));
	  
	  size_sup=last_triggerstored_index+postnumADCstored+prenumADCstored+remainADC;

	  #ifdef PRINT_COUT
	  std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	  std::cout<<"Copied values from ADC data from index: "<<trigger_vect[i]-prenumADCstored<<" to "<<trigger_vect[i]+remainADC<<std::endl;
	  std::cout<<"Copied to suppressed data array from index:"<<last_triggerstored_index+postnumADCstored<<" to "<<last_triggerstored_index+totalnumADCstored+remainADC<<std::endl;
	  std::cout<<"Time in ADC counts from: "<<time[trigger_vect[i]-prenumADCstored]<<" to "<<time[trigger_vect[i]+remainADC-1]<<std::endl; //Don't understand why I have to put this -1
	  #endif

	  break;}
	
	memcpy(&suppressed_data[last_triggerstored_index+postnumADCstored],&ADC[trigger_vect[i]-prenumADCstored],2*totalnumADCstored);
	memcpy(&suppressed_time[last_triggerstored_index+postnumADCstored],&time[trigger_vect[i]-prenumADCstored],8*totalnumADCstored);
	
	size_sup=last_triggerstored_index+postnumADCstored+totalnumADCstored;

	#ifdef PRINT_COUT
	std::cout<<"Trigger num "<<i<<" at "<<trigger_vect[i]<<" clock ticks "<<trigger_vect[i]*tadc<<" us"<<std::endl;
	std::cout<<"Copied values from ADC data from index: "<<trigger_vect[i]-prenumADCstored<<" to "<<trigger_vect[i]+postnumADCstored<<std::endl;
	std::cout<<"Copied to suppressed data array from index:"<<last_triggerstored_index+postnumADCstored<<" to "<<last_triggerstored_index+postnumADCstored+totalnumADCstored<<std::endl;
	std::cout<<"Time in ADC counts from: "<<time[trigger_vect[i]-prenumADCstored]<<" to "<<time[trigger_vect[i]+postnumADCstored]<<std::endl;
	#endif

	last_triggerstored_index=last_triggerstored_index+totalnumADCstored;
      }
    }//else 
  }




  //Write ADC peaks to the binary file
  //std::string s= "/work/cgarcia/DATA/Claudia/GenData/MWDEfficiency_SimPoisson/SuppresedFiles_tbefore2us_tafter10us_newcodev3/Supdata"+filename.substr(filename.find("Noise"), 15);
  //std::string s= "/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.17mV/smallfiles/ZP_v3/Supdata"+filename.substr(filename.find("dataNoise_"), 37);  
  //std::string s= "/work/cgarcia/DATA/MWD_Analysis/RUN109/SuppresedFiles_tbefore2us_tafter10us_v3/Suppressed_run109"+filename.substr(filename.find(".bin"), 7);
  //std::string s="/work/cgarcia/DATA/Claudia/GenData/GaussianNoise_SimPoisson/Height1185ADCCounts/sigmaNoise0.17mV/SuppresedFiles_tbefore2us_tafter10us_newcodev3/Sup"+filename.substr(filename.find("dataNoise"), 19);
  //std::string s="/work/cgarcia/DATA/Claudia/GenDataehHPGeSim/1836keV_0.32mV/SuppresedFiles_tbefore1us_tafter2us_newcodev3/Supd"+filename.substr(filename.find("ata1836keV_"), 20);
  std::string s="hi.bin";
  //std::string s= "/work/cgarcia/DATA/Claudia/HLSData/Software_tbefore1ustafter2us/Suppressed1us_2us_software_run109_00.bin";
  int size = s.length();
  char file_name[size+1];
  // copying the contents of the
  // string to char array

  t2 = boost::chrono::high_resolution_clock::now();
  std::cout<<""<<std::endl;
  std::cout << "Zero Suppression Algorithm computing time = " << boost::chrono::duration_cast<boost::chrono::milliseconds>(t2-t1) << std::endl;

  strcpy(file_name, s.c_str());
  FILE * fp = fopen(file_name, "wb");                                               
  fwrite(&suppressed_data[0], sizeof(int16_t), size_sup, fp);
  fclose(fp);
  


  std::cout<< "Original number of elements in raw data: "<<n<<std::endl;
  std::cout<< "Number of triggers/peaks found: " <<triggercounter<<std::endl;
  std::cout<< "Number of elements in suppressed file: " <<size_sup<<std::endl;





#if defined(USE_GRAPHICS)
  //Plot gradient
  gROOT->SetStyle("ATLAS");
  TCanvas* c = new TCanvas("c");

  unsigned long int min = 255998800;
  unsigned long int max = 256000090;

  double x1 = tadc*min;
  double x2 = tadc*max;
  double xx1[2]={x1,x2};
  double yy1[2]={-200, 1000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0],yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  //graph1->GetXaxis()->SetTitle("Trigger Time [clock ticks]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");


  TGraph* grad = new TGraph();
  TGraph* gsignal = new TGraph();
  TGraph* gsupsignal = new TGraph();

  
  unsigned long int limitplot=6000000; //plot raw data big file
  //unsigned long int limitplot=n; //plot raw data
  //unsigned long int limitplot=size_sup;//plot sup data

  //std::cout<<"limit plot: "<<limitplot<<std::endl;
  int b= 0;

  for(unsigned long int i=min;i<max;i++){
    //plot averaged gradient
    //grad->SetPoint(i,avADCtime[i],avgradient[i]);
    //grad->SetPoint(b,tadc*avADCtime[i],avgradient[i]);
    //plot gradient
    //grad->SetPoint(i,timeus[i],gradient[i]);
    //plot raw signal
    gsignal->SetPoint(b,timeus[i],ADC[i]);   
    //std::cout<<timeus[i]<<" "<<ADC[i]<<" "<<gradient[i]<<std::endl;
    //plot suppressed signal
    //gsupsignal->SetPoint(i,suppressed_time[i]*tadc,suppressed_data[i]);
    b++;
  }

  b=0;
  for(unsigned long int i=50000000;i<53687091;i++){
      grad->SetPoint(b,tadc*avADCtime[i],avgradient[i]);
      b++;
      if(avgradient[i]==-100){
      std::cout<<i<<" "<<tadc*avADCtime[i]<<" "<<avgradient[i]<<std::endl;
      }
      }


  for(unsigned long int i=0;i<size_sup;i++){
  //for(unsigned long int i=0;i<limitplot;i++){  
  //plot suppressed signal
    gsupsignal->SetPoint(i,suppressed_time[i]*tadc,suppressed_data[i]); 
  }

  //plot end of both files
  /*
    int sup=0;
    int raw=0;
    for(unsigned long int i=0;i<size_sup;i++){
    //last trigger time
    if(suppressed_time[i]>320044947){
      gsupsignal->SetPoint(sup,suppressed_time[i],suppressed_data[i]);
      std::cout<<i<<" "<<suppressed_time[i]<<" "<<suppressed_data[i]<<std::endl;
      sup++;}
  }
  for(unsigned long int i=0;i<n;i++){
    if(time[i]>320044947){
      gsignal->SetPoint(raw,time[i],ADC[i]);
      //std::cout<<i<<" "<<time[i]<<" "<<ADC[i]<<std::endl;
      raw++;}
      }*/

  //print last nprint elements
  /*int nprint=100000;
    for(int i=0;i<size_sup;i++){
    if(i>(size_sup-nprint)){
      std::cout<<suppressed_time[i]<<" clock ticks "<<suppressed_time[i]*tadc<<" us  "<<suppressed_data[i]<<std::endl;
      }  
    }*/



  gsignal->SetLineColor(kBlack);
  gsignal->Draw("same,l");

  grad->SetMarkerColor(kOrange);
  grad->SetLineColor(kOrange);
  grad->Draw("same,l");
  /*
  gsupsignal->SetLineColor(kPink);
  gsupsignal->SetMarkerColor(kPink);
  gsupsignal->SetMarkerStyle(6);
  gsupsignal->Draw("same,p");
  */

  //Threshold line for the gradient
  TLine *line=new TLine(xx1[0],threshold,xx1[1],threshold);
  line->SetLineColor(kBlue);
  line->Draw("same");


  TLatex latex;
  latex.SetTextSize(0.06);
  latex.DrawLatex(350,-1500,"200 kHz");

  auto legend = new TLegend(0.5,0.7,0.88,0.9);
  legend->AddEntry(gsignal,"Signal","l");
  legend->AddEntry(grad,"Gradient","l");
  legend->AddEntry(gsupsignal,"Suppressed Data","l");
  legend->AddEntry(line,"Threshold","l");
  legend->Draw("same");


  //c->Print("Gradientfluctuationlines.png");

  c->Modified();
  c->Update();

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
#endif

  return 0;



}

