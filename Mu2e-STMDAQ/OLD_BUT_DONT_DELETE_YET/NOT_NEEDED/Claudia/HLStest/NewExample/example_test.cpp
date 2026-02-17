#include "example.h"

int main() {
  hls::stream<int> s_in[M], s_out[M];
  for (unsigned j = 0; j < M; j++) {
    for (unsigned i = 0; i < 10; i++) {
      s_in[j].write(i);
    }
  }

  auto ret = dut(s_in, s_out);

  std::cout << "ret = " << ret << std::endl;

  /*for (unsigned j = 0; j < M; j++) {
    for (unsigned i = 0; i < 10; i++) {
      int read_out= s_out[j].read();
      std::cout<< read_out<<std::endl;
      if (read_out != i+2)
    		return 1;
    }
    }*/

  for (unsigned j = 0; j < M; j++) {
    for (unsigned i = 0; i < 10; i++) {
      if (s_out[j].read() != i+2)
    	  return 1;
    }
  }



  return 0;
}
