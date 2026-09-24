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

void readFile() {

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
  int numelements=lSize/2;


  rewind (myFile);
  int n=4096;
  int16_t r1[n];

  fread(&r1[0], sizeof(int16_t), n, myFile);
  cout<<"Packet 0"<<endl;
  for(int i=0;i<n;i++){
    cout<<i<<" "<<r1[i]<<endl;
  }
  fseek(myFile,4080*2,SEEK_CUR);
  cout<<"Packet 2"<<endl;

  int n2=4080;
  int16_t r2[n];
  fread(&r2[0], sizeof(int16_t), n2, myFile);
  for(int i=0;i<n2;i++){
    cout<<i<<" "<<r2[i]<<endl;
  }

  cout<<"Packet 3"<<endl;
  int n3=2;
  int16_t r3[n];
  uint32_t slicenum;
  fread(&r3[0], sizeof(int16_t), n3, myFile);
  slicenum=(uint32_t)r3[0] << 16 | (uint32_t)r3[1];
  cout<<"Slice number theory: "<<6<<" Reco: "<<slicenum<<endl;



  //Trigger time of the second macropulse
  




  fclose(myFile);
}
