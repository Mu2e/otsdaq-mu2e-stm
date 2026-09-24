#ifndef STATS_H
#define STATS_H
#include <iostream>
#include <cmath>


class stats {
public :

  static double mean(double* x, int n) {
    double sum = 0.0;
    double N = (double) n;
    for (int i = 0; i < n; i++){
      sum = sum + x[i];
    }
    return sum/(double) N;
  }

  static double mean(std::vector<double> x){
    double sum = 0.0;
    for (unsigned int i = 0; i < x.size(); i++){
      sum = sum + x[i];
    }
    return sum/(double) x.size();
  }

  static double rms(double* x, int n) {
    double mean_val = mean(x,n);
    return rms(x, mean_val, n);
  }

  static double rms(double* x, double mean_val, int n) {
    if (n <= 1) {
      std::cout << "ERROR: stats::rms - trying to calculate rms of array with one or fewer elements : " << n << std::endl;
      exit(-1);
    }
    double variance = 0.0;
    for(int i = 0; i < n; i++){
      variance += pow(x[i] - mean_val, 2);
    }
    double N = (double) n;
    return sqrt(variance/N);
  }

  static double rms(std::vector<double> x) {
    double mean_val = mean(x);
    return rms(x, mean_val);
  }

  static double rms(std::vector<double> x, double mean_val) {
    if (x.size() <= 1) {
      std::cout << "ERROR: stats::rms - trying to calculate rms of array with one or fewer elements : " << x.size() << std::endl;
      exit(-1);
    }
    double variance = 0.0;
    for(unsigned int i = 0; i < x.size(); i++){
      variance += pow(x[i] - mean_val, 2);
    }
    double N = (double) x.size();
    return sqrt(variance/N);
  }


};

#endif
