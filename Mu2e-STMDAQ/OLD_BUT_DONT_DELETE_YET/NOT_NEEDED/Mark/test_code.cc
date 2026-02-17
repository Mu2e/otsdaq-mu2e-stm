// Code to test XML interface and Binary File Code.
// Tries to emulate MIDAS structure ie with beginRun(),endRun(),frontendInit() etc
// - so should be able to take this and just slot it in. Needs Xml.hh and BinaryFile.hh
// Below it does two MIDAS Runs with 1000 triggers. Each run writes out 100 binary files.

#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"
#include "STMDAQ-TestBeam/utils/DateTime.hh"
#include "STMDAQ-TestBeam/utils/dbase.hh"
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/generator_iterator.hpp>
#include <chrono>


// Define variables that are global and will need to be a top of FE code or passed around ...

// Setup a quick float (0.0-1.0) random number generator
boost::variate_generator<boost::hellekalek1995, boost::uniform_real<float> >
     frand(boost::hellekalek1995(), boost::uniform_real<float>(0.0f, 1.0f) );

// Define the BinaryFile instance
BinaryFile *bf;
// Define the XML reader instance 
Xml *xml_file;

int simple_test_pattern(int i, int itrig){
        int test_data;
        if (itrig % 4 == 0)
            test_data = i;
        else if (itrig % 3 == 0)
            test_data = -1*i;
        else if (itrig % 2 == 0)
            test_data = 10*i;
        else
            test_data = -10*i;
        return test_data;
}

void triangle(int16_t* data, unsigned int p1, unsigned int N){
    if ( (p1 + 100) > N  ) p1 = p1 - 120;
    if ( (p1 + 50) > N  ) p1 = p1 - 170;
    int ix = fmin(15,int(30*frand()));
    for (unsigned int i = p1; i < p1+50; i++){
        data[i] = data[i-1] - ix;
    }
    for (unsigned int i = p1+50; i < p1+100; i++){
        data[i] = data[i-1] + ix;
    }
}

// generate 3 triangle pulses across the timeline
void random_signal_test_pattern(int16_t* data, int trigger_rate, unsigned int N){
  
    std::chrono::steady_clock::time_point t1  = std::chrono::steady_clock::now();
    for (unsigned int i = 0; i < N; i++){
        data[i] = int(10*frand());
    }
    float trigger_length = N*2.7/1e6;
    float pulse_gap = 1.0/float(trigger_rate);
    int npulses = int(trigger_length/pulse_gap);
    for (int i = 0; i < npulses; i++){
        unsigned int p1 = int(frand()*N); 
        triangle(data,p1,N); 
    }
    std::chrono::steady_clock::time_point t2  = std::chrono::steady_clock::now();
    auto diff = t2 - t1; 
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);
    std::cout << "Time to generate data for this trigger = " << nsec.count() << " ns " << std::endl;
  
}

void frontend_start() {
  // Open the XML file and instantiate the xml_file object
  std::string xml_path = EnvVars::expand("${STM_XML}");
  std::cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);
  // Create the instance to write the binary files
  bf = new BinaryFile();

}

void frontend_stop() {
    delete xml_file;
    delete bf;    
}

void begin_run(int irun){

// Get the current run number from the MIDAS ODB 

//  status = cm_get_experiment_database(&hDB, NULL);
    int midas_run_number = 0; 
//    INT size=sizeof(midas_run_number);
//   db_get_value(hDB,0,("Equipment/blah/blah/..").c_str(),&midas_run_number,&size,TID_WORD,FALSE);

// For now just get run number from passed argument

    midas_run_number = irun; // int(100*frand());
    std::cout << "begin_run() : run number = " << midas_run_number << std::endl;

// Get max subrun size from XML
    int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100); // in Mb 
    std::cout << "Max max_binary_file_size (int) = " << max_binary_file_size << std::endl;
    int Mbytes = 1048576;
    unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
    std::string binary_file = xml_file->value("stm.raw_binary_filename");
    std::cout << "Max Binary File Size  = " << max_binary_size << " bytes " << std::endl;
    std::cout << "Binary File           = " << binary_file << std::endl;

    if (max_binary_size >= INT32_MAX) {
      std::cout << "Max binary file size has to be less than 2 Gb, else int32 issues, EXITING" << std::endl;
      exit(-1);
    }

    bf->set_subrun_filesize(max_binary_size);
    bf->open_raw_output_file(binary_file, midas_run_number, 0);

}

void end_run(int irun){
    bf->close_output_file();
}


void generate_header(int16_t* header_data, uint16_t trigger_number, uint16_t trigger_type, 
                     uint64_t trigger_time, uint32_t data_size, uint16_t header_size){

    uint32_t header_identifier  = 0xBEEFDEAD;
    // pack_uint32_in_int16_array(header_data, (uint32_t) 0, x);
    std::memcpy( &header_data[0], &header_identifier, sizeof( header_identifier ) );
    std::memcpy( &header_data[2], &trigger_number, sizeof( trigger_number ) );
    std::memcpy( &header_data[3], &trigger_type, sizeof( trigger_type ) );
    std::memcpy( &header_data[4], &trigger_time, sizeof( trigger_time ) );
    uint32_t payload_size = uint32_t (data_size) + (uint32_t) (2*header_size); // in bytes not int16
    std::memcpy( &header_data[8], &payload_size, sizeof( payload_size ) );

//    pack_uint32_in_int16_array(header_data, (uint32_t) 2, size_to_next_header);
}


void event_trigger(int itrig){

    // This should be event data ie 3(?) 64-bit trigger header words plus the slice header/slice data.
    // Here to test just generating small volume of data both +ve/-ve values to test the BinaryFile code
    // Note data is written as SIGNED int16 so in range: -32,768 to 32,767.

    int trigger_length = 10; // in ms
    unsigned int nsamples_per_trigger = int(1e6*float(trigger_length)/2.7);
    int trigger_rate = 1; // in kHz
    const unsigned int N = nsamples_per_trigger;

    if (itrig == 0){
      std::cout << "Samples per trigger = " << nsamples_per_trigger << "; " << float(nsamples_per_trigger*2)/1048576.0 << " Mb " <<  std::endl;
      std::cout << "Trigger rate = " << trigger_rate << " kHz"  << std::endl;
    }

    uint16_t header_size = 10;
    int16_t* test_data = new int16_t[N];
//    random_signal_test_pattern(test_data, trigger_rate, N); 

//    for (int i = 0; i < N; i++){
//        test_data[i] = simple_test_pattern(i, itrig);
//    }

//    int16_t* header_data = new int16_t[M];
//    generate_header(itrig, header_data, M, N);

    int16_t* header_data = new int16_t[header_size];
    generate_header(header_data, 
            (uint16_t) 0x8010,             // trigger number (16) [2] ([0,1] = DEADBEEF)
            (uint16_t) 0x1,                // trigger type (16) [3]
            (uint64_t) 0x87654321FEDCBA98, // trigger time (64) [4,5,6,7]
            (uint32_t) 0xABCD1234,         // data size (32) [8,9]
            (uint16_t) header_size);       // header size (16)


    bf->write_raw_data(test_data,N);
}

int main(){                                                                                           

    frontend_start();
                                                                   
//  Simulate two MIDAS runs that each have a 10 triggers/events
    begin_run(1);
    for (int i = 0; i < 100; i++){
        event_trigger(i);
    }
    end_run(1);

    begin_run(2);
    for (int i = 0; i < 100; i++){
        event_trigger(i);
    }
    end_run(2);

    frontend_stop();

    return 0;                                                                               
}
