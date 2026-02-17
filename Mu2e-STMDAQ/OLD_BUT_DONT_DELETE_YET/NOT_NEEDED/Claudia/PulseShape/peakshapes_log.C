#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TLatex.h"
#include<fstream>
#include <sstream> // std::stringstream

#include "Functions.h" 
using namespace std;


void peakshapes_log(std::string runnumber) {
  //runnumber="00","01","02"..."19"
  gROOT->SetStyle("ATLAS");
  double xx1[2]={0,302};
  //double xx1[2]={0,5};
  //double xx1[2]={1.5,3};  
  //double yy1[2]={-2500, 1000};
  double yy1[2]={-4000, 1500}; 
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(xx1[0],xx1[1]);
  graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  //graph1->Draw("ap");

  //////******CONFIG******//////
  string RUN="110"; //choose the run 

  bool savepeaks = false; //Create a png and pdf for each peak with the energy+-5keV chosen
  bool plotpeaks = true; //Plot each data peak with the energy+-5keV chosen and simulated peak
  
  bool createbinary = false; //Create a binary file with the ADC values of the peak
  bool plotstoredpeak = false; //Plot the stored peak from the binary file created with the peak
  //////******************//////
  
  Functions fFunctions;

  //const double fADC = 320.0520833313;
  const double fADC = 370;
  //Sampling time of ADC (microsec)
  const double tadc=1/(fADC);
  
  //READ LOG FILE AND STORE ENERGIES AND TRIGGER TIMES FOR EACH PEAK
  std::string logfile="/work/cgarcia/DATA/MWD_Analysis/RUN"+RUN+"/M400L200/logrun"+RUN+"_"+runnumber;

  std::cout << "Read energies from filename = " << logfile<< std::endl;

  std::vector<double> peak_En, peak_trigtime;
  std::vector<int> peak_num;

  std::ifstream mylogFile;
  mylogFile.open(logfile);

  //Read lines
  std::string line;
  int countline = 0;
  double trigtime, energy;

  std::string val;

  while(std::getline(mylogFile, line)){
    std::stringstream ss(line);
    //skip two first lines
    if (countline!=0&&countline!=1){ss >> val; ss >> val; ss >> val; ss>>trigtime; ss >> val; ss >> val; ss >> val; ss >> val; ss >> val; ss>>energy;
    peak_trigtime.push_back(trigtime);
    peak_En.push_back(energy);
    //last two elements are wrong because of the last two lines in the log file
    //std::cout<<trigtime<<" "<<energy<<std::endl;
    }
    countline++;
  }


  //CHOOSE THE ENERGY+-5KEV FOR THE PEAKS THAT WE WANT TO PRINT AND STORE ITS ENERGY AND TRIGGER TIMES
  double energylim;
  //Show peaks around this energy
  std::cout<<"List peaks around energy (+-50keV): "<<std::endl;
  std::cin>>energylim;
  //vectors to store just the trig time and energy around the energy selected
  std::vector<double> en, tt;

  int j=0;
  for(long unsigned int i=0;i<(peak_En.size()-2);i++){
    if((peak_En.at(i)>(energylim-50))&&(peak_En.at(i)<(energylim+50))){
      std::cout<<"Peak N: "<<j<<" Trigger time: "<<peak_trigtime.at(i)<<" Energy: "<<peak_En.at(i)<<std::endl;
      tt.push_back(peak_trigtime.at(i));
      en.push_back(peak_En.at(i));

      j++;}
  }


  //READ THE BINARY FILE WITH THE DATA
  std::string filename="/work/cgarcia/DATA/MWD_Analysis/RUN"+RUN+"/run00"+RUN+".new.bin_"+runnumber;
  
  std::cout << "Plot peaks from filename = " << filename<< std::endl;

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile;
  myFile.open(filename, std::ios::in | std::ios::binary);
  int16_t inf;
  //TGraph* gr = new TGraph();
  int ADCread=0;

 
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) )){
    ADC.push_back(inf);
  }


  //plot 302us per pulse in ADC counts
  int sizepeak=302*fADC;





  //STORE PEAK TO BINARY FILE AND PLOT IT OR JUST PLOT IT FROM THE BINARY FILE
  if(plotstoredpeak==true){
    int peaknum;
    double micro;

    while(1){
    cout<<"Choose a peak number from the list to store in a binary file: "<<std::endl;
    cin>>peaknum;
    
    cout<<"Plot and save to binary file x microseconds before the trigger time: "<<std::endl;
    cin>>micro;

    std::string output_filename = "/work/cgarcia/DATA/Claudia/PulseShape_Analysis/Shapes/run"+RUN+"_"+runnumber+"_peak"+std::to_string(peaknum)+"_trigtime.bin";
    
if(createbinary==true){
    std::ofstream output_file;
    output_file.open(output_filename, std::ios::out | std::ios::binary);
    
    //Copy the ADC values of the pulse in a new vector
    std::vector<int16_t> data_element;
    data_element.clear();
    
    std::cout<<"Trigger time for this peak: "<<tt.at(peaknum)<<" us"<<std::endl;
    int startpeak=(tt.at(peaknum)-micro)*fADC; //storing micro us before the trigger time
    int endpeak=startpeak+sizepeak;

    
    for(int i=startpeak;i<endpeak;i++){
      data_element.push_back(ADC.at(i));
    }

    //Write the binary file
    for (int16_t element : data_element){
      output_file.write((char *) &element, sizeof(element));
    }

    std::cout<<"Binary file created with path and name: "<<output_filename<<std::endl;
    output_file.close();
 }//if create binary

    //Read the binary file and plot it

    std::cout<<"Plotting from: "<<output_filename<<std::endl;
    
    std::ifstream myFilepeak;
    myFilepeak.open(output_filename, std::ios::in | std::ios::binary);
    int16_t peakADC;
    int b=0;
    TGraph* gr_peak = new TGraph();

    while( myFilepeak.read( reinterpret_cast<char*>( &peakADC ), sizeof(peakADC) )){
      gr_peak->SetPoint(b,b*tadc,peakADC-920); //To restore the baseline to 0 
      b++;
    }

    TCanvas *canvas0 = new TCanvas("canvas0");
    graph1->Draw("ap");
    gr_peak->SetLineColor(kBlack);
    gr_peak->SetMarkerColor(kBlack);
    gr_peak->SetMarkerStyle(6);
    gr_peak->Draw("same,p");

    canvas0->Update();
    canvas0->Modified();
    canvas0->WaitPrimitive();

    myFilepeak.close();
    }//while(1)
  }//if plotstoredpeaks








 

  //LOOP TO PRINT PEAKS
  if(plotpeaks==true){

  long unsigned int peaknumber=0;
  
  //while(peaknumber!=10000){
  while(peaknumber<tt.size()){  
  
    //cout<<"Choose a peak number from the list to plot (type 10000 to exit)"<<std::endl;
    //cin>>peaknumber;

  TGraph* gr = new TGraph();

  //std::vector<int16_t> gradient;
  TGraph* grad = new TGraph();

  //plot with 3us of margin before the trigger time
  long unsigned int startplot=(tt.at(peaknumber)-3)*fADC;
  long unsigned int endplot=startplot+sizepeak; 
  int index=0;

  std::cout<<"Plotting from element: "<<startplot<<" to "<<endplot<<std::endl;
  std::cout<<"Number of elements: "<<ADC.size()<<std::endl;

  
  if(startplot>=ADC.size()){std::cout<<"This peak exceeds file size"<<std::endl;}
  if(endplot>ADC.size()){endplot=ADC.size();}
  

  unsigned long int window=100;
  for(long unsigned int i=startplot;i<endplot;i++){
    gr->SetPoint(index,index*tadc,ADC.at(i)-920); //To restore the baseline to 0
    //Plot gradient
    grad->SetPoint(index,index*tadc,ADC.at(i+window)-ADC.at(i));

    index++;
  }

  grad->SetLineColor(kOrange);
  grad->SetMarkerColor(kOrange);
  grad->SetMarkerStyle(6);


  gr->SetLineColor(kBlack);
  gr->SetMarkerColor(kBlack);
  gr->SetMarkerStyle(6);



    //SIMULATED PEAK
    //Plot one simulated peak with simulated noise= 0.32mV, fadc=370 and Cs energy on top of the data
    //Read first 302 elements from suppressed file
    std::string  filenamesim  = std::string("/work/cgarcia/DATA/Claudia/PulseShape_Analysis/SupSim1kHz/SupdataNoise_01kHz_fadc370_noise0.32.bin");
    std::cout << "Simulated peak filename = " << filenamesim << std::endl;

    std::vector<int16_t> ADCsim;
    ADCsim.clear();
    std::ifstream myFilesim;
    myFilesim.open(filenamesim, std::ios::in | std::ios::binary);
    int16_t infsim;

    TGraph* grsim = new TGraph();
    
    int readmax=0;

    while( myFilesim.read( reinterpret_cast<char*>( &infsim ), sizeof(infsim) )&&(readmax<=sizepeak)){
      ADCsim.push_back(infsim);
      readmax++;
    }

    std::cout<<"Number of elements of all sim data: "<<ADCsim.size()<<std::endl;
    
    int nADCpeak=ADCsim.size();

    //Generate noise with random numbers following a gaussian, the input parameter is noiseSD mV which is the standard deviation of the gaussian in mV
    double sigma_noise_ADC=fFunctions.noiseSD*38.5;
    std::cout<<"NoiseSD: "<<fFunctions.noiseSD<<" mV = "<<sigma_noise_ADC<<" ADC Counts, xshift= "<<fFunctions.xshift<<std::endl;
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0,sigma_noise_ADC);



    for(int i=0;i<nADCpeak;i++){
      //Read from file
      //grsim->SetPoint(i,i*tadc,ADCsim.at(i));
      //Call Joe's function
      double noise=distribution(generator);
      grsim->SetPoint(i,i*tadc,fFunctions.pulseCalc_Joe(i*tadc,2*energylim/0.57)+noise);
    }



    grsim->SetLineColor(kAzure+1);
    grsim->SetMarkerColor(kAzure+1);
    grsim->SetMarkerStyle(6);


    TCanvas *canvas0 = new TCanvas("canvas0");
    
    graph1->Draw("ap");  
    //grsim->Draw("same,p");
    gr->Draw("same,p");
    //grad->Draw("same,p");



    //Falling edge fitting from data
    double rangex1=2;//us
    double rangex2=2.5;//us
    //TF1*Fit1 = new TF1("Fit1", "(-1)*[0]*( 1.0  - ( 1.0/ (1 + exp((-1)*(x - [1])*[2]))))", rangex1, rangex2);
    TF1*Fit1 = new TF1("Fit1", "(-1)*([0] / (1 + exp((-1)*(x - [1])*[2])) )", rangex1, rangex2);
    Fit1->SetParameters(2*(energylim/0.57),1.1,15);
    gr->Fit(Fit1,"0","",rangex1,rangex2);
    Fit1->SetLineColor(kRed);
    Fit1->SetLineStyle(2);
    //Fit1->Draw("same");

    //Decay fitting from data
    double rangex3=5;//us
    double rangex4=200;//us
    //preamp equation
    //TF1*Fit2 = new TF1("Fit2", "[0]*(1/[1]+(1-1/[1])*exp(([2]-x)/[3]))", rangex3, rangex4);  
    //Fit2->SetParameters(1052,10000000000,0.2,50);
    TF1*Fit2 = new TF1("Fit2", "[0]*exp(([1]-x)/[2])", rangex3, rangex4);
    Fit2->SetParameters(-1185,0.2,50); 
    gr->Fit(Fit2,"1","",rangex3,rangex4);
    Fit2->SetLineColor(kRed);
    Fit2->SetLineStyle(2);
    Fit2->Draw("same");



    auto legend = new TLegend(0.5,0.4,0.88,0.6);
    //legend->AddEntry(grsim,"Simulation","l");
    //legend->AddEntry(gr,"Data","l");
    //legend->AddEntry(Fit1,"Fit","l");
    //legend->AddEntry(grad,"Gradient","l");
    //legend->AddEntry(Fit2,"#tau#approx59.9 #mus","l");      
    //legend->Draw("same");
    

    //SAVE PLOT FOR EACH PEAK
    if(savepeaks==true){
    std::string pdf="log"+runnumber+"_peak"+std::to_string(peaknumber)+"_zoom.pdf";
    std::string png="log"+runnumber+"_peak"+std::to_string(peaknumber)+"_zoom.png";

    char* pngchar = const_cast<char*>(png.c_str());
    char* pdfchar = const_cast<char*>(pdf.c_str());

    canvas0->Print(pngchar,"png");
    //canvas0->Print(pdfchar,"pdf");
    }

    //comment this if use screen cin 
    canvas0->Update();
    canvas0->Modified();
    canvas0->WaitPrimitive();
   
    peaknumber++;
    ////
  }

  std::cout<<"exit loop"<<std::endl; 
  }//if plotpeaks
}
