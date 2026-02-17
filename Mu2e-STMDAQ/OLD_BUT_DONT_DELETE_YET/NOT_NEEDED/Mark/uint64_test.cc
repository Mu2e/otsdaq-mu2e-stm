#include <string> 
#include <fstream>
#include <sstream>                                                                     
#include <iomanip>                                                                     
#include <iostream>                                                                     
#include <bitset>                                              
#include <cstdio>
#include <cinttypes>
#include <assert.h>

void printi16a(int16_t* d, int N){
	std::cout << "\nint16[] array ... " << std::endl;
	for (int i = 0; i < N; i++){
		std::printf("\n%i x[%02x]",d[i],d[i]);
	}
	std::printf("\n\n");
}

void print64(uint64_t ui64,std::string mode){
	std::cout << mode << std::endl;
	unsigned char* ptr = (unsigned char*)&ui64;
	std::printf("Trigger Time %" PRId64, ui64);
	for (size_t i = 0; i < 8; i++)  // print each byte as 2 digit hex
    	printf(" %02x", ptr[i]);
}


void test_trigger_word(uint64_t trigger_time){
	unsigned int N = 9;
	int16_t* data_write = new int16_t[N]; 

    print64(trigger_time,"WRITE");
	// Copy trigger time to first 4 elements of data;
    std::memcpy(data_write, &trigger_time, sizeof trigger_time);
    // Add in some data spanning signed int16 range for checking
    data_write[4] = -32500;
    data_write[5] = -1;
    data_write[6] = 0;
    data_write[7] = 1;
    data_write[8] = 32500;
    printi16a(data_write,N);
    // Write to binary file
    std::ofstream opfile("test.bin", std::ios::out | std::ios::binary);
    opfile.write((char *) data_write, sizeof data_write[0]*N);
    opfile.close();


	int16_t* data_read = new int16_t[N]; 
    std::ifstream ipfile("test.bin", std::ios::in | std::ios::binary);
    ipfile.seekg(0, std::ios::beg);
    ipfile.read((char *) data_read, sizeof data_read[0]*N);
    ipfile.close();

    // Get the trigger_time from this array
	uint64_t* trigger_time_read = reinterpret_cast<uint64_t*>(data_read);
	uint64_t tt = *trigger_time_read;
	print64(tt,"READ");
	// Copy first 4 elements of data into the uint64_t
    printi16a(data_read,N);
}

int main(){  

// Do equivalent of sending and reading int16 array to socket but do it via a binary file

	test_trigger_word(1650015151557);
	std::cout << " ------------------ " << std::endl;
	test_trigger_word(1650015371685);

//trigTime = 1650015151557
//256   384   11409   9669

//BAD CASE
//trigTime = 1650015371685
//256   384   11412   33189


}
