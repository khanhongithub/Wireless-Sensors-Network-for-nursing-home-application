// contiki headers
#include "contiki.h"
#include "net/rime/rime.h"	// Establish connections.
#include "net/netstack.h"
#include "lib/random.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

// Standard C includes:
#include <stdio.h>
#include <stdint.h>

// Other C files
#include "cluster_member.h"
#include "cluster_head.h"

// Define parameters of the network
#define BROADCAST_EMERGENCY_CHANNEL 135
#define BROADCAST_RIME_CHANNEL 129
#define UNICAST_RIME_CHANNEL 146
#define UNICAST_GATEWAY_CHANNEL 149
#define BROADCAST_INTERVAL 4
#define DELAY_INTERVAL 0.5
#define GATEWAY_INTERVAL 3.5
#define CHANNEL 14
#define POWER_TO_GATEWAY 3
#define POWER_REGULAR 1
#define POWER_INITIAL 0
#define MAX_N 20
#define MAX_PAYLOAD 80

// global variable
static uint8_t node_id;		           // Stores node id // @suppress("Type cannot be resolved")
static uint8_t head_id = 255;          // Stores head id // @suppress("Type cannot be resolved")
static uint8_t gateway_id = 7;         // Stores gateway id // @suppress("Type cannot be resolved")

int to_report = 0;                     // used by cm as flag to tell if it is reporting round
int report_end_sync = 0;               // used by ch as flag to tell if send to gateway at this polling

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
	uint8_t value;
}data_report;

static data_report tx_reports;

static report_gateway tx_gateway_report;

// Creates broadcast and unicast connection.
static struct unicast_conn unicast;
static struct broadcast_conn broadcast;

// process
PROCESS(sync_broadcasting, "Sync frame broadcasting of the sensor code.");
PROCESS(uni_reporting, "Used by CM, report to CH.");
PROCESS(head_change, "Run when the duty time of current CH finished.");
PROCESS(node_initial, "Node self-organising the network");
PROCESS(start_node_initial, "Used to start ");
PROCESS(failure_countdown, "used by cm to count a ch failure");
PROCESS(report_to_gateway, "used by ch to report to gateway.");
PROCESS(really_emergency, "press the button and wait for help");

AUTOSTART_PROCESSES(&sync_broadcasting,
		            &uni_reporting,
					&head_change,
					&node_initial,
					&start_node_initial,
					&failure_countdown,
					&report_to_gateway,
					&really_emergency);


// Defines the behavior of a cluster head received reporting frame.
static void reporting_recv(struct unicast_conn *c, const linkaddr_t *from){

	leds_on(LEDS_BLUE);
	/* Get packet length */
	uint8_t len = strlen( (char *)packetbuf_dataptr() );
	/* Get packet's RSSI */
	int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	/* Print address of sending node */
	printf("Got RX packet (unicast) from: 0x%x%x, len: %d, RSSI: %d\r\n", from->u8[0], from->u8[1],len,rssi);

	static data_report rx_uni_report;
	packetbuf_copyto(&rx_uni_report);

	printf("Node Id: %d     ", rx_uni_report.nodeid);
	printf("Event Type: %s     ", rx_uni_report.type);
	printf("Read Value: %d \n\r", rx_uni_report.value);

    Topology_Record(rx_uni_report.nodeid);

    Status_Record(rx_uni_report.nodeid, rx_uni_report.value);

    report_end_sync = 1;

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
	printf("Got RX packet (broadcast) from: 0x%x%x, len: %d, RSSI: %d\r\n", from->u8[0], from->u8[1],len,rssi);
    leds_off(LEDS_GREEN);

    if(Node_Is_Free()){
    	head_id = from->u8[1];
    	printf("received other syncing frame, join the cluster.\n\r");
    	process_post(&failure_countdown, PROCESS_EVENT_MSG, 0);
    	Set_Free(0);
    }

	if(from_cluster_head(from->u8[1])){
		static syncing rx_sync;
		packetbuf_copyto(&rx_sync);

		printf("%s \n\r",rx_sync.type);
		// detect content
		if(strcmp(rx_sync.type, "Sync_Event") == 0){
			process_post(&failure_countdown, PROCESS_EVENT_MSG, 0);
			process_post(&uni_reporting, PROCESS_EVENT_MSG, 0);
		}
		else if(strcmp(rx_sync.type, "Sync_Report") == 0){
			to_report = 1;
			process_post(&failure_countdown, PROCESS_EVENT_MSG, 0);
		    process_post(&uni_reporting, PROCESS_EVENT_MSG, 0);
		}
		else if(strcmp(rx_sync.type, "Switching") == 0){
			process_post(&failure_countdown, PROCESS_EVENT_MSG, 0);
			printf("next, %d\n\r", rx_sync.nodeid);
			head_id = rx_sync.nodeid;
			if(rx_sync.nodeid == node_id){
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

// cm use this process to report to ch
PROCESS_THREAD(uni_reporting, ev, data) {

	node_id = (linkaddr_node_addr.u8[1] & 0xFF);
	static uint8_t report_delay_interval = DELAY_INTERVAL;

	PROCESS_EXITHANDLER(unicast_close(&unicast);)

	static struct etimer delay;


	PROCESS_BEGIN();
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_REGULAR);

	unicast_open(&unicast, UNICAST_RIME_CHANNEL, &unicast_call);

	while(1){
		PROCESS_WAIT_EVENT();
		linkaddr_t addr;
		if(ev == PROCESS_EVENT_MSG){
			int send = 0;
			int status = Random_Input();
			if(to_report){
				printf("[PROCESS_uni_reporting] Data collect. \n\r");
				strcpy(tx_reports.type, "REPORT");
				send = 1;
				to_report = 0;
			}
			else if (status != 0){
				printf("[PROCESS_uni_reporting] Need report Event. \n\r");
		      	strcpy(tx_reports.type, "EVENT");
		        send = 1;
			}
			else {
				printf("[PROCESS_uni_reporting] No report.\n\r");
			}

			if (send){
				tx_reports.nodeid = node_id;
				tx_reports.value = status;
				etimer_set(&delay, report_delay_interval * CLOCK_SECOND * node_id);
				leds_on(LEDS_RED);
			}
		}
		else if (ev == PROCESS_EVENT_TIMER){
			if (etimer_expired(&delay)){
		    	addr.u8[0] = (head_id >> 8) & 0xFF;
		        addr.u8[1] = head_id & 0xFF;

		        packetbuf_copyfrom(&tx_reports, 100);
		        unicast_send(&unicast, &addr);
		        printf("[PROCESS_uni_reporting] Send.\n\r");
		        leds_off(LEDS_RED);
		    }
		}
	}

	PROCESS_END();
}

// used by ch to broadcast sync frame
PROCESS_THREAD(sync_broadcasting, ev, data) {

	static uint8_t broadcast_time_interval = BROADCAST_INTERVAL;
	static struct etimer sync_timer;
	node_id = (linkaddr_node_addr.u8[1] & 0xFF);

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();

    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_REGULAR);

    broadcast_open(&broadcast, BROADCAST_RIME_CHANNEL, &syncing_call);

    while(1) {
    	PROCESS_WAIT_EVENT();
    	if (node_id == head_id && (ev == PROCESS_EVENT_MSG || ev == PROCESS_EVENT_TIMER)){
    		if (Remain_Duty()) {
    		    leds_on(LEDS_BLUE);

    		    if (Need_Report()){
    		        strcpy(tx_contacts.type, "Sync_Report");
    		        report_end_sync = 1;
    		        Reset_Topo();
    		    }
    		    else{
    		    	strcpy(tx_contacts.type, "Sync_Event");
    		    }

    		    process_post(&report_to_gateway, PROCESS_EVENT_MSG, 0);

    		    packetbuf_copyfrom(&tx_contacts ,15);
    		    broadcast_send(&broadcast);
    		    printf("[PROCESS_sync_broadcasting] Sync: %s sent in channel %d with power: %d\r\n", tx_contacts.type, 14, POWER_REGULAR);

    		    // self-event collect.
    		    int status = Random_Input();
    		    Status_Record(node_id, status);
    		    if (status !=0){
    		    	report_end_sync = 1;
    		    	printf("[PROCESS_sync_broadcasting] self-event detected.\n\r");
    		    }

    		    etimer_set(&sync_timer, broadcast_time_interval * CLOCK_SECOND);
    		    leds_off(LEDS_BLUE);
    		}
    		else{
    		    printf("[PROCESS_sync_broadcasting] duty finished.\n\r");
    		    process_post(&head_change, PROCESS_EVENT_MSG, 0);
    		}
    	}
    }
    PROCESS_END();
}

// process that report the data to gateway
PROCESS_THREAD(report_to_gateway, ev, data){
	static uint8_t report_delay_interval = GATEWAY_INTERVAL; // @suppress("Type cannot be resolved")

	PROCESS_EXITHANDLER(unicast_close(&unicast);)

    static struct etimer delay;

	PROCESS_BEGIN();
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_TO_GATEWAY);

	unicast_open(&unicast, UNICAST_GATEWAY_CHANNEL, &unicast_call);

	while(1){
		PROCESS_WAIT_EVENT();
		linkaddr_t addr;
		if(ev == PROCESS_EVENT_MSG){
			etimer_set(&delay, report_delay_interval * CLOCK_SECOND);
		}
		else if (ev == PROCESS_EVENT_TIMER){
			if (etimer_expired(&delay) && report_end_sync){
				leds_on(LEDS_RED);
				addr.u8[0] = (gateway_id >> 8) & 0xFF;
				addr.u8[1] = gateway_id & 0xFF;

				Prepare_Gate_Report(&tx_gateway_report);

				for (int i=0; i<6; i++){
				    printf("%d ", tx_gateway_report.node_list[i]);
				}
				printf(" \n\r");
				for (int i=0; i<6; i++){
				    printf("%d ", tx_gateway_report.node_bool[i]);
				}
				printf(" \n\r");
				for (int i=0; i<6; i++){
				    printf("%d ", tx_gateway_report.node_status[i]);
				}
				printf(" \n\r");

				packetbuf_copyfrom(&tx_gateway_report, 100);
				unicast_send(&unicast, &addr);
				printf("[PROCESS_report_to_gateway] Send to Gateway.\n\r");

				report_end_sync = 0;
				Reset_Node_Status();

				leds_off(LEDS_RED);
		    }
		}
	}

	PROCESS_END();
}

// process used to switch the cluster head
PROCESS_THREAD(head_change, ev, data){

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();

	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_REGULAR);

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

				if (head_id != node_id){
					tx_contacts.nodeid = head_id;
				    printf("[PROCESS_head_change] H_Node %d:, Change head to %d\n\r", node_id, tx_contacts.nodeid);
				    packetbuf_copyfrom(&tx_contacts ,100);
				    broadcast_send(&broadcast);

				    process_post(&failure_countdown, PROCESS_EVENT_MSG, 0);
				    Reset_Topo();
				}
				else {
					Reset_Duty(0);
					process_post(&sync_broadcasting, PROCESS_EVENT_MSG, 0);
				}

				leds_off(LEDS_YELLOW);
			}
		}
	}

	PROCESS_END();
}

// used by cm to count down the time of last received sync,
// if it is too long, consider the ch is failed, return itself to free node and run initial process.
PROCESS_THREAD(failure_countdown, ev, data){
	static uint8_t ch_failue_count = BROADCAST_INTERVAL;

	PROCESS_EXITHANDLER( printf("[PROCESS_failure_countdown] Start the node! \n\r");)
	PROCESS_BEGIN();
	static struct etimer failure_count;

	while(1) {
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_MSG){
			etimer_set(&failure_count, 2 * ch_failue_count * CLOCK_SECOND);
		}
		else if (ev == PROCESS_EVENT_TIMER){
			if (etimer_expired(&failure_count) && (Node_Is_Free() == 0)){
				if (Node_Is_Free() || is_cluster_head()){
					printf("[PROCESS_failure_countdown] Do nothing.\n\r");
				}
				else{
					Set_Free(1);
				    head_id = 255;
				    printf("[PROCESS_failure_countdown] Failure Detected.\n\r");
				    process_post(&node_initial, PROCESS_EVENT_MSG, 0);
				}
			}
		}
	}

	PROCESS_END();
}

// process used for free node to self-organized network
PROCESS_THREAD(node_initial, ev, data){

	PROCESS_EXITHANDLER( printf("[PROCESS_node_initial] Join or construct the network. \n\r");)
	PROCESS_BEGIN();

	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_INITIAL);

	static uint8_t overhead = 4;
	static struct etimer delay;

	while(1){
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_MSG){
			etimer_set(&delay, 0.5 * node_id * overhead * CLOCK_SECOND);
		    leds_on(LEDS_GREEN);
		}
		else if (ev == PROCESS_EVENT_TIMER){
			leds_off(LEDS_GREEN);
			if (etimer_expired(&delay) && Node_Is_Free()){
				printf("[PROCESS_node_initial] Not Received Any Syncing Frame, Set self as CH.\n\r");
				head_id = node_id;
				Set_Free(0);
				Reset_Duty(0);
				Reset_Topo();
				process_post(&sync_broadcasting, PROCESS_EVENT_MSG, 0);
			}
		}
	}
	PROCESS_END();
}

// Process used to start the initial process
PROCESS_THREAD(start_node_initial, ev, data){

	node_id = (linkaddr_node_addr.u8[1] & 0xFF);
	Setup_Topo(node_id);

	PROCESS_EXITHANDLER( printf("[PROCESS_node_initial] Start the node! \n\r");)
	PROCESS_BEGIN();
	printf("[PROCESS_node_initial] Process Start! \n\r");

	process_post(&node_initial, PROCESS_EVENT_MSG, 0);

	PROCESS_END();
}

// When button pressed, send directly to gateway for help
PROCESS_THREAD(really_emergency, ev, data){

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();
	button_sensor.configure(BUTTON_SENSOR_CONFIG_TYPE_INTERVAL, CLOCK_SECOND*2);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL);
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, POWER_TO_GATEWAY);

	broadcast_open(&broadcast, BROADCAST_EMERGENCY_CHANNEL, &syncing_call);

	static syncing tx_emergency;
	strcpy(tx_emergency.type, "Emergency");
	tx_emergency.nodeid = node_id;

	while(1) {
	    PROCESS_WAIT_EVENT();
	    if (ev == sensors_event) {
	        if (data == &button_sensor) {
	        	if (button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) == BUTTON_SENSOR_PRESSED_LEVEL) {
	                leds_on(LEDS_RED);
	            }
	            else if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) == BUTTON_SENSOR_RELEASED_LEVEL) {
	        	    leds_off(LEDS_ALL);
	                }
	            }
	        }
	    else if(ev == button_press_duration_exceeded){
	        leds_on(LEDS_RED);

	        packetbuf_copyfrom(&tx_emergency ,20);
	        broadcast_send(&broadcast);
	        printf("[PROCESS_really_emergency] Emergency Node: %d, with type: %s.\n\r", tx_emergency.nodeid, tx_emergency.type);
	    }
	}

	PROCESS_END();
}
















