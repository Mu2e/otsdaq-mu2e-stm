/////////////////////////////////////////////////////////////////////////
/// This header provides the configurtion information for Mu2e
/////////////////////////////////////////////////////////////////////////

#ifndef MU2E_CONFIG_hh
#define MU2E_CONFIG_hh

#include <iostream>
#include <stdlib.h>

// Data vars
#include "STMDAQ-TestBeam/utils/dataVars.hh"

// On-spill event width (us)
//static const double onEventWidth = 1.6949722; // 1695 ns
static const double onEventWidth1 = 1.700; // 1700 ns
static const double onEventWidth2 = 1.675; // 1675 ns
static const double onEventCycle = 4*onEventWidth1 + 1*onEventWidth2;
static const double onEventWidth = onEventCycle/5;

// Spill width (us)
static const double spillWidthInit = 43120; // 43.12 ms

// Number of events in an on-spill cycle
static const double onEventCycleNum = ceil(spillWidthInit/onEventCycle);

// Number of events per spill
static const uint onEventsSpill = onEventCycleNum*5;

static const double spillWidth = 4*onEventCycleNum*onEventWidth1
  +onEventCycleNum*onEventWidth2;

// Off-spill event width (us)
static const double offEventWidth = 100; // 100 us

// Regular spill gap(us)
static const double regSpillGap = 5000; // 5 ms

// Number of off-spill events per regular spill gap
static const uint offEventsRegGap = regSpillGap/offEventWidth;

// Gap between spill 4 and 5 (us)
//static const double spillGap4to5 = 41880; // 41.88 ms

// Number of off-spill events in gap between spill 4 and 5
//static const uint offEventsGap4to5 = spillGap4to5/offEventWidth;

// Gap between spill 8 and 1 (us)
static const double spillGap8to1 = 1020000; // 1020 ms
//static const double spillGap8to1 = 983000; // 983 ms

// Number of off-spill events in gap between spill 8 and 1
static const uint offEventsGap8to1 = spillGap8to1/offEventWidth;

// Number of spills per MI cycle
static const uint spillsPerMIcycle = 8;
 
// Total number of on and off spill events per MI cycle
static const uint totEventsMIcycle = onEventsSpill + offEventsRegGap + // Spill 1
  onEventsSpill + offEventsRegGap + // Spill 2
  onEventsSpill + offEventsRegGap + // Spill 3
  onEventsSpill + offEventsRegGap + // Spill 4
  onEventsSpill + offEventsRegGap + // Spill 5
  onEventsSpill + offEventsRegGap + // Spill 6
  onEventsSpill + offEventsRegGap + // Spill 7
  onEventsSpill + offEventsGap8to1; // Spill 8

// Total number of on-spill events per MI cycle
static const uint totOnEventsMIcycle = spillsPerMIcycle*onEventsSpill; 

// Total number of off-spill events per MI cycle
static const uint totOffEventsMIcycle = 7*offEventsRegGap + // 7 regular gaps per MI cycle
  offEventsGap8to1; // Gap between spills 8 and 1

// Number of MI cycles per super-cycle
//static const uint numMIcycles = 40;
static const uint numMIcycles = 1;

// Time for one test beam cycle (us)
static const double timeTBcycle = 4*1e6; // 4 secs

// Total time for each MI cycle (us)
static const double totTimeMIcycle = spillWidth + regSpillGap + // Spill 1
  spillWidth + regSpillGap + // Spill 2
  spillWidth + regSpillGap + // Spill 3
  spillWidth + regSpillGap + // Spill 4
  spillWidth + regSpillGap + // Spill 5
  spillWidth + regSpillGap + // Spill 6
  spillWidth + regSpillGap + // Spill 7
  spillWidth + spillGap8to1; // Spill 8

// Total time for each super cycle
static const double totTimeSuperCycle = numMIcycles*totTimeMIcycle // 40 MI cycles
  + timeTBcycle; // ... and one test beam cycle

// Number of distinct super cycles
//static const uint numSuperCycles = 60;
static const uint numSuperCycles = 1;

// Set all initial event modes in MI cycle to 0 (on-spill)
//int16_t* eventModesPerMIcycle;

// On-spill mode
static const uint16_t onMode = 0;
// On-spill mode
static const uint16_t offMode = 1;

// Total number of defined sample periods
// 8*onEventsSpill + 7*offEventsRegGap + offEventsGap8to1 = 16
static const uint totSamplePeriods = 20; 


// Accelator config parameter struct
struct modeConfig{                                                        
  
  // The period number
  uint periodNum;                                                         
  // Mode (on-spill/off-spill)
  uint16_t mode;                                                          
  // The period length
  double period;                                                          
  // Number of events in the period
  uint eventNum;                                                          
  
};  

// Structs of periods and modes for 1 MI cycle
static const struct modeConfig pM[totSamplePeriods] = {
  {0,onMode,spillWidth,onEventsSpill}, // Spill 1, on-spill
  {1,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {2,onMode,spillWidth,onEventsSpill}, // Spill 2, on-spill
  {3,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {4,onMode,spillWidth,onEventsSpill}, // Spill 3, on-spill
  {5,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {6,onMode,spillWidth,onEventsSpill}, // Spill 4, on-spill
  {7,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  //  {7,offMode,spillGap4to5,offEventsGap4to5}, // Gap 4 --> 5, off-spill
  {8,onMode,spillWidth,onEventsSpill}, // Spill 5, on-spill
  {9,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {10,onMode,spillWidth,onEventsSpill}, // Spill 6, on-spill
  {11,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {12,onMode,spillWidth,onEventsSpill}, // Spill 7, on-spill 
  {13,offMode,regSpillGap,offEventsRegGap}, // Regular gap, off-spill
  {14,onMode,spillWidth,onEventsSpill}, // Spill 4, on-spill
  {15,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part1], off-spill
  {16,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part2], off-spill
  {17,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part3], off-spill
  {18,offMode,spillGap8to1/5,offEventsGap8to1/5}, // Gap 8 --> 1 [part4], off-spill
  {19,offMode,spillGap8to1/5,offEventsGap8to1/5}}; // Gap 8 --> 1 [part5] off-spill

#endif
