#ifndef PEAKS_H
#define PEAKS_H

#include <vector>

class peaks {

public:
  int npeaks;
  uint32_t trigger_number;
  uint16_t trigger_type;
  uint32_t nadc_values;
  std::vector<double> peak_heights;
  std::vector<double> peak_times;
};

#endif

