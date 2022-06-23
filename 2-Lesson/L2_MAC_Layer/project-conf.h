#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_


// PHY LAYER PARAMETERS
#define CHANNEL 14
#define TX_POWER 3


// MAC LAYER PARAMETERS
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 128


// MEASUREMENT PARAMETERS
#define INTER_PACKET_TIME	2		//  Inter-Packet Arrival Time 128 Ticks = 1 second.
#define MAX_MESSAGE_LENGTH 	12		//  Message length (values btw 8-100)
#define TOTAL_TX_PACKETS	100		//  Number of total transmit packets.


#endif /* PROJECT_CONF_H_ */
