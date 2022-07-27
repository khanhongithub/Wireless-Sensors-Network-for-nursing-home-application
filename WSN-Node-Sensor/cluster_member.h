// Cluster member running functions:
//
// event generator: use the random number generator every cycle of polling
// *sensor event detection: check if the sensor value exceed the threshold
// *sensor value report: return the average value of the sensor in recent time
// *fail detection: to decide the failure of cluster head.

//******Head files*******//
// Contiki-specific includes:
#include "contiki.h"
#include "dev/adc-zoul.h"      // ADC
#include "dev/zoul-sensors.h"  // Sensor functions
#include "dev/sys-ctrl.h"
// Standard C includes:
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "math.h"

int Random_Input();

int Get_Sensor_Status();

void Update_Status(int status);

uint16_t getVarianceValue(uint16_t adc_input[20]);

int FindBPM(uint16_t adc_input[]);

void Threshold(int bpm);
