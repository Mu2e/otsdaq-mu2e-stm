#include <stdlib.h> 

#include "STMDAQ-TestBeam/utils/Logger.hh"
#include "STMDAQ-TestBeam/utils/Random.hh"
#include "STMDAQ-TestBeam/utils/DateTime.hh"
#include "STMDAQ-TestBeam/utils/ziggurat.hh"

Random* Random::_instance = 0;
Destroyer<Random> Random::_destroyer;
unsigned long Random::_random_seed = 1;
std::default_random_engine Random::_engine ;
unsigned Random::_random_seed_zig = 1;

// Ziggurat algorithm variables
float Random::_fn[128];
uint32_t Random::_kn[128];
float Random::_wn[129];

void Random::Init(unsigned long random_seed){

  _random_seed = random_seed;
  _random_seed_zig = random_seed;
  _engine.seed(_random_seed);

  // Initialise ziggurat algorithm
  Init_Zig();

  stringstream ss; ss << "[ Random :: Init ] with user specified seed = " << _random_seed;
  Logger::Instance()->write(Logger::INFO,ss.str());
  _init();

}

void Random::Init(){

  _random_seed = std::chrono::system_clock::now().time_since_epoch().count();
  _random_seed_zig = std::chrono::system_clock::now().time_since_epoch().count();
  _engine.seed(_random_seed);

  // Initialise ziggurat algorithm
  Init_Zig();

  stringstream ss; ss << "[ Random :: Init ] with random/chrono seed = " << _random_seed;
  Logger::Instance()->write(Logger::INFO,ss.str());
  _init();

}

void Random::Init_Zig(){

  // Initialise ziggurat algoritm normal distribution
  r4_nor_setup ( _kn, _fn, _wn );

  stringstream ss; ss << "Initialised ziggurat algorithm";;
  Logger::Instance()->write(Logger::INFO,ss.str());


}

double Random::GaussValue(double mean, double sigma, bool ziggurat){

  double value = 0;
  if (ziggurat) {
    value = mean + (r4_nor ( _random_seed_zig, _kn, _fn, _wn ) * sigma);
  }
  else{
    std::normal_distribution<double> g_dist(mean, sigma);
    value = g_dist(_engine);
  }

  return value;

}

int Random::PoissValue(double rate){

 std::poisson_distribution<int> p_dist (1/rate);
 return p_dist(_engine);

}

int Random::IntegerValue(int min_value, int max_value, bool ziggurat){
  
  int value = 0;
  if (ziggurat) {
    value = min_value + int(r4_uni ( _random_seed_zig ) * ((max_value+1)-min_value));
  }
  else{
    std::uniform_int_distribution<int> i_dist(min_value, max_value);
    value = i_dist(_engine);
  }
  
  return value;


}

double Random::RealValue(double min_value, double max_value, bool ziggurat){

  double value = 0;
  if (ziggurat) {
    value = min_value + (r4_uni ( _random_seed_zig ) * (max_value-min_value));
  }
  else{
    std::uniform_real_distribution<> r_dist(min_value, max_value);
    value= r_dist(_engine);
  }

  return value;

}

int Random::DiscreteValue(int* distribution, int n){

  std::vector<int> distribution_vect;
  for(int i = 0; i < n; i++){distribution_vect.push_back(distribution[i]);}
  std::discrete_distribution<int> d_dist(distribution_vect.begin(), distribution_vect.end());
    
  return d_dist(_engine);

}


Random* Random::Instance () {
  if (! _instance) {
    Logger::Instance()->write(Logger::ERROR,"[ Random :: Instance ] You need to initialise Random() interface once using Random::Init() or Random::Init(seed)");
    exit(-1);
  }
  return _instance;
}
 
void Random::_init() { 

  if (!_instance) {
    _instance = new Random;
    _destroyer.SetDoomed(_instance);
  }
}

