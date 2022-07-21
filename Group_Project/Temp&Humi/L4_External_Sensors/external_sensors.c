/*
   Wireless Sensor Networks Laboratory

   Technische Universität München
   Lehrstuhl für Kommunikationsnetze
   http://www.lkn.ei.tum.de

   copyright (c) 2017 Chair of Communication Networks, TUM

   contributors:
   * Thomas Szyrkowiec
   * Mikhail Vilgelm
   * Octavio Rodríguez Cervantes
   * Angel Corona

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.0 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   LESSON 3: External Sensors.
*/

// Contiki-specific includes:
#include "contiki.h"
#include "net/rime/rime.h"     // Establish connections.
#include "net/netstack.h"      // Wireless-stack definitions
#include "dev/leds.h"          // Use LEDs.
#include "dev/button-sensor.h" // User Button
#include "dev/adc-zoul.h"      // ADC
#include "dev/zoul-sensors.h"  // Sensor functions
#include "dev/sys-ctrl.h"
// Standard C includes:
#include <stdio.h>      // For printf.
#include <math.h>       // For round

// Reading frequency in seconds.
#define TEMP_READ_INTERVAL CLOCK_SECOND*1

typedef struct{
	uint8_t node_list[6];
	int node_bool[6];
	uint8_t node_status[6];
}packet;

/*** CONNECTION DEFINITION***/
static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from);
static struct unicast_conn unicast;
static const struct unicast_callbacks unicast_call = {unicast_recv};
/**
 * Callback function for received packet processing.
 *
 */



//function for outputting the lux value read from sensor
    		//@param m: calibration value m inscribed on the back of the sensor
    		//@param b: calibration value b inscribed on the back of the sensor
    		//@param adc_input: phidget input value. Use ZOUL_SENSORS_ADC1 or ZOUL_SENSORS_ADC3 depending on where the sensor is connected to.
    		//@return int : lux value with a max of 1000.
static int getTempSensorValue(uint16_t adc_input){
    		//Convert the voltage with the provided formula
    		int temperature = (222.2*adc_input/2048)-66.111;
    		//Return the value of the light with maximum value equal to 1000
    		if (temperature<-30)
			    return -30;
    		else if (temperature>80)
    			return 80;
    		else
    			return temperature;

}

static int getHumiSensorValue(uint16_t adc_input){
    		//Convert the voltage with the provided formula
    		int humidity = (190.6*adc_input/2048)-13.2;
    		//Return the value of the light with maximum value equal to 1000
    		if (humidity<10)
    			return 10;
    		else if (humidity>95)
    		    return 95;
    		else
    		    return humidity;
}

/**
 * Connection information
 */
//static struct broadcast_conn broadcastConn;

/**
 * Assign callback functions to the connection
 */
//static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv};

/*** CONNECTION DEFINITION END ***/

//--------------------- PROCESS CONTROL BLOCK ---------------------
PROCESS (ext_sensors_process, "External Sensors process");
AUTOSTART_PROCESSES (&ext_sensors_process);


//------------------------ PROCESS' THREAD ------------------------

PROCESS_THREAD (ext_sensors_process, ev, data) {

	/* variables to be used */
	static struct etimer temp_reading_timer;
	static uint16_t adc1_value, adc3_value;
	linkaddr_t addr;
	uint8_t gateway_id = 7;
	packet packet_send;

	packet_send.node_list[0] = 9;
	packet_send.node_bool[0] = 9;

	for (int i=1; i<6; i++){
		packet_send.node_list[i] = 0;
		packet_send.node_bool[i] = 0;
	}

	PROCESS_EXITHANDLER( unicast_close(&unicast); )
	PROCESS_BEGIN ();


	printf("\r\nZolertia RE-Mote external sensors");
	printf("\r\n====================================");

	/*
	 * set your group's channel
	 */
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL,14);

	/*
	 * open the connection
	 */
	unicast_open(&unicast,149,&unicast_call);


	/* Configure the ADC ports */
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1 | ZOUL_SENSORS_ADC3);


	etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);

	while (1) {

		PROCESS_WAIT_EVENT();  // let process continue

		/* If timer expired, pront sensor readings */
	    if(ev == PROCESS_EVENT_TIMER) {

	    	leds_on(LEDS_GREEN);

	    	/*
	    	 * Read ADC values. Data is in the 12 MSBs
	    	 */
	    	adc1_value = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;
	    	adc3_value = adc_zoul.value(ZOUL_SENSORS_ADC3) >> 4;

	    	/*
	    	 * Print Raw values
	    	 */

	    	printf("\r\nADC1 value [Raw] = %d", adc1_value);
	        printf("\r\nADC3 value [Raw] = %d", adc3_value);

    		int Temp = getTempSensorValue(adc1_value);
    		int Humi = getHumiSensorValue(adc3_value);

    		printf("\r\nADC1 value [Reverted] = %d\n", Temp);
    		printf("\r\nADC3 value [Reverted] = %d\n", Humi);

    		packet_send.node_status[0] = Temp;
    		packet_send.node_status[1] = Humi;
    		packet_send.node_status[2] = 0;
    		packet_send.node_status[3] = 0;
    		packet_send.node_status[4] = 0;
    		packet_send.node_status[5] = 0;

    		addr.u8[0] = (gateway_id >> 8) & 0xFF;
    		addr.u8[1] = gateway_id & 0xFF;

    		packetbuf_copyfrom(&packet_send, 100);
    		unicast_send(&unicast, &addr);


    		leds_off(LEDS_GREEN);

    		etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	    }
    }

	PROCESS_END ();
}


//functions
static void
unicast_recv(struct unicast_conn *c, const linkaddr_t *from) {

}


