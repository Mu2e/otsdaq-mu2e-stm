#include "EnvVars.hh"

std::string EnvVars::expand( const std::string & s ) {

  if( s.find( "${" ) == std::string::npos ) return s;

  std::string pre  = s.substr( 0, s.find( "${" ) );
  std::string post = s.substr( s.find( "${" ) + 2 );

  if( post.find( '}' ) == std::string::npos ) return s;

  std::string variable = post.substr( 0, post.find( '}' ) );
  std::string value    = "";

  post = post.substr( post.find( '}' ) + 1 );

  if( getenv( variable.c_str() ) != NULL ) value = std::string( getenv( variable.c_str() ) );

  return expand( pre + value + post ); //recursive in case more than one env var

}

