#include <algorithm>

// Binary file class
#include "STMDAQ-TestBeam/utils/BinaryFile.hh"

// Data variables
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// *************************************************
// Initialisations
// *************************************************

// Initialisation constructor - setup DB
BinaryFile::BinaryFile(){
  _subrun = 0;
  _fsize = 0;
  _filename = "NOT_DEFINED";
#if defined(USE_SQL)
  _db = new DBase("127.0.0.1",5432,DBase::WRITE);
#endif
}

// Delete instance of DB
BinaryFile::~BinaryFile(){
#if defined(USE_SQL)
  delete _db;
#endif
}

// *************************************************
// Return values
// *************************************************

// Return run number
int BinaryFile::run() const{
  return _run;
}

// Return subrun number
int BinaryFile::subrun() const{
  return _subrun;
}

// Return filesize
unsigned int BinaryFile::fsize() const{
  return _fsize;
}

// Return filename
std::string BinaryFile::filename() const{
  return _filename;
}

// *************************************************
// Set variables
// *************************************************

// Set the run number
void BinaryFile::set_run(int run){
  _run = run;
}

// Set the subrun number
void BinaryFile::set_subrun(int subrun){
  _subrun = subrun;
}

// Set the max subrun filesize
void BinaryFile::set_subrun_filesize(int64_t subrun_size){
  _subrun_size_limit = subrun_size;
}

// Set the raw file version
void BinaryFile::set_raw_file_version(int iversion){ // 0 for old format without unix timestamp, 1 with unix time stamp
  _raw_file_version = iversion;
}

// // Set the software trigger header size
// int BinaryFile::Header_Size(){

//   int HEADER_SIZE = sw_tHdr_Size; // new file 
//   if (_raw_file_version == 0) { // old file
//     HEADER_SIZE = HEADER_SIZE - 8;  // in bytes !!
//   }
//   return HEADER_SIZE;
// }


// *************************************************
// Open file functions
// *************************************************

// Open a input binary file to read
void BinaryFile::open_input_file(std::string filename){
  _t1 = std::chrono::steady_clock::now();
  _filename = filename;
  _fsize = 0; // get file size in bytes
  struct stat buf;
  if (stat(filename.c_str(), &buf) == 0){
    _ipfile.open(_filename, std::ios::in | std::ios::binary);
    _ipfile.seekg(0, std::ios::end);
    _fsize = _ipfile.tellg();
    _ipfile.seekg(0, std::ios::beg);
  }
  else {
    std::cout << "File = " << filename << " does not exist ... " << std::endl;
    exit(-1);
  }
}

// Open an output binary file to write
void BinaryFile::open_output_file(std::string path, int run, int subrun){
  std::stringstream ss; ss << path << "_run" << std::setfill('0') << std::setw(7) << run << "_subrun" << std::setfill('0') << std::setw(5) << subrun << ".bin";
  _filename = ss.str();
  _path = path;
  _run = run;
  _subrun = subrun;
  _fsize = 0;
  std::cout << "open_output_file : " << _filename << std::endl;
  _opfile.open(_filename, std::ios::out | std::ios::binary);
}

// Open a raw data binary file to write and store info in DB
void BinaryFile::open_raw_output_file(std::string path, int run, int subrun){
  open_output_file(path, run, subrun);
#if defined(USE_SQL)
  DateTime time;
  int ctt  = time.CurrentTime();
  _db->write_run_subrun_time(_run,_subrun,ctt,0);
#endif 
}

// *************************************************
// Read file functions
// *************************************************

// Get the run number from the filename
int BinaryFile::getRunfromFilename(){
  int ipos = _filename.find("run");
  std::string run = _filename.substr(ipos+3, 7);
  std::cout << "getRunfromFilename(): " << run << std::endl;
  return std::atoi(run.c_str());
}

// Get the subrun number from the filename
int BinaryFile::getSubRunfromFilename(){
  int ipos = _filename.find("subrun");
  std::string subrun = _filename.substr(ipos+6, 5);
  std::cout << "getSubRunfromFilename(): " << subrun << std::endl;
  return std::atoi(subrun.c_str());
}

// Read integer
int BinaryFile::read_int(){
  int value;
  _ipfile.read( reinterpret_cast<char *> (&value), sizeof(int));
  return value;
}

// Read binary file data with given offset
void BinaryFile::read_data(int16_t* data, unsigned int N, unsigned int offset){

  // Check that amount being read when take into offset doesn't go beyond end of file...
  // N.B we have unsigned int here so behaviour will get odd if N = 2e9 and offset = 2e9
  if (N >= INT32_MAX/2) {
    std::cout << " You are trying to use an array > 2 Gb -- problems ... EXITING ... " 
	      << std::endl;
    exit(-1);
  }
  if (offset + 2*N <= _fsize) {
    _ipfile.seekg(offset, std::ios::beg);
    _ipfile.read( (char*) data, N*sizeof(data[0]));
  }
  else {
    std::cout << "Requested to read subset of file that goes beyond EOF, offset = " 
	      << offset << " bytes and asked to read " 
	      << 2*N << " bytes and filesize = " 
	      << _fsize <<  " bytes" << std::endl;
    exit(-1);
  }
}

// Read binary file data - no offset
void BinaryFile::read_data(int16_t* data, unsigned int N){
  if (N >= INT32_MAX/2) {
    std::cout << " You are trying to use an array > 2 Gb -- problems ... EXITING ... " << std::endl;
    exit(-1);
  }
  _ipfile.seekg(0, std::ios::beg);
  _ipfile.read( (char*) data, N*sizeof(data[0]));
}


// // Get the locations of each trigger in the raw data
// std::vector<uint32_t> BinaryFile::get_trigger_locations(bool external_triggers, bool internal_triggers){
//   int HEADER_SIZE = Header_Size();
//   int NINT16 = HEADER_SIZE/2;

//   std::vector<uint32_t> trigger_locations;
//   uint32_t offset = 0;
//   // std::cout << "File size in bytes = " << _fsize << std::endl;
//   uint32_t* d = new uint32_t[1];
//   while (offset < _fsize) {
//     int16_t* header = getTriggerHeader(offset);

//     printi16a(header, HEADER_SIZE/2);

//     if (! checkDeadBeef(header)){
//       std::cout << "ERROR: Expected DEADBEEF ... EXIT" << std::endl;
//       exit(-1);
//     }

//     uint16_t triggerType = getTriggerType(header);

//     if ( (triggerType == 1) && internal_triggers){
//       trigger_locations.push_back(offset);
//     }
//     if ( (triggerType == 0) && external_triggers){
//       trigger_locations.push_back(offset);
//     }

//     uint64_t trigger_time = getTriggerTime(header);
//     uint32_t adc0_time = getADC0Time(header);

//     uint32_t data_size = getDataSize(header);
//     offset = offset + HEADER_SIZE + data_size;
//   }
//   return trigger_locations;

// }

// // Get and return the trigger header as an int16_t array
// int16_t* BinaryFile::getTriggerHeader(int offset){

//   int HEADER_SIZE = Header_Size();
//   int NINT16 = HEADER_SIZE/2;
//   //std::cout << "Software trigger header length = " << HEADER_SIZE << " bytes; " << " INT16 size = " << NINT16 << std::endl;
//   int16_t* header = new int16_t[NINT16];

//   // Check for read error or end of file ...
//   _ipfile.seekg(offset, std::ios::beg);
//   _ipfile.read( (char*) header, HEADER_SIZE);

//   return header;
// }

// // Check that the trigger header starts with 0xDEADBEEF
// bool BinaryFile::checkDeadBeef(int16_t* header){
//   uint16_t beef = (uint16_t) header[sw_tHdr_0]; 
//   uint16_t dead = (uint16_t) header[sw_tHdr_1]; 
//   if ( (dead != (uint16_t) 0xdead) || (beef != (uint16_t) 0xbeef) ) {
//     std::cout << "dead = " << std::hex << dead << " beef = " << std::hex << beef << std::endl;
//     return false;
//   }
//   else {
//     return true;
//   }
// }

// // Get and return the trigger number from the trigger header
// uint32_t BinaryFile::getTriggerNumber(int16_t* header){
//   return _getInt32_cane(header, sw_tHdr_TrigNum1);
// }

// // Get and return the trigger type from the trigger header
// uint16_t BinaryFile::getTriggerType(int16_t* header) {
//   uint16_t modeChanType = (uint16_t) header[sw_tHdr_MdChTp];
//   uint16_t triggerType = (modeChanType >> 12) & 0xF; // Get trigger type
//   // std::cout << "Trigger Type = " << triggerType << std::endl;
//   if (triggerType != 0 && triggerType != 1)
//     std::cout << "getTriggerType : trigger type not 0 or 1 but " << triggerType << std::endl;
//   return triggerType;
// }

// // Get and return the trigger time from the trigger header
// uint64_t BinaryFile::getTriggerTime(int16_t* header){
//   uint64_t trigger_time;
//   trigger_time = _getInt64_dog(header, sw_tHdr_TrigTime1);

//   //std::cout << "trigger time (dog) = " << trigger_time << std::endl;
//   //trigger_time = _getInt64_hund(header, sw_tHdr_TrigTime1);
//   //std::cout << "trigger time (hund) = " << trigger_time << std::endl;
  
//   return trigger_time;
// }

// // Get and return the first ADC time from the trigger header
// uint32_t BinaryFile::getADC0Time(int16_t* header){
//   return _getInt32_cane(header, sw_tHdr_ADCoffset1);
// }

// // Get and return the data size from the trigger header
// uint32_t BinaryFile::getDataSize(int16_t* header){
//   //uint32_t value1 =  _getInt32_perro(header, sw_tHdr_dataSize1);
//   uint32_t value2 = _getInt32_cane(header, sw_tHdr_dataSize1);
//   return value2;
// }

// // Get the raw data from the binary file
// data* BinaryFile::getData(uint32_t offset){

//   int16_t* header = getTriggerHeader(offset);
//   //std::cout << "getData() : offset = " << offset << std::endl;
//   //printi16a(header, sw_tHdr_Size/2); // correct to here

//   uint32_t data_size = getDataSize(header);
//   //std::cout << "getData() : data_size = " << data_size << std::endl; // correct

//   data* d = new data();
//   d->t0 = 0xdeadbeef;
//   d->nadc = (data_size - sw_sHdr_Size)/2 ;
//   int HEADER_SIZE = Header_Size();
//   uint32_t data_offset = offset + HEADER_SIZE + sw_sHdr_Size; 
//   d->adc = new int16_t[d->nadc];
//   _ipfile.seekg(data_offset, std::ios::beg);
//   _ipfile.read( (char*) d->adc, 2*d->nadc);

//   uint32_t N_TO_PRINT; 
//   if (d->nadc > 100){
//     N_TO_PRINT = 100;
//   }
//   else {
//     N_TO_PRINT = d->nadc;
//   }
//   //printi16a(d->adc, N_TO_PRINT);
  
//   return d;
// }


// Read and return peaks from a trigger's worth of MWD data
peaks* BinaryFile::read_MWD_data() {
  peaks* peak_data = new peaks();
  uint32_t HEADER;
  uint32_t trigger_num;
  uint32_t nadc_values;
  uint16_t trigger_type;
  int npeaks;

  _ipfile.read( reinterpret_cast<char *>(&HEADER), sizeof (uint32_t)); // SHOULD BE 0xCAFE AC1D
  _ipfile.read( reinterpret_cast<char *>(&trigger_num), sizeof (uint32_t));
  _ipfile.read( reinterpret_cast<char *>(&trigger_type), sizeof (uint16_t));
  _ipfile.read( reinterpret_cast<char *>(&npeaks), sizeof (int));
  _ipfile.read( reinterpret_cast<char *>(&nadc_values), sizeof (uint32_t));

  std::cout << std::hex << HEADER << std::dec << " " << trigger_num << " " << trigger_type << " " << npeaks << " # adc values " << nadc_values << std::endl;

  peak_data->trigger_number = trigger_num;
  peak_data->trigger_type = trigger_type;
  peak_data->npeaks = npeaks;
  peak_data->nadc_values = nadc_values;
  for (int i = 0; i < npeaks; i++){
    double peak_height;
    _ipfile.read( reinterpret_cast<char *>(&peak_height), sizeof (double));
    peak_data->peak_heights.push_back(peak_height);
  }
  for (int i = 0; i < npeaks; i++){
    double peak_time;
    _ipfile.read( reinterpret_cast<char *>(&peak_time), sizeof (double));
    peak_data->peak_times.push_back(peak_time);
  }
  return peak_data;
}

// Print a vector of int16_t values 
void BinaryFile::printi16a(int16_t* d, int N){
  
  std::cout << "\nint16[] array ... " << std::endl;
  for (int i = 0; i < N; i++){
    std::printf("\n[%i] int: %i hex: x[%02x]",i,d[i],d[i]);
  }
  std::printf("\n\n");
}

// *************************************************
// Write file functions
// *************************************************

// Write integer value
void BinaryFile::write_int(int value){
  _opfile.write( (char *)&value, sizeof(value)); 
}

// Write vector of values
void BinaryFile::write_vector(std::vector<double> values){
  for (double element : values){
    _opfile.write( (char *)&element, sizeof(element)); 
  }
}

// Write raw data binary files
void BinaryFile::write_raw_data(int16_t* data, unsigned int size){  // might need some const bollocks here
  if (size >= INT32_MAX/2) {
    std::cout << " You are trying to write an array > 2 Gb -- problems ... EXITING ... " << std::endl;
    exit(-1);
  }
  int bytes_to_write = size*sizeof(int16_t);
  _opfile.write((char *) data, bytes_to_write);
  if (! _opfile) std::cerr << "write failed\n";
  // std::cout << "_opfile.write: " << bytes_to_write << std::endl;
  _fsize = _fsize + bytes_to_write;
  // std::cout << "writing " << bytes_to_write << " bytes of data to file" << std::endl;
}

// Tell the class what the size of the incoming raw data in number of 
// int16 (2 bytes) ie sum of trigger header + slice headers + slice data ie 
// trigger header plus the data size stored in the trigger header divided by TWO. 
// So that we can open a new file if this exceeds the file size limit.
void BinaryFile::incoming_raw_data_size(uint32_t size) {
  if (size >= INT32_MAX/2) {
    std::cout << " You are trying to write an array > 2 Gb -- problems ... EXITING ... " << std::endl;
    exit(-1);
  }
  int bytes_to_write = size*sizeof(int16_t);
  // std::cout << _fsize << " " << bytes_to_write << " " <<  _subrun_size_limit << std::endl;
  if (_fsize + bytes_to_write > _subrun_size_limit){
    close_raw_output_file();
    _subrun = _subrun + 1;
    open_raw_output_file(_path,_run,_subrun);
  }
}

// Write MWD data peaks
void BinaryFile::write_MWD_data(peaks* peak_data){

  if (peak_data->trigger_type != 0 && peak_data->trigger_type != 1){
    std::cout << "write_MWD_data: trigger_type is not 0 or 1, it is :: " << peak_data->trigger_type << std::endl;
  }
  else {
    uint32_t HEADER = 0xCAFEAC1D;
    _opfile.write((char *) &HEADER, sizeof (HEADER));
    _opfile.write((char *) &peak_data->trigger_number, sizeof (uint32_t));
    _opfile.write((char *) &peak_data->trigger_type, sizeof (uint16_t));
    _opfile.write((char *) &peak_data->npeaks, sizeof (int));
    _opfile.write((char *) &peak_data->nadc_values, sizeof (uint32_t));
    write_vector(peak_data->peak_heights);
    write_vector(peak_data->peak_times);
  }
}
       
// *************************************************
// Close file functions
// *************************************************

// Close an input binary file        
void BinaryFile::close_input_file(){
  _ipfile.close();
  // std::cout << "closing current input binary file" << std::endl;
}

// Close an output binary file
void BinaryFile::close_output_file(){
  std::cout << " closing file ... " << std::endl;
  _opfile.close();
}
        
// Close an output raw binary file
void BinaryFile::close_raw_output_file(){
  std::cout << " closing file ... " << std::endl;
  _opfile.close();
  _t2 = std::chrono::steady_clock::now();
#if defined(USE_SQL)
  DateTime time;
  int ctt  = time.CurrentTime();
  _db->write_run_subrun_time(_run,_subrun,ctt,1);
#endif 
  auto diff = _t2 - _t1; 
  auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);
  //	    std::cout << "Time between open and closing file = " << nsec.count()  << " ns" <<std::endl;
}


// *************************************************
// PRIVATE FUNCTIONS
// *************************************************

// Get and return os and print run/file details
std::ostream & operator<<(std::ostream &os, const BinaryFile& bf) {
  os << "Run = " << bf.run() << "| Subrun = " << bf.subrun() 
     << "| Filename = " << bf.filename() << "| Size = " << bf.fsize();
  return os;
}

// Get word DOG from header
uint64_t BinaryFile::_getInt64_dog(int16_t* header, int istart){

  uint64_t w4 = (uint64_t) header[istart+3] << 48 & (uint64_t) 0xFFFF000000000000;
  uint64_t w3 = (uint64_t) header[istart+2] << 32 & (uint64_t) 0x0000FFFF00000000;
  uint64_t w2 = (uint64_t) header[istart+1] << 16 & (uint64_t) 0x00000000FFFF0000;
  uint64_t w1 = (uint64_t) header[istart] << 0    & (uint64_t) 0x000000000000FFFF;
  uint64_t value = (uint64_t)(w1 | w2 | w3 | w4); 

  //  std::cout << "_getInt64_dog " << std::hex << w1 << " " << std::hex << w2 << " " << std::hex << w3 << " " << std::hex << w4 << " " << std::hex << value << std::endl;

  return value;
}

// Get word CANE from header
uint32_t BinaryFile::_getInt32_cane(int16_t* header, int istart){
  
  uint32_t w2 = (uint32_t) header[istart+1] << 16 & (uint32_t) 0xFFFF0000;
  uint32_t w1 = (uint32_t) header[istart] << 0 & (uint32_t) 0x0000FFFF;
  uint32_t value = (uint32_t)(w1 | w2); 

  //  std::cout << "_getInt32_cane " << std::hex << w1 << " " << std::hex << w2 << " " << std::hex << value << std::endl;
  return value;
}

// Get word HUND from header
uint64_t BinaryFile::_getInt64_hund(int16_t* header, int istart){

  int16_t* values = new int16_t[4];
  for (int i = 0; i < 4; i++) {
    values[i] = header[istart + i];
  }
  uint64_t* valueX = reinterpret_cast<uint64_t*>(values);
  uint64_t value = *valueX;
  delete values;
  std::cout << "_getInt64_hund " << std::hex << value << std::endl;
  return value;
}

// Get word PERRO from header
uint32_t BinaryFile::_getInt32_perro(int16_t* header, int istart){

  //will run if not on stack
  //int16_t v[2];// = {1,2}; 

  //initialise a new array with zeroes
  int16_t* values = new int16_t[2]();

  for (int i = 0; i < 2; i++) {
    values[i] = header[istart + i];
  }
  uint32_t* valueX = reinterpret_cast<uint32_t*>(values);
  uint32_t value = *valueX;
  delete values;
  //  std::cout << "_getInt32_perro " << std::hex << value << std::endl;
  return value;
}




