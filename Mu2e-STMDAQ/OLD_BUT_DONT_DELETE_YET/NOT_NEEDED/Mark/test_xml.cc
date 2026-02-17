#include "STMDAQ-TestBeam/utils/xml.hh"
#include "STMDAQ-TestBeam/utils/EnvVars.hh"
#include <iostream>     // std::ios, std::istream, std::cout


Xml *xml_file;

int main(){   

  std::string xml_path = EnvVars::expand("${STM_XML}");
  std::cout << "XML_PATH = " << xml_path << std::endl;
  xml_file = new Xml(xml_path);
  int max_binary_file_size = xml_file->int_value("stm.max_binary_file_size",100); // in Mb 

  int Mbytes = 1048576;
  unsigned int max_binary_size = (unsigned int) max_binary_file_size * (unsigned int) Mbytes;
  std::string binary_file_directory = xml_file->value("stm.binary_file_directory");
  std::cout << "Max Binary File Size  = " << max_binary_size << " bytes " << std::endl;
  std::cout << "Binary File Directory = " << binary_file_directory << std::endl;

  unsigned int fmc144_ext_slice_length = xml_file->int_from_hex_value("stm.fmc144_ext_slice_length");
  std::cout << "fmc144_ext_slice_length [hex] = " << std::hex << fmc144_ext_slice_length << " [int] = " << std::dec << fmc144_ext_slice_length << std::endl;

}
