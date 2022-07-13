#include "cluster_head.h"

// variables
static uint8_t duty_cycle = 5;
static int node_free = 1;

static uint8_t node_list[] = {1, 2, 3, 4, 5, 6};
static int node_bool[6] = {1, 1, 1, 0, 0, 0};


	// functions used to calculate the duty time
	// depends on: event counter, duty counter, preset event probability, remaining energy*
void Cal_Duty(){

	duty_cycle = 5;
}

// checking the remaining duty time, returns 1 when >0,
int Remain_Duty(){
	if(duty_cycle > 0 ){
		duty_cycle--;
		printf("Remaining duty time after this round: %d\n\r", duty_cycle);
		return 1;
	}
	else{
		printf("No more duty time, run CH switch!\n\r");
		return 0;
	}
}

// used to reset the duty time when input is 0, others resets manually.
void Reset_Duty(int duty){
	if(duty == 0){
		Cal_Duty();
		printf("Reset by Cal_Duty().\n\r");
	}
	else if(duty > 0){
		duty_cycle = duty;
		printf("Reset manually.\n\r");
	}
	else{
		printf("Invalid input value. Reset fail.\n\r");
	}
}

// use to get the next
uint8_t Get_Next_CH(uint8_t curr_head){
	int found = 0;
	uint8_t next_id = 1;

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

	return next_id;
}
