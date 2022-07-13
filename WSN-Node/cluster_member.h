// Cluster member running functions:
//
// event generator: use the random number generator every cycle of polling
// *sensor event detection: check if the sensor value exceed the threshold
// *sensor value report: return the average value of the sensor in recent time
// *fail detection: to decide the failure of cluster head.

//******Head files*******//
#include "contiki.h"
#include "routing.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// event probability
#define Threshold_upp 80
#define Threshold_low 30

//static uint8_t event_counter = 0;
//static uint8_t duty_counter = 0;

// event_generator: use the random number generator every cycle of polling
int Event_Generator();

int Sensor_Input();

int Random_Input();

//uint_8 Value_Report();

