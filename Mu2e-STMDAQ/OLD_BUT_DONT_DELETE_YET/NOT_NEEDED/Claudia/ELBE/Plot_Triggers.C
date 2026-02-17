#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include<fstream>
#include "TROOT.h"
#include "TLatex.h"

#include <stdio.h>

void Plot_Triggers(char file_name[]){

  gROOT->SetStyle("ATLAS");

  //auto c1= new TCanvas("c1","c1",1200,600);
  auto c1= new TCanvas("c1"); 
 
  bool print=true;


  //Read all the file
  //char file_name[] = "/work/cgarcia/DATA/ELBETestBeam/stm_hzdr_raw__run0000023_subrun00000.bin";
  //char file_name[] = "/work/markl/STMDAQ-TestBeam/Mark/data/stm_hzdr_raw__run0000009_subrun00000.bin";
  
  FILE *myFile = fopen(file_name, "rb");
  if (myFile==NULL)
    exit(1);

  //Size of file in bytes: lSize is the number of bytes in the .bin file
  fseek (myFile , 0 , SEEK_END);
  unsigned long lSize = ftell (myFile);
  std::cout<<"File size in bytes: "<<lSize<<std::endl;
  rewind (myFile);

  uint32_t* deadbeef = new uint32_t[1];

  uint32_t* data_size = new uint32_t[1]();
  std::vector<uint32_t> data_sizev;

  uint32_t* numslices = new uint32_t[1]();
  std::vector<uint32_t> numslicesv;

  uint32_t* trig_num = new uint32_t[1]();
  std::vector<uint32_t> trig_numv;

  uint16_t* MdChTp = new uint16_t[1]();
  std::vector<uint16_t> Channel;
  std::vector<uint16_t> Mode;
  std::vector<uint16_t> ModeExt;
  std::vector<uint16_t> ModeInt;

  uint64_t* trig_time = new uint64_t[1]();
  std::vector<uint64_t> trig_timev;

  uint32_t* ADCoffset = new uint32_t[1]();
  std::vector<uint32_t> ADCoffsetv;

  uint16_t* drop_packets = new uint16_t[1]();
  std::vector<uint16_t> drop_packetsv;

  uint64_t* unix_time = new uint64_t[1]();
  std::vector<uint64_t> unix_timev;



  uint32_t* slice_num = new uint32_t[1]();
  std::vector<uint32_t> slice_numv;

  uint32_t* slice_size = new uint32_t[1]();
  std::vector<uint32_t> slice_sizev;

  uint64_t* ADCtime = new uint64_t[1]();
  std::vector<uint64_t> ADCtimev;



  std::vector<int16_t> ADCDatav;


  //Time
  //Plot Raw data
  //Sampling time of ADC 
  const double fADC = 320.0520833313; //MHz
  const double tadc = (1./fADC); //us
  std::vector<double> timev;

  const double clocktrigfreq = 13; //MHz 
  const double adc_offset_time_clock = 125; //MHz

  TGraph *gr = new TGraph (); 
 

  unsigned long totalbytesread = 0;
  unsigned long tHdrBytes = 40;
  unsigned long sHdrBytes = 16;
  unsigned long totaldatabytes = 0;

  //Bytes to read lSize to read all file
  unsigned long bytestoread=lSize;
  //Number of triggers in binary file
  int trigger=0;
  int k=0;

  //Header structure -- true if old structure (no unix time)
  //old header structure from Run  01 to 10 (true)
  //new header structure from Run 15 to 28 (false)
  bool oldheaderstructure=false;
  if(oldheaderstructure==true){tHdrBytes = 32;}

  //Code
  while(totalbytesread<bytestoread){
    if(print==true){
    std::cout<<"---Trigger Header---"<<std::endl;
    }
    //Read Trigger Header
    //Read deadbeef
    fread(&deadbeef[0], sizeof(uint32_t), 1, myFile);
    if(print==true){
    std::cout<<"deadbeef: "<<std::hex << deadbeef[0]<<std::dec<<std::endl;
    }
    fread(&data_size[0], sizeof(uint32_t), 1, myFile);
    data_sizev.push_back(data_size[0]);
    if(print==true){
    std::cout<<"Data Size "<<data_size[0]<<std::endl;
    }
    fread(&numslices[0], sizeof(uint32_t), 1, myFile);
    numslicesv.push_back(numslices[0]);
    if(print==true){
    std::cout<<"Num Slices "<<numslices[0]<<std::endl;
    }
    fread(&trig_num[0], sizeof(uint32_t), 1, myFile);
    trig_numv.push_back(trig_num[0]);
    if(print==true){
      std::cout<<"Trig Num "<<trig_num[0]<<std::endl;
    } 
    fread(&MdChTp[0], sizeof(uint16_t), 1, myFile);
    // Get the channel (HPGe/LaBr)
    uint16_t channel = ((uint16_t)MdChTp[0] >> 8) & 0xF;
    // Get the mode (0 = external, 1 = internal)
    uint16_t mode = ((uint16_t)MdChTp[0] >> 12) & 0xF;
    Channel.push_back(channel);
    //External
    if(mode==0){ModeExt.push_back(mode);}
    //Internal
    if(mode==1){ModeInt.push_back(mode);}
    Mode.push_back(mode);
    if(print==true){
    std::cout<<"Channel "<<channel<<std::endl;
    std::cout<<"Mode "<<mode<<std::endl;
    }

    fread(&trig_time[0], sizeof(uint64_t), 1, myFile);
    trig_timev.push_back(trig_time[0]);

    if(print==true){
      std::cout<<"Trigger Time "<<trig_time[0]<<" clock ticks, "<<trig_time[0]/clocktrigfreq<<" us"<<std::endl;
    }
    fread(&ADCoffset[0], sizeof(uint32_t), 1, myFile);
    ADCoffsetv.push_back(ADCoffset[0]);
    if(print==true){
    std::cout<<"ADC Offset "<<ADCoffset[0]<<std::endl;
    }
    fread(&drop_packets[0], sizeof(uint16_t), 1, myFile);
    drop_packetsv.push_back(drop_packets[0]);
    if(print==true){
    std::cout<<"Dropped Packets "<<drop_packets[0]<<std::endl;
    }

    if(oldheaderstructure==false){
      fread(&unix_time[0], sizeof(uint64_t), 1, myFile);
      unix_timev.push_back(unix_time[0]);
      if(print==true){
	std::cout<<"Unix Time "<<unix_time[0]<<std::endl;
      }
    }
    if(print==true){
    std::cout<<"---Slice Header---"<<std::endl;
    }
    //Read Slice Header
    fread(&slice_num[0], sizeof(uint32_t), 1, myFile);
    slice_numv.push_back(slice_num[0]);
    if(print==true){
    std::cout<<"Slice Num "<<slice_num[0]<<std::endl;
    }
    fread(&slice_size[0], sizeof(uint32_t), 1, myFile);
    slice_sizev.push_back(slice_size[0]);
    if(print==true){
    std::cout<<"Slice Size "<<slice_size[0]<<std::endl;
    }
    fread(&ADCtime[0], sizeof(uint64_t), 1, myFile);
    ADCtimev.push_back(ADCtime[0]);
    if(print==true){
    std::cout<<"ADC time "<<ADCtime[0]<<std::endl;
    }
    uint32_t sizedat=slice_size[0];
    int   numelements=sizedat/2;
    int16_t* ADCData = new int16_t[sizedat]();

    //Read data
    fread(&ADCData[0], sizeof(int16_t), numelements, myFile);
    for(int i=0; i<numelements;i++){
      ADCDatav.push_back(ADCData[i]);
      //Add//
      if(trigger==4&&i<10){std::cout<<ADCData[i]<<std::endl;}
      //Until here//
    }

    
    
    totalbytesread= totalbytesread + tHdrBytes + sHdrBytes + sizedat;
    totaldatabytes = totaldatabytes + sizedat;
    trigger++;
   

  }

  std::cout<<"File size in bytes: "<<lSize<<std::endl;
  std::cout<<"Total bytes read, should be the same as file size: "<<totalbytesread<<std::endl;
  std::cout<<"ADCDatav.size()*2 in bytes: "<<ADCDatav.size()*2<<std::endl;
  std::cout<<"Total bytes of data in all slices, should be the same as ADCDatav.size()*2: "<<totaldatabytes<<std::endl;
  

  std::cout<<"Number of total triggers: "<<trigger<<std::endl;
  std::cout<<"Number of internal triggers: "<<ModeInt.size()<<std::endl;
  std::cout<<"Number of external triggers: "<<ModeExt.size()<<std::endl;





  bool plotpeaks = true;
  bool plottrignum = false;
  bool plotdroppedpackets = false;

  bool plotpeakstrigtime = false;
  bool plottrignum_extintmode = false;
  bool plot_Deltaextintmode = false;
  bool plotNoiseSignal = false;

  bool writetofilecsv = false;
  std::ofstream output_file;
  std::string output_filename = "stm_hzdr_raw_LaBr__run0000032_subrun00001.csv";

  if(writetofilecsv==true){
    output_file.open(output_filename);
  }

  
  bool writetofilebin = true;
  std::string output_filenamebin = "stm_hzdr_raw_LaBr__run0000032_subrun00001_noheaders.bin";
  int size = output_filenamebin.length();
  char output_filenamebin_char[size+1];
  strcpy(output_filenamebin_char, output_filenamebin.c_str());

  FILE * fp;
  
  if(writetofilebin==true){
    fp = fopen(output_filenamebin_char, "wb");
  }
  
  //Raw Data peaks
   if(plotpeaks==true){
     //HPGe
     //double xx1[2]={0, 1000};
     //double yy1[2]={-700,1000};
     //LaBr3
     double xx1[2]={0.7,1};
     double yy1[2]={-8000,4000};
     TGraph *graph1 = new TGraph (2,xx1,yy1);
     graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
     graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
     graph1->GetXaxis()->SetTitle("Time [#mus]");
     graph1->GetYaxis()->SetTitle("ADC Counts");
     graph1->SetMarkerStyle(1);
     graph1->Draw("ap");

     //int n = 10000000;    
     int n = 10000;  
     //int n = ADCDatav.size();
     for(int i=0; i<n;i++){
       timev.push_back(i*tadc);
     }

     int b=0;
     //int start = 400000000;    
     int start = 0;
     std::cout<<n<<std::endl;
     for(int j=0;j<n;j++){
       gr->SetPoint(b,timev.at(start+j),ADCDatav.at(start+j));
       if(writetofilecsv==true){
	 int16_t element = ADCDatav.at(start+j);
	 output_file << element << "\n";
       }
       b++;
     }

     //binary
     if(writetofilebin==true){
       fwrite(&ADCDatav[0], sizeof(int16_t), b, fp);
     }

      fclose(fp);
     
      output_file.close();
      
      gr->SetMarkerColor(kBlack);
      gr->SetLineColor(kViolet+2);
      gr->SetMarkerStyle(7);
      gr->Draw("same, lp");
     
   }



   //Trigger times and trigger number
   if(plottrignum==true){
     double xx1[2]={0, 400000000};
     double yy1[2]={0, 60};
     TGraph *graph1 = new TGraph (2,xx1,yy1);
     graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
     graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
     graph1->GetXaxis()->SetTitle("Trigger Time [clock ticks]");
     graph1->GetYaxis()->SetTitle("Trigger Number");
     graph1->SetMarkerStyle(1);
     graph1->Draw("ap");


     int n= trig_numv.size();
     std::cout<<n<<std::endl;

     for(int j=0;j<n;j++){
       gr->SetPoint(j,trig_timev.at(j),trig_numv.at(j));
     }
     gr->SetMarkerColor(kCyan+2);
     gr->SetMarkerStyle(8);
     gr->Draw("same, l");  
    
   }

  //Dropped packets 
   if(plotdroppedpackets==true){
     double xx1[2]={0,400};
     double yy1[2]={0, 80000};
     TGraph *graph1 = new TGraph (2,xx1,yy1);
     graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
     graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
     graph1->GetXaxis()->SetTitle("Trigger Number");
     graph1->GetYaxis()->SetTitle("Dropped Packets");
     graph1->SetMarkerStyle(1);
     graph1->Draw("ap");
 
     for(int j=0;j<trig_numv.size();j++){
       gr->SetPoint(j,trig_numv.at(j),drop_packetsv.at(j));
     }

     gr->SetLineWidth(3);
     gr->SetMarkerColor(kOrange+2);
     gr->SetMarkerStyle(7);
     gr->Draw("same,p");
  }


   //Plot data with gaps (having into account trigger time)
   if(plotpeakstrigtime==true){
     //HPGe
     //Choose the number of the trigger to plot: 0 first external, 1 first internal, 2 second external...
     int triggernumberplot = 4;
     //Range in graph that we are going to plot
     double starttrig = trig_timev.at(triggernumberplot)/clocktrigfreq; //us
     std::cout<<starttrig<<std::endl;
     double endtrig = starttrig+100000; //us

     //Triggers to plot 
     double xx1[2]={starttrig, endtrig};
     double yy1[2]={-20000, 1000};
     TGraph *graph1 = new TGraph (2,xx1,yy1);
     graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
     graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
     graph1->GetXaxis()->SetTitle("Time [#mus]");
     graph1->GetYaxis()->SetTitle("ADC Counts");
     graph1->SetMarkerStyle(1);
     graph1->Draw("ap");

     int k=0;
     int n = trig_numv.size();


     //Number of ADC values to skip
     //This gives the start index of the data we want to plot
     unsigned long int ADCskip = 0;
     for(int i=0;i<triggernumberplot;i++){
       ADCskip= ADCskip+slice_sizev.at(i)/2;
     }


     for(int i=triggernumberplot;i<n;i++){
       int n1 = slice_sizev.at(i)/2;
       
       std::cout<<"Trigger time: "<< trig_timev.at(i)/clocktrigfreq<<"us, ADC offset: "<<ADCoffsetv.at(i)/adc_offset_time_clock <<" us, External(0) or Internal(1): "<< Mode.at(i) <<", Slice size (ADC values): "<<n1<<std::endl;

       for(int j=0;j<n1;j++){
	 //n1<k
	 //Accounts for the number of points permitted in a TGraph
	 if(k<100000000){
	   gr->SetPoint(k,(trig_timev.at(i)/clocktrigfreq)+(ADCoffsetv.at(i)/adc_offset_time_clock)+j*tadc,ADCDatav.at(ADCskip+k));
	   //Add
	   if(k<10){std::cout<<ADCDatav.at(ADCskip+k)<<std::endl;}
	     //Until here
	   k++;
	 }
       }
     }

     gr->SetMarkerColor(kViolet+2);
     gr->SetLineColor(kViolet+2);
     gr->SetMarkerStyle(7);
     gr->Draw("same,p");
   }



   //Plot trigger number and external and internal mode timings
   if(plottrignum_extintmode==true){
     
     double xx1[2];
     xx1[0]=0;
     xx1[1]=trig_numv.at(trig_numv.size()-1) +10;
     double yy1[2];
     yy1[0]=trig_timev.at(xx1[0])/clocktrigfreq;
     yy1[1]=trig_timev.at(trig_numv.size()-1)/clocktrigfreq + 10000000;
     
     TGraph *graph1 = new TGraph (2,xx1,yy1);
     graph1->GetXaxis()->SetRangeUser(xx1[0], xx1[1]);
     graph1->GetYaxis()->SetRangeUser(yy1[0], yy1[1]);
     graph1->GetXaxis()->SetTitle("Trigger Number");
     graph1->GetYaxis()->SetTitle("Trigger Time [clock ticks/13 MHz = #mus]");
     graph1->SetMarkerStyle(1);
     graph1->Draw("ap");

     TGraph *gr_ext = new TGraph ();
     TGraph *gr_int = new TGraph ();

     int n= trig_numv.size();
     std::cout<<n<<std::endl;

     int kext=0;
     int kint=0;

     for(int j=0;j<n;j++){
       //External
       if(Mode.at(j)==0){
	 gr_ext->SetPoint(kext,trig_numv.at(j),trig_timev.at(j)/clocktrigfreq);
	 std::cout<<"Mode: "<< Mode.at(j)<<" Trig Num: "<<trig_numv.at(j)<<" Trig Time: "<<trig_timev.at(j)/clocktrigfreq<<" us, ADC offset: "<<ADCoffsetv.at(j)/adc_offset_time_clock<<" us"<<std::endl; 
	 kext++;
       }
       //Internal
       if(Mode.at(j)==1){
	 gr_int->SetPoint(kint,trig_numv.at(j),trig_timev.at(j)/clocktrigfreq);
	 std::cout<<"Mode: "<< Mode.at(j)<<" Trig Num: "<<trig_numv.at(j)<<" Trig Time: "<<trig_timev.at(j)/clocktrigfreq<<" us, ADC offset: "<<ADCoffsetv.at(j)/adc_offset_time_clock<<" us"<<std::endl;
	 kint++;
       }
     }

     

     gr_ext->SetMarkerColor(kCyan+2);
     gr_ext->SetMarkerStyle(8);
     gr_ext->Draw("same,p");

    
     gr_int->SetMarkerColor(kOrange);
     gr_int->SetMarkerStyle(8);
     gr_int->Draw("same,p");

     auto legend = new TLegend(0.1,0.7,0.48,0.9);
     legend->AddEntry(gr_ext,"External Mode","p");
     legend->AddEntry(gr_int,"Internal Mode","p");
     legend->Draw("same");   
   }
  
 




   if(plot_Deltaextintmode==true){
     
     double xmin =-1e+6 ;
     double xmax = 1.4e+7;
     TH1F*hextint = new TH1F("hextint","", 100, xmin, xmax);
     TH1F*hexts = new TH1F("hexts","", 100, xmin, xmax);
     hextint->GetXaxis()->SetTitle("#DeltaTrigger Times [clock ticks/13 MHz = #mus]");
     hexts->GetXaxis()->SetTitle("#DeltaTrigger Times [clock ticks/13 MHz = #mus]");
     //hextint->GetYaxis()->SetRangeUser(0,50);
     //hexts->GetYaxis()->SetRangeUser(0,80);

     std::vector<uint64_t> trig_timeExtv,trig_timeIntv;
     int n= trig_numv.size();
     std::cout<<n<<std::endl;
     for(int j=0;j<n;j++){
       //External
       if(Mode.at(j)==0){
	 trig_timeExtv.push_back(trig_timev.at(j));
	 std::cout<<"Mode: "<< Mode.at(j)<<" Trig Num: "<<trig_numv.at(j)<<" Trig Time: "<<trig_timev.at(j)/clocktrigfreq<<" us"<<std::endl;
       }
       //Internal
       if(Mode.at(j)==1){
	 trig_timeIntv.push_back(trig_timev.at(j));
	 std::cout<<"Mode: "<< Mode.at(j)<<" Trig Num: "<<trig_numv.at(j)<<" Trig Time: "<<trig_timev.at(j)/clocktrigfreq<<" us"<<std::endl;
       }
     }

     //Externals
     for(int i = 1; i<trig_timeExtv.size();i++){
       std::cout<<i<<" Subtract: "<<trig_timeExtv.at(i)/clocktrigfreq<<" - "<<trig_timeExtv.at(i-1)/clocktrigfreq<<std::endl;
       double difext=(trig_timeExtv.at(i)-trig_timeExtv.at(i-1))/clocktrigfreq;
       std::cout<<"Ext-Ext "<<difext<<" us"<<std::endl;
       hexts->Fill(difext);}

     std::cout<<" "<<std::endl;
     
     //Internals
     /* for(int i = 1; i<trig_timeIntv.size();i++){
       std::cout<<i<<" Subtract: "<<trig_timeIntv.at(i)/clocktrigfreq<<" - "<<trig_timeIntv.at(i-1)/clocktrigfreq<<std::endl;
       double difint=(trig_timeIntv.at(i)-trig_timeIntv.at(i-1))/clocktrigfreq; 
       std::cout<<"Int-Int "<<difint<<" us"<<std::endl;
       hextint->Fill(difint);}
     */
     //Internals and externals
      for(int i = 0; i<trig_timeIntv.size();i++){
       
       std::cout<<i<<" Subtract: "<<trig_timeIntv.at(i)/clocktrigfreq<<" - "<<trig_timeExtv.at(i)/clocktrigfreq<<std::endl;
       double difextint=(trig_timeIntv.at(i)-trig_timeExtv.at(i))/clocktrigfreq;
       std::cout<<"Int-Ext "<<difextint<<" us"<<std::endl;
       hextint->Fill(difextint);}


     hexts->SetLineColor(kOrange+2);
     hexts->Draw("");

     hextint->SetLineColor(kBlue+2);
     hextint->Draw("same");

     auto legend = new TLegend(0.1,0.7,0.48,0.9);
     legend->AddEntry(hexts,"Externals","f");
     legend->AddEntry(hextint,"Internal-External","f");
     legend->Draw("same");
   }




   //Plot noise of the signal in a histogram
   if(plotNoiseSignal==true){
     int nbins = 100;
     double xmin = 10;
     double xmax = 30;
     TH1F *hpeaks= new TH1F("BothModes", "", nbins, xmin, xmax);
     
     double timemax_plot = 150000; //us
     //double timemax_plot = 1000; //us  
     int timetoindex = int(timemax_plot*fADC);

     for(int j=0;j<timetoindex;j++){
       //std::cout<<ADCDatav.at(j)<<" "<<(ADCDatav.at(j)-0.354947)/(-1.75514)<<std::endl;
       //When we plot the energy spectrum the height of the peaks are negative so we have to use this convertion:
       //hpeaks->Fill((ADCDatav.at(j)-0.354947)/(-1.75514));
       //Now, the ADC values from the signal are positive so we use that 1ADC value is 0.57 keV
       //Or in mVolts 1mV is equal to 38.5 ADC count
       hpeaks->Fill(ADCDatav.at(j)/(38.5)); //this is in mV 
     }

          
     //Normalize the y axis to the number of entries
     TH1*hnorm = (TH1*)(hpeaks->Clone("hnorm"));
     hnorm->Scale(1./hnorm->Integral());
     hnorm->SetFillColor(kOrange-3);
     hnorm->SetLineColor(kOrange-3);
     //hnorm->GetYaxis()->SetRangeUser(0,0.08);
     hnorm->GetXaxis()->SetTitle("Noise [mV]");
     //DRAW NORMALIZED HISTOGRAM
     hnorm->Draw("HIST");
     TLatex latex;
     latex.SetTextSize(0.06);
     latex.SetTextColor(kBlue);
     latex.DrawLatex(13,0.01,"#sigma=0.32 mV");
     latex.DrawLatex(13,0.05,"#mu=18.19 mV");
     TLatex latex1;
     latex1.SetTextSize(0.04);
     latex1.DrawLatex(13,0.03,"Run27 - Beam Off, Source Out");



     TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", 12, 25);
     Fit->SetParameters(1,20,2);
     hnorm->Fit(Fit,"0","",12,25);
     Fit->SetLineColor(kRed);
     Fit->SetLineStyle(2);
     Fit->Draw("same");
     
   }

   






   //Fit singnal
  //Tau Value HPGe
  /*TF1*Fit1 = new TF1("Fit1", "[0]*exp(-x/[1])+[2]", 230, 650);
  Fit1->SetParameters(-123890 , 56.3977,  726.584);
  gr->Fit(Fit1,"0","",240, 650);
  Fit1->SetLineColor(kRed);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same");
  */

  //Tau value LaBr3
  /*TF1*Fit1 = new TF1("Fit1", "x/[0]+[1]", 53.19,53.22);
  Fit1->SetParameters(0.04,1337.91);
  gr->Fit(Fit1,"0","",53.19,53.22);
  Fit1->SetLineColor(kRed);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same");
  */

  /*TF1*Fit1 = new TF1("Fit1", "[0]*exp(-x/[1])+[2]",fit1,fit2);
  Fit1->SetParameters(-1.3e66,0.04,-4000);
  gr->Fit(Fit1,"0","",fit1,fit2);
  Fit1->SetLineColor(kRed);
  Fit1->SetLineStyle(2);
  Fit1->Draw("same");
  */
 

   auto legend = new TLegend(0.1,0.7,0.48,0.9);
   //legend->AddEntry(Fit1,"#tau#approx56.4 #mus","l");
   //legend->AddEntry(Fit1,"#tau#approx0.038 #mus","l"); 
   //legend->Draw("same");

   c1->Print("PeakLaBr3_pointline.png","png");
   //c1->Print("Run18_SeeGaps.pdf","pdf"); 
  
}
