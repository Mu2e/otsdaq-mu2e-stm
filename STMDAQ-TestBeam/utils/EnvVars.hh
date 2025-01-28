#ifndef EnvVars_hh
#define EnvVars_hh

#include <string>

namespace EnvVars {

  //Expand all environment variables in a string
  std::string expand(const std::string & s);

}//namespace

#endif
