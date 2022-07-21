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

   LESSON 3: Gateway
*/

// Contiki-specific includes:
#include "contiki.h"
#include "dev/leds.h"			// Use LEDs.
#include "sys/clock.h"			// Use CLOCK_SECOND.
#include "net/rime/rime.h"		// Establish connections.
#include "dev/cc2538-rf.h"

// Standard C includes:
#include <stdio.h>			// For printf.

//---------------- FUNCTION PROTOTYPES ----------------

// Broadcast connection setup:
static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from);
//static struct broadcast_conn broadcastConn;
static struct unicast_conn unicast;
//static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv};
static const struct unicast_callbacks unicast_call = {unicast_recv};

// The packet stucture
typedef struct{
	uint8_t node_list[6];
	int node_bool[6];
	uint8_t node_status[6];
}packet_rev;


static char head[40] = "\r\nClusterHead: ";
static char node[40] = "\r\nNode: ";
static char status[40] = "\r\nStatus: ";
static char temphumi[20];
static char wind[20];
static char inter_value[5];
static packet_rev packet;


//--------------------- PROCESS CONTROL BLOCK ---------------------
PROCESS(gateway_main_process, "Lesson 3: Gateway");
AUTOSTART_PROCESSES(&gateway_main_process);

//------------------------ PROCESS' THREADS ------------------------
PROCESS_THREAD(gateway_main_process, ev, data) {

	//PROCESS_EXITHANDLER( broadcast_close(&broadcastConn); )
	PROCESS_EXITHANDLER( unicast_close(&unicast); )
	PROCESS_BEGIN();

	/*
	* set your group's channel
	*/
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);


	unicast_open(&unicast, 149, &unicast_call);

	// If all is OK, we can start the other two processes:

	while(1) {
		// Contiki processes cannot start loops that never end.
		PROCESS_WAIT_EVENT();
	}
	PROCESS_END();
}

//---------------- FUNCTION DEFINITIONS ----------------

// Defines the behavior of a broadcast connection upon receiving data.
static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from) {
	leds_toggle(LEDS_RED);
	leds_on(LEDS_GREEN);


	int rxBytes = 0;

		/*
		 * get the buffered message and message size
		 */
		rxBytes = packetbuf_copyto(&packet);
		printf("%d\r\n", packet.node_list[0]);
		if (rxBytes>0) {

			if (packet.node_list[0]==8)
			{
		    	strcpy(wind, "Windspeed: ");

		    	snprintf(inter_value, sizeof(inter_value), "%d", packet.node_status[0]);
	    		strcat(wind, inter_value);
	    		strcat(wind, "\n");
	    		printf(wind);
			}

			if (packet.node_list[0]==9)
			{
		    	strcpy(temphumi, "TempHumi: ");

		    	snprintf(inter_value, sizeof(inter_value), "%d", packet.node_status[0]);
	    		strcat(temphumi, inter_value);
	    		strcat(temphumi, " ");
		    	snprintf(inter_value, sizeof(inter_value), "%d", packet.node_status[1]);
	    		strcat(temphumi, inter_value);
	    		strcat(temphumi, "\n");

	    		printf(temphumi);
			}

			else {
	    	// generate the packet content
	    	strcpy(head, "ClusterHead: ");
	    	strcpy(node, "Node: ");
	    	strcpy(status, "Status: ");

	    	snprintf(inter_value, sizeof(inter_value), "%d", from->u8[1]);
	    	strcat(head, inter_value);
	    	strcat(head, " ");

	    	for (int i = 0; i<6; i++){
	    		snprintf(inter_value, sizeof(inter_value), "%d", packet.node_bool[i]);
	    		strcat(node, inter_value);
	    		strcat(node, " ");
	    	}

	    	for (int i = 0; i<6; i++){
	    		snprintf(inter_value, sizeof(inter_value), "%d", packet.node_status[i]);
	    		strcat(status, inter_value);
	    		strcat(status, " ");
	    	}

	    	strcat(head, node);
	    	strcat(head, status);
	    	strcat(head, "\n");
	    	printf(head);
			}
		}
	leds_off(LEDS_GREEN);
}



