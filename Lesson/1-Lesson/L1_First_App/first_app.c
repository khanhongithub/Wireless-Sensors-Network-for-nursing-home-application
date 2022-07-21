#include "contiki.h"
#include "net/rime/rime.h"	// Establish connections.
#include "net/netstack.h"
#include "lib/random.h"
#include "dev/leds.h"


static uint8_t node_id = 1;

static uint8_t node_list[6] = {1, 2, 3, 4, 5, 6};
static int node_bool[6] = {1, 1, 1, 0, 0, 0};
static int ndoe_leaved[6] = {0, 0, 0, 1, 0, 0};
int is_event = 1;
static uint8_t event_sta[6] = {1, 0, 0, 0, 0, 0};
static uint8_t value[6]= {100, 10, 20, 0, 0, 0};

typedef struct{
	uint8_t node_list[6];
	int node_in[6];
	int node_leave[6];
	int event;
	uint8_t event_static[6];
	uint8_t value[6];
}collection;

static collection tx_contacts;


static struct broadcast_conn broadcast;
PROCESS(sync_broadcasting, "Sync frame broadcasting of the sensor code.");
AUTOSTART_PROCESSES(&sync_broadcasting);

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
}

static const struct broadcast_callbacks syncing_call = {syncing_recv};

PROCESS_THREAD(sync_broadcasting, ev, data) {

	static uint8_t broadcast_time_interval = 3;
	static struct etimer sync_timer;

	PROCESS_EXITHANDLER(broadcast_close(&broadcast));
	PROCESS_BEGIN();

    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL, 14);
    NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_TXPOWER, 3);

    etimer_set(&sync_timer, broadcast_time_interval * CLOCK_SECOND);
    broadcast_open(&broadcast, 119, &syncing_call);

    while(1) {
    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sync_timer));
        leds_on(LEDS_BLUE);
    	//fill the packet buffer
        for (int i=0; i<6; i++){
        	tx_contacts.node_list[i] = node_list[i];
            tx_contacts.node_in[i] = node_bool[i];
            tx_contacts.node_leave[i] = ndoe_leaved[i];

            tx_contacts.event_static[i] = event_sta[i];
            tx_contacts.value[i] = value[i];
        }
        tx_contacts.event = is_event;

        packetbuf_copyfrom(&tx_contacts ,100);
        broadcast_send(&broadcast);

        printf("%d \n\r", tx_contacts);

        printf("Sync message sent in channel %d with power: %d\r\n", 14, 3);
        etimer_reset(&sync_timer);
        leds_off(LEDS_BLUE);



        // add more function to loop the process it self.
    }
    PROCESS_END();
}




