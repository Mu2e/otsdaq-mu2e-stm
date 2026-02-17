
#include "example.h"

int main() {

  out_stream_t s_out;
  //in_stream_t s_in;
  int16_data allchunk[chunk];
  out_stream_data result;

  //SIM
  for(int16_data j = 0; j < chunk; j++){
    allchunk[j]=j;
    //s_in.data1=j;  
}


  example(allchunk,s_out);

  std::cout<<"Return: "<<std::endl;
  for (int16_data j = 0; j < chunk; j++) {
    result =s_out.read();
    std::cout<<result.data<<std::endl;
    
    //alldata[j]= result.data;
    //outFILE<<"index: "<<j<<", "<<alldata[j]<<endl;
    //bitset<16> x0(alldata[j]);
  }
  return 0;
}
