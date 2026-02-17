// discrete_distribution with a weight for each value
#include <iostream>
#include <chrono>
#include <random>

int main()
{
  const int nrolls = 1000; // number of experiments
  //const int nstars = 1000;   // maximum number of stars to distribute

  std::default_random_engine generator; //with this generator get always the same result for the distribution
  
  //construct a trivial random generator engine from a time-based seed:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator2 (seed); //with this generator get a different distribution every time we run it

  std::discrete_distribution<int> distribution[10];
  //wheights (probability of having 0 to 11 comptons of photoelectric effects at different energies)
  distribution[0]= {0,762,196,37,4,1,0,0,0,0,0,0}; //100keV X-rays
  distribution[1]= {0,381,310,185,88,23,9,3,1,0,0,0}; //200keV X-rays 
  distribution[2]= {5,205,244,228,165,96,39,12,2,3,1,0}; //300keV X-rays 
  distribution[3]= {13,156,184,244,176,128,54,31,11,2,1,0}; //400keV X-rays 
  distribution[4]= {17,124,186,232,210,124,63,29,11,3,1,0}; //500keV X-rays 
  distribution[5]= {24,119,161,211,196,147,68,48,17,7,2,0}; //600keV X-rays 
  distribution[6]= {28,112,154,221,203,137,84,39,12,6,3,0}; //700keV X-rays 
  distribution[7]= {40,110,160,171,227,154,74,38,14,11,1,0}; //800keV X-rays 
  distribution[8]= {32,124,167,187,178,157,81,49,11,10,4,0}; //900keV X-rays 
  distribution[9]= {45,125,153,186,165,146,101,43,25,6,2,3}; //1000keV X-rays 
 
 
  

  int p[12]={};

  for (int i=0; i<nrolls; ++i) {
    //Choose the distribution according to the energy of peaks simulated
    int number = distribution[9](generator2);
    std::cout<<number<<std::endl;
    ++p[number];
  }

  std::cout << "a discrete_distribution:" << std::endl;
  for (int i=0; i<12; ++i)
    //std::cout << i << ": " << std::string(p[i]*nstars/nrolls,'*') << std::endl;
    std::cout << i << ": " << std::string(p[i],'*') << std::endl;  


  return 0;






}
