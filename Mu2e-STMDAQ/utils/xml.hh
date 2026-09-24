#ifndef XML_HH
#define XML_HH

// XML - example code
//    xml_file = new Xml("test.xml");
//    int default_baseline_value = xml_file->int_value("stm.default_baseline_value",100);   
//    std::string binary_file_directory = xml_file->value("stm.binary_file_directory");
//    float test_value = xml_file->float_value("stm.test_float_value",100.0);


#include <boost/property_tree/ptree.hpp>                                                    
#include <boost/property_tree/xml_parser.hpp>                                               
#include <boost/property_tree/detail/file_parser_error.hpp>                                 
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/foreach.hpp>                                 
#include <iostream>   
#include <string>                                                                      
#include <regex>

namespace pt = boost :: property_tree; 

class Xml {

    public:  
        pt::ptree stm_tree;

        Xml(std::string filename){
            try {                                                                                   
               pt::read_xml(filename, stm_tree);                                                        
            } catch (pt :: xml_parser_error &e) {                                                     
	      std :: cout << "ERROR: Failed to parse the xml file \n " << e.what() << "\n";                       
            } catch (...) {                                                                         
                std :: cout << "Failed !!!";                                                        
            }   
        }
        int int_value(std::string name, int def_value) {
            return stm_tree.get(name, def_value);
        }
        long long_value(std::string name, long def_value) {
            return stm_tree.get(name, def_value);
        }
        float float_value(std::string name, float def_value){
            return stm_tree.get(name, def_value);
        }
        double double_value(std::string name, double def_value){
            return stm_tree.get(name, def_value);
        }
        std::string value(std::string name){
	  std::string value = stm_tree.get<std::string>(name);
	  value = std::regex_replace(value, std::regex("^ +| +$|( ) +"), "$1");
	  return value;
        }
        unsigned int int_from_hex_value(std::string name){
	  unsigned int x;   
	  std::stringstream ss;
	  std::string val = value(name);
	  ss << std::hex << val;
	  ss >> x;
	  return static_cast<uint>(x);
	}
        

};
#endif
