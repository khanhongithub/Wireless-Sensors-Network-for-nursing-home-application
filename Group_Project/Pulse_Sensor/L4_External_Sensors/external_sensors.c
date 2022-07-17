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
#include "math.h"

// Reading frequency in seconds.
#define TEMP_READ_INTERVAL CLOCK_SECOND*0.05
#define WAITING_INTERVAL CLOCK_SECOND*1


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


static int FindBPM(int a[], int n, int *pMaxPos)
   {
	 int i,max;
	 max = a[0];
	 *pMaxPos = 0;

	 for (i=0; i<n; i++)
	 {
		 if (a[i]>max){
			 max = a[i];
			 *pMaxPos = i;
		 }
	 }
	 return max;
   }


static double getVarianceValue(double adc_input[]){
	double sum = 0;
	int length = 0;
	double average = 0;
	double var = 0;
    		for (int i = 0; i <=39; i++)
    		{
    			sum += adc_input[i];
    		}

    		length = 40;
    		average = sum/length;

    		for (int j = 0; j <= 39; j++)
    		{
    			var += pow(adc_input[j]-average,2)/length;
    		}

    			return var;
}



double pulse_buffer[40];
int i = 0;


/**
 * Connection information
 */
static struct broadcast_conn broadcastConn;

/**
 * Assign callback functions to the connection
 */
static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv};

/*** CONNECTION DEFINITION END ***/

//--------------------- PROCESS CONTROL BLOCK ---------------------
PROCESS (ext_sensors_process, "External Sensors process");
AUTOSTART_PROCESSES (&ext_sensors_process);


//------------------------ PROCESS' THREAD ------------------------


PROCESS_THREAD (ext_sensors_process, ev, data) {

	/* variables to be used */
	static struct etimer temp_reading_timer;
	static struct etimer waiting_timer;



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
	broadcast_open(&broadcastConn,129,&broadcast_callbacks);


	/* Configure the ADC ports */
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1 | ZOUL_SENSORS_ADC3);


	etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	//etimer_set(&waiting_timer, WAITING_INTERVAL);


	while (1) {

		PROCESS_WAIT_EVENT();  // let process continue

		/* If timer expired, pront sensor readings */
	    if(ev == PROCESS_EVENT_TIMER) {

	    	leds_on(LEDS_PURPLE);

	    	/*
	    	 * Read ADC values. Data is in the 12 MSBs
	    	 */
	    	pulse_buffer[i] = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;
	    	printf("\r\nHeart beat: %d", pulse_buffer[i]);

	    	/*
	    	 * Print Raw values
	    	 */

	        //printf("\r\nADC1 value [Raw] = %d", adc_value);
            i++;

            if (i >= 39){
            int var = getVarianceValue(pulse_buffer);

            if (var > 35000)
            {
            	printf("\r\nHeart beat detected!");

            }

            printf("\r\nVariance value = %d", var);
			//etimer_set(&waiting_timer, WAITING_INTERVAL);
			i = 0;
            }

    		leds_off(LEDS_PURPLE);


    		etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	    }
    }

	PROCESS_END ();
}

