#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <random>
#include <list>

using namespace std;
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

// User defined variables

//------------Variables of the pulses generated-----------------------

// ADC samling frequency (MHz)
//const double fADC = 320.0520833313;
const double fADC = 370;  
//Sampling time of ADC (microsec)
const double tadc=1/(fADC);

//The amplitude of the pulse is the half of this value
//const double twiceA=2370;
//const double twiceA=1793;
//The x point in where we have the pulse (100 us)
 double xshift=100;
// double xshift=0; 
//fall from baseline time
const double invtaufall=15;
//decaytime(rise to baseline)
const double invtaudecay=0.028;
// Standard deviation of gaussian noise
//const double noiseSD = 10;
//Pulse duration (microsec)
const double pulseLength=xshift+220;
//Number of bytes in one pulse
unsigned long int pulseBytes=2*pulseLength;







template <typename BidirectionalIterator, typename T>
  BidirectionalIterator getClosest(BidirectionalIterator first, 
				   BidirectionalIterator last, 
				   const T & value)
{
  BidirectionalIterator before = std::lower_bound(first, last, value);

  if (before == first) return first;
  if (before == last)  return --last; // iterator must be bidirectional

  BidirectionalIterator after = before;
  --before;

  return (*after - value) < (value - *before) ? after : before;
}

template <typename BidirectionalIterator, typename T>
  std::size_t getClosestIndex(BidirectionalIterator first, 
			      BidirectionalIterator last, 
			      const T & value)
{
  return std::distance(first, getClosest(first, last, value));
}
