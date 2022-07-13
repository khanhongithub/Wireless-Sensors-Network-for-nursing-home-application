#include "contiki.h"
#include "net/rime/rime.h"	// Establish connections.
#include "net/netstack.h"
#include "lib/random.h"
#include "dev/leds.h"

// Standard C includes:
#include <stdio.h>
#include <stdint.h>

// Other C files
#include "cluster_member.h"
#include "cluster_head.h"
#include "helpers.h"

// Define parameters of the network
#define BROADCAST_RIME_CHANNEL 129
#define UNICAST_RIME_CHANNEL 146
#define BROADCAST_INTERVAL 2
#define DELAY_INTERVAL 0.5
#define CHANNEL 14
#define POWER 3
#define MAX_N 20
#define MAX_PAYLOAD 80

// global variable
static uint8_t node_id;		        // Stores node id
static uint8_t head_id = 1;             // Stores head id

int is_cluster_head(){
	return node_id == head_id;
}

int from_cluster_head(uint8_t from){
	return (from == head_id) && (node_id != head_id);
}
// packet content
typedef struct{
	char type[MAX_N];
	char content[MAX_N];
	uint8_t nodeid;
}syncing;

static syncing tx_contacts;

typedef struct{
	uint8_t nodeid;
	char type[MAX_N];
	char content[MAX_N];
	uint8_t value;
}data_report;

static data_report tx_reports;


// Creates an instance of a broadcast connection.
static struct unicast_conn unicast;
static struct broadcast_conn broadcast;

// process
PROCESS(sync_broadcasting, "Sync frame broadcasting of the sensor code.");
PROCESS(uni_reporting, "Used by CM, report to CH.");
PROCESS(head_change, "Run when the duty time of current CH finished.");
PROCESS(node_initial, "Used for node initialisation");

AUTOSTART_PROCESSES(&sync_broadcasting, &uni_reporting, &head_change, &node_initial);


// Defines the behavior of a cluster member received syncing frame.
static void reporting_recv(struct unicast_conn *c, const linkaddr_t *from){

	leds_on(LEDS_BLUE);
	/* Get packet length */
	uint8_t len = strlen( (char *)packetbuf_dataptr() );
	/* Get packet's RSSI */
	int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	/* Print address of sending node */
	printf("Got RX packet (unicast) from: 0x%x%x, len: %d, RSSI: %d\r\n",
			from->u8[0], from->u8[1],len,rssi);

	static data_report rx_contacts;
	packetbuf_copyto(&rx_contacts);

	printf("Node Id: %d \n\r",rx_contacts.nodeid);
	printf("Event Type: %s \n\r",rx_contacts.type);
	printf("Read Value: %d \n\r",rx_contacts.value);



	// wait to think how to implement.
	// depends on the type of unicast, report or collect,
	// do report to gateway or data aggregation and then send to gateway



	leds_off(LEDS_BLUE);
}

static const struct unicast_callbacks unicast_call = {reporting_recv};

// define the behavior of node received
static void syncing_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
	leds_on(LEDS_GREEN);
	/* Get packet length */
	uint8_t len = strlen( (char *)packetbuf_dataptr() );
	/* Get packet's RSSI */
	int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	/* Print address of sending node */
	printf("Got RX packet (broadcast) from: 0x%x%x, len: %d, RSSI: %d\r\n",
			from->u8[0], from->u8[1],len,rssi);
    leds_off(LEDS_GREEN);

	if(from_cluster_head(from->u8[1])){
		static syncing rx_contacts;
		packetbuf_copyto(&rx_contacts);

		printf("%s \n\r",rx_contacts.type);
		// detect content
		if(strcmp(rx_contacts.type, "Syncing") == 0){
			int value = Random_Input();
			if(Event_Generator(value)){
				printf("Need report with value: ");
				tx_reports.nodeid = node_id;
			    tx_reports.value = value;
		    	printf("%d \n\r", tx_reports.value);
				process_post(&uni_reporting, PROCESS_EVENT_MSG, 0);
			}
			else{
				printf("no report.\n\r");
			}
		}
		else if(strcmp(rx_contacts.type, "Switching") == 0){
			printf("next, %d\n\r", rx_contacts.nodeid);
			head_id = rx_contacts.nodeid;
			if(rx_contacts.nodeid == node_id){
				printf("i am next!\n\r");
				Reset_Duty(0);
				process_post(&sync_broadcasting, PROCESS_EVENT_MSG, 0);
			}
		}
		else{
			printf("unexpected message.\n\r");
		}
	}
}

// Defines the functions used as callbacks for a broadcast connection.
static const struct broadcast_callbacks syncing_call = {syncing_recv};

// defines the behavior of cluster member received ch change message
/*
static void c_change_recv(struct broadcast_conn *c, const linkaddr_t *from){
	;
}

static const struct broadcast_callbacks c_change_call = {c_change_recv};
*/


PROCESS_THREAD(uni_reporting, ev, data) {

	node_id = (linkaddr_node_addr.u8[1] & 0xFF);
	static uint8_t report_delay_interval = DELAY_INTERVAL;

	PROCESS_EXITHANDLER(unicast_close(&unicast);)

	static struct etimer delay;

	PROCESS_BEGIN();
	// Configure your team's channel (11 - 26).
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, 3);

	unicast_open(&unicast, UNICAST_RIME_CHANNEL, &unicast_call);

	while(1){
		PROCESS_WAIT_EVENT();
		linkaddr_t addr;
		if(ev == PROCESS_EVENT_MSG){
		    strcpy(tx_reports.type, "EVENT");
		    printf("Packet from %d ", node_id);
		    etimer_set(&delay, report_delay_interval * CLOCK_SECOND * node_id);
		    leds_on(LEDS_RED);
		}
		else if (ev == PROCESS_EVENT_TIMER){
			if (etimer_expired(&delay)){
		    	addr.u8[0] = (head_id >> 8) & 0xFF;
		        addr.u8[1] = head_id & 0xFF;

		        packetbuf_copyfrom(&tx_reports, 100);
		        unicast_send(&unicast, &addr);
		        printf("Send.\n\r");
		        leds_off(LEDS_RED);
		    }
		}
	}

	PROCESS_END();
}

PROCESS_THREAD(sync_broadcasting, ev, data) {

	static uint8_t broadcast_time_interval = BROADCAST_INTERVAL;
	static struct etimer sync_timer;
	node_id = (linkaddr_node_addr.u8[1] & 0xFF);

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();

    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, 3);

    broadcast_open(&broadcast, BROADCAST_RIME_CHANNEL, &syncing_call);

    while(1) {
    	PROCESS_WAIT_EVENT();
    	if (node_id == head_id && ev == PROCESS_EVENT_MSG){
    		etimer_set(&sync_timer, broadcast_time_interval * CLOCK_SECOND);
    	}
    	else if (node_id == head_id && ev == PROCESS_EVENT_TIMER) {
        	if (etimer_expired(&sync_timer)){
        		if (Remain_Duty()) {
     		       	leds_on(LEDS_BLUE);
    	        	//fill the packet buffer
      		      	strcpy(tx_contacts.type, "Syncing");
      		      	packetbuf_copyfrom(&tx_contacts ,10);

            		broadcast_send(&broadcast);

            		printf("Sync message sent in channel %d with power: %d\r\n", 14, 3);
            		etimer_reset(&sync_timer);
            		leds_off(LEDS_BLUE);
        		}
        		else{
        			printf("duty finished.\n\r");
        			process_post(&head_change, PROCESS_EVENT_MSG, 0);
        		}
        	}

        }
        // add more function to loop the process it self.
    }
    PROCESS_END();
}

PROCESS_THREAD(head_change, ev, data){

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();

	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, 3);

	broadcast_open(&broadcast, BROADCAST_RIME_CHANNEL, &syncing_call);

	static uint8_t switching_delay_interval = DELAY_INTERVAL;
	static struct etimer delay;

	while(1){
		PROCESS_WAIT_EVENT();
		if (ev == PROCESS_EVENT_MSG){
			leds_on(LEDS_YELLOW);
			etimer_set(&delay, switching_delay_interval * CLOCK_SECOND);
		}
		else if (ev == PROCESS_EVENT_TIMER){
			if (etimer_expired(&delay)){
				strcpy(tx_contacts.type, "Switching");
				head_id = Get_Next_CH(head_id);
				tx_contacts.nodeid = head_id;
				printf("H_Node %d:, Change head to %d\n\r", node_id, tx_contacts.nodeid);

				packetbuf_copyfrom(&tx_contacts ,100);
				broadcast_send(&broadcast);

				printf("Switch message sent in channel %d with power: %d\r\n", 14, 3);
				leds_off(LEDS_YELLOW);
			}
		}
	}

	PROCESS_END();
}

PROCESS_THREAD(node_initial, ev, data){

	node_id = (linkaddr_node_addr.u8[1] & 0xFF);

	PROCESS_EXITHANDLER( printf("This where we start!\n\r");)
	PROCESS_BEGIN();

	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, 3);

	static uint8_t initial = DELAY_INTERVAL;
	static uint8_t overhead = BROADCAST_INTERVAL;

	static struct etimer delay;
	etimer_set(&delay, (initial * node_id + overhead) * CLOCK_SECOND);

	while(1){
		if(node_id == head_id){
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&delay));
		leds_on(LEDS_GREEN);
		process_post(&sync_broadcasting, PROCESS_EVENT_MSG, 0);
		leds_off(LEDS_GREEN);
		}
		else{
		    PROCESS_WAIT_EVENT();
		}
	}

	PROCESS_END();
}


// new process faliure counter: similar to the sync broadcasting in structure,
// set a num to decrease every etimer, and reset when msg received.
// when 0 reached, run process node_initial.

