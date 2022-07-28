// implementation of the cluster member
#include "cluster_member.h"

static int now_status = 0;
static int report_status = 2;
static int low_thres = 50;
static int upp_thres = 200;

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

void Threshold(int bpm){
	if (bpm != 0){
		if(bpm <= upp_thres && bpm >= low_thres){
			Update_Status(0);
		}
		else{
			Update_Status(1);
		}
	}
	else{
		Update_Status(2);
	}
}

uint16_t getVarianceValue(uint16_t adc_input[]){
	int sum = 0;
	uint16_t length = 0;
	int average = 0;
	uint16_t var = 0;

	for (int i = 0; i <=39; i++){
    	sum += adc_input[i];
    }

    length = 40;
    average = sum/length;

    for (int j = 0; j <= 39; j++){
    	var += abs(adc_input[j]-average)*abs(adc_input[j]-average);
    	var = var/length;
    }

    return var;
}

int FindBPM(uint16_t adc_input[]){
    int a = 0;
    int b = 0;
    uint16_t sum = 0;
    uint16_t length = 0;
   	int average = 0;
    int threshold = adc_input[0];

    for (int j = 0; j < 40; j++){
    	if (adc_input[j]>threshold){
    		threshold = adc_input[j];
    	}
    	sum += adc_input[j];
    }

	length = 40;
	average = sum/length;
    threshold = threshold - (average/5);

    for (int m=1; m < 40; m++){
        if (adc_input[m-1]<=threshold && adc_input[m]>threshold){
            a = m;
    	    break;
    	}
    }

    if (a < 39){
        for (int n=a+1; n<40; n++){
    	    if (adc_input[n-1]<=threshold && adc_input[n]>threshold){
    		    b = n;
    		    break;
    	    }
        }
    }

    if ((a != 0 && b != 0)&&(a != b)){
    	 return 60/((b-a)*0.05);
    }
    else {
    	return 0;
    }
}
