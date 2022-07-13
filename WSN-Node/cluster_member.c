// implementation of the cluster member
#include "cluster_member.h"

int Event_Generator(int ran){
	if (ran >= Threshold_low && ran <= Threshold_upp){
		return 1;
	}
	else{
		return 0;
	}
}

int Random_Input(){
	return rand() % 100;
}
