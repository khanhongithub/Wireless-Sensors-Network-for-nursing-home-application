// Cluster member running functions:
//
// sync frame broadcasting
// *gateway report: send packet to gateway
// *data aggregation: collect the data from the
// *fail detection: to decide the failure of cluster member.

//******Head files*******//
#include "contiki.h"
#include "cluster_member.h"

#include <stdint.h>

typedef struct{
	uint8_t node_list[6];
	int node_bool[6];
	uint8_t node_status[6];
}report_gateway;

// functions
void Cal_Duty();

void Event_record(int status);

void Reset_Duty(int duty);

int Remain_Duty();

int Need_Report();

uint8_t Get_Next_CH(uint8_t curr_head);

int Node_Is_Free();

void Set_Free(int status);

void Topology_Record(uint8_t id);

void Status_Record(uint8_t id, uint8_t status);

void Setup_Topo(uint8_t node_id);

report_gateway *Prepare_Gate_Report(report_gateway *packet);

void Reset_Node_Status();

void Reset_Topo();

void print_topo();
