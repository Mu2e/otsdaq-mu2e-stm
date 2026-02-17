#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include<list>



using namespace std;

//Receive array with all headers and data stored in one macropulse(data[]), length of the array (dataNumTot),
//the number of packets, number of elements in one packet (packetLen)
//size of packet header (pHdrLen), trigger headers (tHdrLen)
//and ADC values

//Skip packet headers and leave the first Trigger header
void CreateBinary (int16_t** data, unsigned long int dataNumTot, int packetNum, unsigned int packetLen, uint pHdrLen, uint tHdrLen)
{
  //For each packet
  unsigned long int n = dataNumTot-packetNum*pHdrLen-(packetNum-1)*tHdrLen;
  cout<<"n= "<<n<<endl;
  int16_t* bin_data = new int16_t[n];
  unsigned long int h=0; //from 0 to n



  for(int packetn=0;packetn<packetNum;packetn++){
    unsigned long int j=0;
    unsigned long int i=0;
    std::cout<<"Packet "<<packetn<<std::endl;
    int sizep;
    if(packetn==0){sizep=packetLen-pHdrLen;}
    else{sizep=packetLen-pHdrLen-tHdrLen;}
    while(i<sizep){
      //Remove packet header (6bytes = 3 uint16_t->Packet Header)
      if(j<pHdrLen){ //3
	j++;
	continue;}
      if(packetn==0){
        //Keep trigger header
        for(unsigned int k=0;k<(packetLen-pHdrLen);k++){bin_data[h]=data[packetn][j];
          cout<<"j= "<<j<<" i= "<<i<<" data stored= "<<bin_data[h]<<endl;
          j++;
          i++;
          h++;}
      }
      else{j=j+tHdrLen; //16
        //Remove trigger headers
        for(unsigned int k=0;k<(packetLen-pHdrLen-tHdrLen);k++){bin_data[h]=data[packetn][j];
          cout<<"j= "<<j<<" i= "<<i<<" data stored= "<<bin_data[h]<<endl;
          j++;
          i++;
          h++;}
      }
    }
  }


  char file_name[] = "Data.bin";
  //Open in read mode
  FILE *myFile = fopen(file_name, "rb");

  //If the binary file doesn't exist create it and write the array into the binary file
  if(!myFile){
    // fclose(myFile);
    std::cout<<"First call, create and write to binary file"<<std::endl;
    FILE * fp = fopen(file_name, "wb");
    fwrite(&bin_data[0], sizeof(int16_t), n, fp );
    fclose(fp);
  }


  //Else if the binary file exists write the array at the end of the file
  if (myFile){
    //fclose(myFile);
    std::cout<<"Call, open and append data to binary file"<<std::endl;
    FILE * fp = fopen(file_name, "a+");
    fwrite(&bin_data[0], sizeof(int16_t), n, fp);
    fclose(fp);
  }

  return;
}







int main (void)
{
  unsigned long int dataNumTot=123;
  int packetNum=3;
  unsigned int packetLen=41;
  uint pHdrLen=3;
  uint tHdrLen=16;
  //Example
  //Packet 1
  //Packet Header {1,1,1} //Remove
  //Trigger Header {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {3,3,3}
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {3,3,3}

  //Packet 2
  //Packet Header {1,1,1} //Remove
  //Trigger Header {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2} //Remove
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {4,4,4}
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {4,4,4}

  //Packet 3
  //Packet Header {1,1,1} //Remove
  //Trigger Header {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2} //Remove
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {5,5,5}
  //Slice Header {8,8,8,8,8,8,8,8}
  //Data {5,5,5}



  int16_t** data = new int16_t*[3]();
  //3packets
  for(int p=0;p<3;p++){
    data[p] = new int16_t[41];
    std::cout<<"Packet: "<<p<<std::endl;
    for(int count=0;count<=41;count++){
      if(count<=2){data[p][count] = 1; std::cout<<data[p][count]<<std::endl;}
      if(count>2&&count<=18){data[p][count] = 2; std::cout<<data[p][count]<<std::endl;}
      if(count>18&&count<=26){data[p][count] = 8; std::cout<<data[p][count]<<std::endl;}
      if(count>26&&count<=29&&p==0){data[p][count] = 3; std::cout<<data[p][count]<<std::endl;}
      if(count>26&&count<=29&&p==1){data[p][count] = 4; std::cout<<data[p][count]<<std::endl;}
      if(count>26&&count<=29&&p==2){data[p][count] = 5; std::cout<<data[p][count]<<std::endl;}
      if(count>29&&count<=37){data[p][count] = 8; std::cout<<data[p][count]<<std::endl;}
      if(count>37&&count<=40&&p==0){data[p][count] = 3; std::cout<<data[p][count]<<std::endl;}
      if(count>37&&count<=40&&p==1){data[p][count] = 4; std::cout<<data[p][count]<<std::endl;}
      if(count>37&&count<=40&&p==2){data[p][count] = 5; std::cout<<data[p][count]<<std::endl;}

    }

  }


  CreateBinary(data,dataNumTot,packetNum, packetLen, pHdrLen, tHdrLen);

  CreateBinary(data,dataNumTot,packetNum, packetLen, pHdrLen, tHdrLen);

  CreateBinary(data,dataNumTot,packetNum, packetLen, pHdrLen, tHdrLen);


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

  int16_t r1[numelements];
  fread(&r1[0], sizeof(int16_t), numelements, myFile);
  std::cout<<"Read the file"<<std::endl;
  for(int i=0;i<numelements;i++){
    cout<<r1[i]<<endl;
  }

  fclose(myFile);

}
