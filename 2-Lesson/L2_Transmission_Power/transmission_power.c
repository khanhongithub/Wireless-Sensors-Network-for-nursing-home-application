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

   LESSON 2: TRansmission Power
*/

// Contiki-specific includes:
#include "contiki.h"
#include "net/rime/rime.h"	// Establish connections.
#include "lib/random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/cc2538-rf.h"

// Standard C includes:
#include <stdio.h>
#include <stdint.h>


/*** CONNECTION DEFINITION***/

/**
 * Callback function for received packet processing.
 *
 */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {

	leds_on(LEDS_GREEN);

	uint8_t len = strlen( (char *)packetbuf_dataptr() );
	int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

	printf("Got RX packet (broadcast) from: 0x%x%x, len: %d, RSSI: %d\r\n",from->u8[0], from->u8[1],len,rssi);

	leds_off(LEDS_GREEN);
}

/**
 * Connection information
 */
static struct broadcast_conn broadcastConn;

/**
 * Assign callback functions to the connection
 */
static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv};

/*** CONNECTION DEFINITION END ***/

int power_index[14]= {-24, -15, -13, -11, -9, -7, -5, -3, -1, 0, 1, 3, 5, 7};
int tx_index = 0;

/*** MAIN PROCESS DEFINITION ***/
PROCESS(transmission_power_process, "Lesson 2: Transmission Power");
PROCESS(button_power_change, "Power Changed by button");
AUTOSTART_PROCESSES(&transmission_power_process, &button_power_change);

/*** MAIN THREAD ***/
PROCESS_THREAD(transmission_power_process, ev, data) {

	static struct etimer et;

	PROCESS_EXITHANDLER(broadcast_close(&broadcastConn));
	PROCESS_BEGIN();

	/*
	 * set your group's channel
	 */
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
	/*
	 * Change the transmission power here
	 */
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, power_index[0]); // 5,3,1,-1 ... int value form table
	// radio_value_t a = 0;

	/*
	 * open broadcast connection
	 */
	broadcast_open(&broadcastConn,129,&broadcast_callbacks);

	etimer_set(&et, CLOCK_SECOND + 0.1*random_rand()/RANDOM_RAND_MAX); //randomize the sending time a little

	while(1){

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		leds_on(LEDS_RED);

		/*
		 * fill the packet buffer
		 */
		packetbuf_copyfrom("Hello",6);

		/*
		 * send the message
		 */
		NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, power_index[tx_index]); // 5,3,1,-1 ... int value form table
		broadcast_send(&broadcastConn);

		/*
		 * for debugging
		 */

		// NETSTACK_CONF_RADIO.get_value(RADIO_PARAM_TXPOWER, &a);
		// printf("real power %d\n\r", a);

		printf("Broadcast message sent with power: %d\r\n",power_index[tx_index]); // or the configured Power

		/*
		 * reset the timer
		 */
		etimer_reset(&et);

		leds_off(LEDS_RED);
	}

	PROCESS_END();
}


/*** Use button to change the tx power ***/
PROCESS_THREAD(button_power_change, ev, data){
	PROCESS_BEGIN();
	while(1){
		PROCESS_WAIT_EVENT();
		if(ev == sensors_event) {
		    if(data == &button_sensor) {
		        if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
		                BUTTON_SENSOR_PRESSED_LEVEL) {
		            leds_on(LEDS_GREEN);
		            }
		        else if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
		        		    BUTTON_SENSOR_RELEASED_LEVEL) {
		        	leds_off(LEDS_GREEN);
		        	tx_index = tx_index + 1;
		            if(tx_index > 13){
		            	tx_index = 0;
		            }
		        }
		    }
		}
	}
	PROCESS_END();
}

