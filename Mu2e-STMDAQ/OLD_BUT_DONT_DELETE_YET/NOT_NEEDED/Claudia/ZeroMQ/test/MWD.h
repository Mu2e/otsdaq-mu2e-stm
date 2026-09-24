#ifndef MWD_H
#define MWD_H

#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <iostream>



class MWD {
 public:

  std::vector<double> mwd_alg(std::vector<int16_t> ADC,double tau, double T0, double M, double L);
};





std::vector<double>
  MWD::mwd_alg(std::vector<int16_t> ADC, double tau, double T0, double M, double L){
 

  unsigned long n=ADC.size();

  //Deconvolution
  std::vector<double> a;
  a.clear();

  a.push_back(ADC.at(0));

  for(unsigned long i=1; i<n; i++){
    a.push_back(ADC.at(i)-(1-(T0/tau))*ADC.at(i-1)+a.at(i-1));
   
  }
 
 
 //Differentiation
  std::vector<double> D;
  D.clear();
 
  for (unsigned long i = 0; i < M; ++i) {
    D.push_back(a.at(i));
 
  }

  for (unsigned long i = M; i < n; ++i) {
    D.push_back(a.at(i)-a.at(i-M));

  }

  //Averaging
  std::vector<double> l;
  l.clear();
  double sum = 0.;

  for (unsigned long i = 0; i < L-1; ++i) {
    sum += D.at(i);
    l.push_back(D.at(i));
    }
  
  sum += D.at(L-1);
  l.push_back(sum/L);

  for (unsigned long i = L; i < n; ++i) {
    sum += D.at(i)-D.at(i-L);
    l.push_back(sum/L);
  }


  return l;


}



#endif
