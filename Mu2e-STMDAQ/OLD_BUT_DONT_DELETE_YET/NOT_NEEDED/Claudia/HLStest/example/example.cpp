#include "example.h"



void example(int16_data allchunk[chunk], out_stream_t &output)
{
  #pragma HLS reset variable=output
  
  #pragma HLS interface ap_fifo port=allchunk
  #pragma HLS interface ap_fifo port=output


  cout<<"------------Executing EXAMPLE function------------"<<endl;
  out_stream_data result;

  std::cout<<"------------OUTPUT FIFO"<<std::endl;
  for(int16_data i=0;i<chunk;i++){
    result.data=allchunk[i]+1;
    output << result;
  }

}
