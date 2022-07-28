#include "cluster_head.h"

// variables
static uint8_t self_id;
static uint8_t duty_cycle = 3;
static int node_free = 1;
static int report_cycle = 4;

static uint8_t node_list[6] = {1, 2, 3, 4, 5, 6};
static int node_bool[6] = {0, 0, 0, 0, 0, 0};
static uint8_t node_status[6] = {9, 9, 9, 9, 9, 9};
static int node_default[6] = {0, 0, 0, 0, 0, 0};

static int check_counter = 100;
static int event_counter = 80;

// functions used to calculate the duty time
// depends on: event counter, check counter, preset event probability
void Cal_Duty(){
	int prob = 10 - (10 * event_counter) / check_counter;
	duty_cycle = 3 + prob;
}

void Event_record(int status){
	if (status == 1){
		event_counter = event_counter + 1;
		check_counter = check_counter + 1;
	}
	else{
		check_counter = check_counter + 1;
	}
}

// checking the remaining duty time, returns 1 when >0,
int Remain_Duty(){
	if(duty_cycle > 0){
		duty_cycle--;
		printf("[CH_Remain_Duty] Remaining duty time after this round: %d\n\r", duty_cycle);
		return 1;
	}
	else{
		printf("[CH_Remain_Duty] No more duty time, run CH switch!\n\r");
		return 0;
	}
}

// at the end or beginning of duty, collect the data
int Need_Report(){
	int true = 0;

	if (report_cycle == 4){
		true = 1;
		report_cycle--;
	}
	else if (report_cycle == 0){
		report_cycle = 4;
	}
	else{
		report_cycle--;
	}

	if (duty_cycle == 0){
		true = 1;
	}
	return true;
}

// used to reset the duty time when input is 0, others resets manually.
void Reset_Duty(int duty){
	report_cycle = 4;

	if(duty == 0){
		Cal_Duty();
		int seed = event_counter * self_id;
		srand(seed);
		printf("[CH_Reset_Duty] Reset by Cal_Duty().   ");
		printf("Check:%d Event:%d duty_cycle:%d \n\r", check_counter, event_counter, duty_cycle);
	}
	else if(duty > 0){
		duty_cycle = duty;
		int seed = event_counter * self_id;
		srand(seed);
		printf("[CH_Reset_Duty] Reset manually.   ");
		printf("Check:%d Event:%d duty_cycle:%d \n\r", check_counter, event_counter, duty_cycle);
	}
	else{
		duty_cycle = 3;
		printf("[CH_Reset_Duty] Invalid input value. Reset to default value.\n\r");
	}
}

// use to get the next cluster head id
uint8_t Get_Next_CH(uint8_t curr_head){
	int found = 0;
	uint8_t next_id = 0;

	if (curr_head < 6){
		for (uint8_t i = curr_head; i<6; i++){
        	if (node_bool[i]){
        		next_id =  node_list[i];
    			found = 1;
    			break;
    		}
    	}
	}
	if (curr_head == 6 || !found){
		for (uint8_t i = 0; i<6; i++){
		    if (node_bool[i]){
		    	next_id =  node_list[i];
		    	found = 1;
		    	break;
		    }
		}
	}

	if (!found){
		next_id = curr_head;
	}

	return next_id;
}

// function returns the status of node is inside a cluster or not. 1 for free, 0 for not
int Node_Is_Free(){
	return node_free;
}

// use to change the status of the node 1 for free, 0 for not
void Set_Free(int status){
	if (status == 1 || status == 0){
		node_free = status;
	}
	else{
		printf("[CH_Set_Free] Invalid node status value. \n\r");
	}
}

void Topology_Record(uint8_t id){
	id = id -1;
	node_bool[id] = 1;
}

void Status_Record(uint8_t id, uint8_t status){
	id = id - 1;
	node_status[id] = status;
}

void Setup_Topo(uint8_t node_id){
	self_id = node_id;
	srand(rand() % node_id);

	event_counter = 30 + rand() % 30;
	int seed = event_counter * node_id;
	srand(seed);

    int index = self_id - 1;
    for (int i=0; i<6; i++){
    	if(i == index){
    		node_default[i] = 1;
    	}
    	else{
    		node_default[i] = 0;
    	}
    }
}

report_gateway *Prepare_Gate_Report(report_gateway *packet){
    for (int i=0; i<6; i++){
    	packet->node_list[i] = node_list[i];
    	packet->node_bool[i] = node_bool[i];
    	packet->node_status[i] = node_status[i];
    }
    return packet;
}

report_gateway *Prepare_Emergency_Report(report_gateway *packet){
	int index = self_id - 1;
    for (int i=0; i<6; i++){
    	packet->node_list[i] = 0;
    	packet->node_bool[i] = node_default[i];
    	if (i == index){
    		packet->node_status[i] = 3;
    	}
    	else{
    		packet->node_status[i] = 9;
    	}
    }
    return packet;
}

void Reset_Node_Status(){
	for (int i=0; i<6; i++){
	    node_status[i] = 9;
	}
}

void Reset_Topo(){
	for (int i=0; i<6; i++){
	    node_bool[i] = node_default[i];
	}
}

void print_topo(){
	for (int i=0; i<6; i++){
		printf("%d ", node_bool[i]);
    }
}
















