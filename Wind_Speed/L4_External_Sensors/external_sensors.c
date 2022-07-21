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
#include <stdlib.h>

// Reading frequency in seconds.
#define TEMP_READ_INTERVAL CLOCK_SECOND*0.2

// The packet stucture
typedef struct{
	uint8_t node_list[6];
	int node_bool[6];
	uint8_t node_status[6];
}packet;

/*** CONNECTION DEFINITION***/

/**
 * Callback function for received packet processing.
 *
 */


//function for outputting the lux value read from sensor
    		//@param m: calibration value m inscribed on the back of the sensor
    		//@param b: calibration value b inscribed on the back of the sensor
    		//@param adc_input: phidget input value. Use ZOUL_SENSORS_ADC1 or ZOUL_SENSORS_ADC3 depending on where the sensor is connected to.
    		//@return int : lux value with a max of 1000.


int windspeed_buffer[20];
int i = 0;
int n = 0;
int wind_speed;

/**
 * Connection information
 */
//static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from);
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);
//static struct unicast_conn unicast;
static struct broadcast_conn broadcastConn;
//static const struct unicast_callbacks unicast_call = {unicast_recv};
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/**
 * Assign callback functions to the connection
 */
//static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv};
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {

	leds_on(LEDS_GREEN);

	uint8_t len = strlen( (char *)packetbuf_dataptr() );
	int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

	printf("Got RX packet (broadcast) from: 0x%x%x, len: %d, RSSI: %d\r\n",from->u8[0], from->u8[1],len,rssi);

	leds_off(LEDS_GREEN);
}
/*** CONNECTION DEFINITION END ***/

//--------------------- PROCESS CONTROL BLOCK ---------------------
PROCESS (ext_sensors_process, "External Sensors process");

AUTOSTART_PROCESSES (&ext_sensors_process);


//------------------------ PROCESS' THREAD ------------------------


PROCESS_THREAD (ext_sensors_process, ev, data) {

	/* variables to be used */
	linkaddr_t addr;
	uint8_t gateway_id = 7;
	static struct etimer temp_reading_timer;
	packet packet_send;
	for (int i=0; i<6; i++){
		packet_send.node_list[i] = 8;
		packet_send.node_bool[i] = 8;
	}
	//static uint16_t adc1_value;
	PROCESS_EXITHANDLER( broadcast_close(&broadcast_call); )
	PROCESS_BEGIN ();


	printf("\r\nZolertia RE-Mote external sensors");
	printf("\r\n====================================");

	/*
	 * set your group's channel
	 */
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL,14);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER,5);
	/*
	 * open the connection
	 */
	broadcast_open(&broadcastConn,149,&broadcast_call);


	/* Configure the ADC ports */
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1);


	etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);

	while (1) {

		PROCESS_WAIT_EVENT();  // let process continue

		/* If timer expired, pront sensor readings */
	    if(ev == PROCESS_EVENT_TIMER) {

	    	//leds_on(LEDS_PURPLE);

	    	/*
	    	 * Read ADC values. Data is in the 12 MSBs
	    	 */
	    	windspeed_buffer[i] = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;

	    	if (i>=19){
	    		leds_on(LEDS_GREEN);
	    		for (int j=0;j<19;j++)
	    		{
	    			if (windspeed_buffer[j+1]-windspeed_buffer[j]>1500){
	    				n++;
	    			}
	    		}
	    		wind_speed = n*0.6;
	    		packet_send.node_status[0] = wind_speed;
	    		addr.u8[0] = (gateway_id >> 8) & 0xFF;
	    		addr.u8[1] = gateway_id & 0xFF;



	    		packetbuf_copyfrom(&packet_send, 100);
	    		broadcast_send(&broadcastConn);

	    		leds_off(LEDS_GREEN);

                i = 0;
                n = 0;
	    		}

	    	else{
	            i++;
	    	}



    		//leds_off(LEDS_PURPLE);


    		etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	    }
    }

	PROCESS_END ();
}




