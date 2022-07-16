// Cluster member running functions:
//
// event generator: use the random number generator every cycle of polling
// *sensor event detection: check if the sensor value exceed the threshold
// *sensor value report: return the average value of the sensor in recent time
// *fail detection: to decide the failure of cluster head.

//******Head files*******//
#include "contiki.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// event_generator: use the random number generator every cycle of polling

int Random_Input();

