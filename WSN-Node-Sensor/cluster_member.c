// implementation of the cluster member
#include "cluster_member.h"

static int now_status = 0;
static int report_status = 2;

int Random_Input(){
	return rand() % 3;
}

int Get_Sensor_Status(){
	int now = report_status;
	if (report_status == 1){
		report_status = now_status;
	}
	return now;
}

void Update_Status(int status){
	now_status = status;
	if (report_status != 1){
		report_status = now_status;
	}
	else {
		report_status = 1;
	}
}

uint16_t getVarianceValue(uint16_t adc_input[20]){
	int sum = 0;
	int length = 0;
	int average = 0;

    for (int m = 0; m < 20; m++){
    	sum += adc_input[m];
    }

    length = 20;
    average = sum/length;

    return average;
}
