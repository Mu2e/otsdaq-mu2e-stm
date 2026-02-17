#include "STMDAQ-TestBeam/utils/BinaryFile.hh"
#include <string> 
#include <sstream>                                                                     

int main(){  
	BinaryFile *bf = new BinaryFile();
	bf->open_input_file("data/stm_hzdr_raw__run0000088_subrun00000.bin");
	unsigned int filesize = bf->fsize();
	std::cout << "File: " << bf->filename() << " Filesize = " << filesize << " bytes " << std::endl;

//  Read the whole file
	unsigned int N = filesize/2;  // #int16 is filesize in bytes / 2 
	int16_t* data = new int16_t[N];
	bf->read_data(data,N); // read whole file
	for (unsigned int i = 0; i < 100; i++){
		std::cout << "data[" << i << "] = " << data[i] << std::endl;
	}

//  Read subset of file with a given offset (in bytes) from the start of the file
	unsigned int byte_offset_from_start = 2*512;
	N = 512; // # of int16 values to read  
	data = new int16_t[N];
	bf->read_data(data,N,byte_offset_from_start); // read subset of file
	for (unsigned int i = 0; i < 50; i++){
		std::cout << "data[" << i << "] = " << data[i] << std::endl;
	}

	bf->close_input_file();
	delete bf;
//
}

