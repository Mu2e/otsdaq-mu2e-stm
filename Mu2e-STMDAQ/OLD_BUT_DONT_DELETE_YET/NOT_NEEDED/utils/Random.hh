#ifndef RANDOM_HH
#define RANDOM_HH

#include <chrono>
#include <random>
#include "STMDAQ-TestBeam/utils/Destroyer.hh"

using namespace std;

class Random {
public:
  static Random*         Instance();
  static void            Init();
  static void            Init(unsigned long random_seed);
  static void            Init_Zig();

  static int             IntegerValue(int min_value, int max_value, bool ziggurat);
  static double          GaussValue(double mean, double sigma, bool ziggurat);
  static int             PoissValue(double rate);
  static double          RealValue(double min_value, double max_value, bool ziggurat);
  static int             DiscreteValue(int* distribution, int n);

protected:
  Random() { }
  friend class Destroyer<Random>;
  virtual ~Random() { }

private:
  static Random* _instance;
  static std::default_random_engine _engine; 
  static unsigned long _random_seed; 
  static unsigned _random_seed_zig; 
  static Destroyer<Random> _destroyer;
  static void _init();

  // Ziggurat algorithm variables                                                          
  static float _fn[128];
  static uint32_t _kn[128];
  static float _wn[129];

};

#endif
