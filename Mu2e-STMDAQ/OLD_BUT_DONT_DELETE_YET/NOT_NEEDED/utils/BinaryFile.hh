#ifndef BINARYFILE_HH
#define BINARYFILE_HH

#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>   
#include <iomanip>   
#include <string> 
#include <stdio.h>
#include <fstream>                                                                     
#include <sstream>                                   
#include <chrono>                                  
#if defined(USE_SQL) 
#include "STMDAQ-TestBeam/utils/dbase.hh"
#endif
#include "STMDAQ-TestBeam/utils/DateTime.hh"
#include "STMDAQ-TestBeam/MWD/data.hh"
#include "STMDAQ-TestBeam/MWD/peaks.hh"

class BinaryFile {

public:  

  // *************************************************
  // Initialiations
  // *************************************************

  // Initialisation constructor - setup DB
  BinaryFile();
  // Delete instance of DB
  ~BinaryFile();

  // *************************************************
  // Return values
  // *************************************************

  // Return run number
  int run() const;
  // Return subrun number
  int subrun() const;
  // Return filesize
  unsigned int fsize() const;
  // Return filename 
  std::string filename() const;

  // *************************************************
  // Set variables
  // *************************************************

  // Set the run number
  void set_run(int run);
  // Set the subrun number
  void set_subrun(int subrun);
  // Set the max subrun filesize
  void set_subrun_filesize(int64_t subrun_size);
  // Set the raw file version
  void set_raw_file_version(int iversion); // 0 for old format without unix timestamp, 1 with unix time stamp

  // Set the software trigger header size
  //  int Header_Size();

  // *************************************************
  // Open file functions
  // *************************************************

  // Open an input binary file to read
  void open_input_file(std::string filename);
  // Open an output binary file to write
  void open_output_file(std::string path, int run, int subrun);
  // Open a raw data binary file to write and store info in DB
  void open_raw_output_file(std::string path, int run, int subrun);

  // *************************************************
  // Read file functions
  // *************************************************

  // Get the run number from the filename
  int getRunfromFilename();
  // Get the subrun number from the filename
  int getSubRunfromFilename();

  // Read integer value
  int read_int();

  // Read binary file data with given offset
  void read_data(int16_t* data, unsigned int N, unsigned int offset);
  // Read binary file data - no offset
  void read_data(int16_t* data, unsigned int N);

  // Get the locations of each trigger in the raw data 
  // return array of file positions of triggers 
  //  std::vector<uint32_t> get_trigger_locations(bool external_triggers, bool internal_triggers); 
  // Get and return the trigger header as an int16_t array
  //  int16_t* getTriggerHeader(int offset);
  // Check that the trigger header starts with 0xDEADBEEF
  //  bool checkDeadBeef(int16_t* header);
  // Get and return the trigger number from the trigger header
  //  uint32_t getTriggerNumber(int16_t* header);
  // Get and return the trigger type from the trigger header
  //  uint16_t getTriggerType(int16_t* header);
  // Get and return the trigger time from the trigger header
  //  uint64_t getTriggerTime(int16_t* header);
  // Get and return the first ADC time from the trigger header
  //  uint32_t getADC0Time(int16_t* header);
  // Get and return the data size from the trigger header
  //  uint32_t getDataSize(int16_t* header);

  // Get the raw data from the binary file
  data* getData(uint32_t byte_offset_in_file);

  // Read and return peaks from a trigger's worth MWD data
  peaks* read_MWD_data(); // read a triggers worth of MWD data

  // Print a vector of int16_t values
  void printi16a(int16_t*, int);

  // *************************************************
  // Write file functions
  // *************************************************

  // Write integer value
  void write_int(int value);
  // Write vector of values
  void write_vector(std::vector<double> values);

  // Write raw data binary files
  void write_raw_data(int16_t* data, unsigned int size);  // might need some const bollocks here
  // Tell the class what the size of the incoming raw data in number of
  // int16 (2 bytes) ie sum of trigger header + slice headers + slice data ie                         
  // trigger header plus the data size stored in the trigger header divided by TWO.                   
  // So that we can open a new file if this exceeds the file size limit.
  void incoming_raw_data_size(uint32_t size); 

  // Write MWD data peaks
  void write_MWD_data(peaks* peak_data);

  // *************************************************
  // Close file functions
  // *************************************************

  // Close an input binary file
  void close_input_file();
  // Close an output binary file
  void close_output_file();
  // Close an output raw binary file
  void close_raw_output_file();

private:

#if defined(USE_SQL) 
  DBase* _db; // Database instance
#endif
  int _raw_file_version; // Raw file version
  int _run; // Run number
  int _subrun; // Subrun number
  unsigned int _fsize; // File size
  unsigned int _subrun_size_limit; // Subrun file size limit - max 2 Gbytes
  std::string _filename; // Filename
  std::string _path; // File path
  std::ofstream _opfile; // Output file
  std::ifstream _ipfile; // Input
  std::chrono::steady_clock::time_point _t1; // Time 1
  std::chrono::steady_clock::time_point _t2 ; // Time 2

  // Get and return os and print run/file details
  friend std::ostream & operator<<(std::ostream &os, const BinaryFile& bf);
  
  // Get word DOG from header
  uint64_t _getInt64_dog(int16_t* header, int istart);
  // Get word CANE from header
  uint32_t _getInt32_cane(int16_t* header, int istart); 
  // Get word HUND from header
  uint64_t _getInt64_hund(int16_t* header, int istart);
  // Get word PERRO from header
  uint32_t _getInt32_perro(int16_t* header, int istart);

};

#endif
