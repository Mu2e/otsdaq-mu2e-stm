#include <string> 
#include <fstream>
#include <sstream>                                                                     
#include <iomanip>                                                                     
#include <iostream>                                                                     
#include <bitset>                                              
#include <cstdio>
#include <cinttypes>
#include <assert.h>

/*
# Binary File Header Structure Per Trigger
# uint64_t bits: 0-31 (0xDEADBEEF) || 32-63: data size 
# uint32_t trigger number (external or internal)
# uint16_t mode + channel + data type
# uint64_t trigger time
# uint32_t ADC offset 
# uint16_t number of dropped packets
# Missing number of slices
*/

void printi16a(int16_t* d, int N){
    std::cout << "\nint16[] array ... " << std::endl;
    for (int i = 0; i < N; i++){
        std::printf("\n[%i] %i x[%02x]",i,d[i],d[i]);
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

void pack_uint32_in_int16_array(int16_t* a16, uint32_t ioffset, uint32_t i32){
    uint16_t ua = (uint16_t) (i32 >> 16);
    uint16_t ub = (uint16_t) (i32 & 0x0000FFFFuL);

    a16[ioffset] = (int16_t)ua;
    a16[ioffset+1] = (int16_t)ub;

}

void generate_header(int16_t* header_data, uint16_t trigger_number, uint16_t trigger_type, 
    uint64_t trigger_time, uint32_t data_size, uint16_t header_size){

    uint32_t header_identifier  = 0xBEEFDEAD;
    // pack_uint32_in_int16_array(header_data, (uint32_t) 0, x);
    std::memcpy( &header_data[0], &header_identifier, sizeof( header_identifier ) );
    std::memcpy( &header_data[2], &trigger_number, sizeof( trigger_number ) );
    std::memcpy( &header_data[3], &trigger_type, sizeof( trigger_type ) );
    std::memcpy( &header_data[4], &trigger_time, sizeof( trigger_time ) );
    uint32_t payload_size = uint32_t (data_size) + (uint32_t) (header_size); // in bytes not int16
    std::memcpy( &header_data[8], &payload_size, sizeof( payload_size ) );

//    pack_uint32_in_int16_array(header_data, (uint32_t) 2, size_to_next_header);
}

int main(){
    uint16_t header_size = 10;
    int16_t* header_data = new int16_t[header_size];
    generate_header(header_data, 
            (uint16_t) 0x8010,             // trigger number (16) [2] ([0,1] = DEADBEEF)
            (uint16_t) 0x1,                // trigger type (16) [3]
            (uint64_t) 0x87654321FEDCBA98, // trigger time (64) [4,5,6,7]
            (uint32_t) 0xABCD1234,         // data size (32) [8,9]
            (uint16_t) 2*header_size);     // header size (16) in bytes
    printi16a(header_data,10);
}
