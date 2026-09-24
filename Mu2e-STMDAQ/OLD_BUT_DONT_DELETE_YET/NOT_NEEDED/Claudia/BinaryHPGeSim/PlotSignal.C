#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"
#include<fstream>



using namespace std;

void PlotSignal() {

  gROOT->SetStyle("ATLAS");
  //All Range
  double xx1[2]={0, 100000};
  double yy1[2]={-4000, 100};
  
  //double xx1[2]={70000, 90000};
  //double yy1[2]={-2000, 100};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 100000);
  graph1->GetYaxis()->SetRangeUser(-4000, 100);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("ADC Counts");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");







  //Read and plot one macropulse of the binary file
  //Read all the file
  char file_name[] = "Data.bin";
  FILE *myFile = fopen(file_name, "rb");
  if (myFile==NULL)
    exit(1);

  //Size of file in bytes: lSize is the number of bytes in the .bin file
  fseek (myFile , 0 , SEEK_END);
  unsigned long lSize = ftell (myFile);
  std::cout<<"File size in bytes: "<<lSize<<std::endl;
  rewind (myFile);
  unsigned long numelements = lSize/2;


  //All the elements in first one slice
  unsigned long int sliceLen = 2032;
  int numSlices = 2;
  int extNumpackets = 394;
  int intNumpackets = 157;



  //Number of macropulses
  uint macroCount= 4;
  //Number of internal triggers per macropulse
  uint int_trig_num = 10;

  TGraph *grext_int = new TGraph ();

  unsigned long int vectorsize=macroCount*(extNumpackets*numSlices*sliceLen+int_trig_num*intNumpackets*numSlices*sliceLen);
  int16_t* pulse = new int16_t[sliceLen];
  vector<int16_t> pulses;
  vector<double> time;

  //for each macropulse
  for(int i=0;i<macroCount;i++){
    //Macropulse
    //Packet 0
    //Remove trigger and slice header for the first packet (16+8)
    fseek(myFile,24*2,SEEK_CUR);
    //Read slice
    fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
    for(int h=0;h<sliceLen;h++){
      pulses.push_back(pulse[h]);
    }
    
    //Remove slice header
    fseek(myFile,8*2,SEEK_CUR);
    //Read slice
    fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
    for(int h=0;h<sliceLen;h++){
      pulses.push_back(pulse[h]);
    }   

    //Packet 1....extNumpackets
    for(int k=1;k<extNumpackets;k++){
      //Jump slice header
      fseek(myFile,8*2,SEEK_CUR);
      //Read slice
      fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
      for(int h=0;h<sliceLen;h++){
	pulses.push_back(pulse[h]);
      }
      
      //Jump slice header
      fseek(myFile,8*2,SEEK_CUR);
      //Read slice
      fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
      for(int h=0;h<sliceLen;h++){
	pulses.push_back(pulse[h]);
      }
    }

    //Gap
    //Loop over all the triggers in the internalmode
    for(int j=0;j<int_trig_num;j++){
      //Packet 0
      //Remove trigger and slice header for the first packet (16+8)
      fseek(myFile,24*2,SEEK_CUR);
      //Read slice
      fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
      for(int h=0;h<sliceLen;h++){
	pulses.push_back(pulse[h]);
      }

      //Remove slice header
      fseek(myFile,8*2,SEEK_CUR);
      //Read slice
      fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
      for(int h=0;h<sliceLen;h++){
	pulses.push_back(pulse[h]);
      }

      //Packet 1....intNumpackets
      for(int k=1;k<intNumpackets;k++){
	//Jump slice header
	fseek(myFile,8*2,SEEK_CUR);
	//Read slice
	fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
	for(int h=0;h<sliceLen;h++){
	  pulses.push_back(pulse[h]);
	}

	//Jump slice header
	fseek(myFile,8*2,SEEK_CUR);
	//Read slice
	fread(&pulse[0], sizeof(int16_t), sliceLen, myFile);
	for(int h=0;h<sliceLen;h++){
	  pulses.push_back(pulse[h]);
	}

      }
    }
  }


  //Fill the time vector;
  // ADC samling frequency (Hz)
  const double fADC = 320*1e6;
  //Sampling time of ADC (microsec)
  const double tadc=1/(fADC*1e-6);

  for(int i=0;i<vectorsize;i++){
    //Time in microseconds
    time.push_back(i*tadc);
    //std::cout<<pulses.at(i)<<std::endl;
    grext_int->SetPoint(i,time.at(i),pulses.at(i));
  }


  grext_int->Draw("same,l");

}
