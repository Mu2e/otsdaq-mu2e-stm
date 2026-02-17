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

void PlotMacroNumberTriggerTime() {

  gROOT->SetStyle("ATLAS");
  //All Range                                                                                                                               
  double xx1[2]={0, 4000000};
  double yy1[2]={0, 40};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0, 4000000);
  graph1->GetYaxis()->SetRangeUser(0, 40);
  graph1->GetXaxis()->SetTitle("Time [#mus]");
  graph1->GetYaxis()->SetTitle("Trigger Number");
  graph1->SetTitle("");
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");






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

  //All the elements per macropulse and internal triggers are n:
  //unsigned long int n = dataNumTot-packetNum*pHdrLen-(packetNum-1)*tHdrLen;
  //cout<<"n= "<<n<<endl;

  //Number of elements in the macropulse
  uint64_t dataNumTotExt=1607536;
  //Number of elements in gap
  uint64_t dataNumTotInt=640576;


  //Number of macropulses
  uint macroCount= 4;
  //Number of internal triggers per macropulse
  uint int_trig_num = 10;

  uint16_t macronumbers[macroCount];
  //the macronumber is two int16_t
  uint16_t storemacronumbers[2];

  uint16_t slice_macrotimes[macroCount];
  //the macrotime is 4 int16_t
  uint16_t storeslice_macrotimes[4];



  uint16_t int_trignumbers[macroCount*int_trig_num];
  //the internal triggernumber is two int16_t
  uint16_t storeint_trignumbers[2];
  uint16_t slice_inttrigtimes[macroCount*int_trig_num];
  //the internal trigger time is 4 int16_t
  uint16_t storeslice_inttrigtimes[4];



  //The macronumber is the two first int16_t in the packet number 0 of external mode (one per macropulse)
  uint32_t macronumber;
  //The macro time is the adc number read until the moment (adc1) times adc1
  //this is the third, fourth, fifth and sixth elements int16_t in the slice header
  uint64_t slice_macrotime;


  //The internal trigger number is the 8th and 9th int16_t element in the packet number 0 of internal mode (10 (int triggers) per macropulse)
  uint32_t int_trignumber;
  //The internal trigger time is the adc number read until the moment (adc1) times adc1
  //this is the third, fourth, fifth and sixth elements int16_t in the slice header
  uint64_t slice_inttrigtime;


  vector<int> xmacros, ymacros, xgaps, ygaps;

  //for each macropulse
  for(int i=0;i<macroCount;i++){
    //read first two elements, they are macronumber
    fread(&storemacronumbers[0], sizeof(uint16_t), 2, myFile);
    //Reconstruct the macronumber
    macronumber=(uint32_t)storemacronumbers[0] << 16 | (uint32_t)storemacronumbers[1];
    std::cout<<"Macronumber: "<<macronumber<<std::endl;
    ymacros.push_back(macronumber);
    //jump the 14 rest of elements in the trigger header and the two next of the slice header (16 elements)
    //Move offset bytes from position origin (SEEK_CUR for current position)
    //int fseek( FILE *stream, long offset, int origin );
    fseek(myFile,16*2,SEEK_CUR);


    //read the four following numbers, they are the slice time for the macro pulse
    fread(&storeslice_macrotimes[0], sizeof(uint16_t), 4, myFile);
    slice_macrotime=(uint64_t)storeslice_macrotimes[0] << 48 | (uint64_t)storeslice_macrotimes[1] << 32 | (uint64_t)storeslice_macrotimes[2] << 16 | (uint64_t)storeslice_macrotimes[3];
    xmacros.push_back(slice_macrotime/1000);
    std::cout<<"Slice Macrotime: "<<slice_macrotime/1000<<" us"<<std::endl;

    
    //jump all the values in the macropulse until gap from the beginning of the file (SEEK_SET)
    fseek(myFile,((i+1)*dataNumTotExt+i*int_trig_num*dataNumTotInt)*2,SEEK_SET);
    

    //loop over all the triggers in the internalmode
    for(int j=0;j<int_trig_num;j++){

      /*      if(i==1){      
	int n=100;                                                                                                                      
      uint16_t prueba[n];                                                                                                               
      fread(&prueba[0], sizeof(uint16_t), n, myFile);                                                                                   
      for(int i=0;i<n;i++){cout<<i<<" "<<prueba[i]<<endl;                                                                               
      }                                                                                                                               
      exit(0);

      }*/

      //jump the 7 first elements in the trigger header gap
      fseek(myFile,7*2,SEEK_CUR);


      //read 8th and 9th uint16_t element, they are internal trigger number
      fread(&storeint_trignumbers[0], sizeof(uint16_t), 2, myFile);
      //cout<<storeint_trignumbers[0]<<" "<<storeint_trignumbers[1]<<endl;

      //Put together the two numbers in one
      int_trignumber=(uint32_t)storeint_trignumbers[0] << 16 | (uint32_t)storeint_trignumbers[1];
      ygaps.push_back(int_trignumber);
      std::cout<<"Inttrignumber: "<<int_trignumber<<std::endl;


      //jump the 7 rest of elements in the trigger header gap and the two next of the slice header (9 elements)
      fseek(myFile,9*2,SEEK_CUR);



      //read the four following numbers, they are the slice time for the internal modes
      fread(&storeslice_inttrigtimes[0], sizeof(uint16_t), 4, myFile);
      slice_inttrigtime=(uint64_t)storeslice_inttrigtimes[0] << 48 | (uint64_t)storeslice_inttrigtimes[1] << 32 | (uint64_t)storeslice_inttrigtimes[2] << 16 | (uint64_t)storeslice_inttrigtimes[3];
      xgaps.push_back(slice_inttrigtime/1000);
      std::cout<<"Slice Inttrigtime: "<<slice_inttrigtime/1000<<" us"<<std::endl;

      //jump all the values in the internal trigger until the next internal trigger
      fseek(myFile,((i+1)*dataNumTotExt+i*int_trig_num*dataNumTotInt+(j+1)*dataNumTotInt)*2,SEEK_SET);


    }

  }


  //Macro pulses: External mode
  TGraph *grmacros = new TGraph (xmacros.size(), &xmacros[0], &ymacros[0]);
  grmacros->SetMarkerColor(kOrange);
  grmacros->SetMarkerStyle(20);



  //Gaps: Internal mode
  TGraph * grgaps = new TGraph(xgaps.size(), &xgaps[0], &ygaps[0]);
  grgaps->SetMarkerColor(kViolet+2);
  grgaps->SetMarkerStyle(20);
  grgaps->Draw("same,p");
  grmacros->Draw("same,p");


  auto legend = new TLegend(0.1,0.7,0.48,0.9);
  legend->AddEntry(grmacros,"External Mode","p");
  legend->AddEntry(grgaps,"Internal Mode","p");
  legend->Draw("same");
}
