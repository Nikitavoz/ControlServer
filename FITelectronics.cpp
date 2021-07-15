#include "FITelectronics.h"

//global variables
double systemClock_MHz = LHCclock_MHz;
double TDCunit_ps = 1e6 / 30 / 64 / systemClock_MHz; // 13 ps
double halfBC_ns = 500. / systemClock_MHz;
double phaseStepLaser_ns = halfBC_ns / 1024;
double phaseStep_ns = phaseStepLaser_ns;
