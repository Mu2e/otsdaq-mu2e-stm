#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <time.h>
#include <TStopwatch.h>
#include <math.h>
#include <TH1F.h>
#include <TTimeStamp.h>
#include <TTime.h>

void testclock() {
  TStopwatch t;
  t.Start();
  double sum =0;
  for(long int j=0;j<100000000;j++){
    sum+=pow(j,3);
  }
  TH1F*h2 = new TH1F("TH2","", 100, 0, 1000);
  for(long int j = 0; j < 500000000; ++j){
    h2->Fill(j);}
  t.Stop();
  t.Print();
  t.Start();
  double sum2 =0;
  for(long int j=0;j<100000000;j++){
    sum2+=pow(j,3);
  }
  t.Stop();
  t.Print();
}
