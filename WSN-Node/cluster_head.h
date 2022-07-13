// Cluster member running functions:
//
// sync frame broadcasting
// *gateway report: send packet to gateway
// *data aggregation: collect the data from the
// *fail detection: to decide the failure of cluster member.

//******Head files*******//
#include "contiki.h"
#include "routing.h"
#include "cluster_member.h"

#include <stdint.h>

// functions
void Cal_Duty();

void Reset_Duty(int duty);

int Remain_Duty();

uint8_t Get_Next_CH(uint8_t curr_head);
