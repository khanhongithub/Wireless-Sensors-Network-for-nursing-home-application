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


//function for outputting the lux value read from sensor
    		//@param m: calibration value m inscribed on the back of the sensor
    		//@param b: calibration value b inscribed on the back of the sensor
    		//@param adc_input: phidget input value. Use ZOUL_SENSORS_ADC1 or ZOUL_SENSORS_ADC3 depending on where the sensor is connected to.
    		//@return int : lux value with a max of 1000.
static int getLightSensorValue(float m, float b, uint16_t adc_input){
    		//Convert the voltage in lux with the provided formula
    		int luminosity = m*(adc_input/4.096)+b;
    		//Return the value of the light with maximum value equal to 1000
    		if (luminosity<1)
			return 1;
    		else if (luminosity>1000)
    			return 1000;
    		else
    			return luminosity;
}

int windspeed_buffer[20];
int i = 0;
int n = 0;
int wind_speed;

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
	//static uint16_t adc1_value;

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
	adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC1);


	etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);

	while (1) {

		PROCESS_WAIT_EVENT();  // let process continue

		/* If timer expired, pront sensor readings */
	    if(ev == PROCESS_EVENT_TIMER) {

	    	leds_on(LEDS_PURPLE);

	    	/*
	    	 * Read ADC values. Data is in the 12 MSBs
	    	 */
	    	windspeed_buffer[i] = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;

	    	if (i>=19){
	    		for (int j=0;j<19;j++)
	    		{
	    			if (windspeed_buffer[j+1]-windspeed_buffer[j]>1500){
	    				n++;
	    			}
	    		}
	    		wind_speed = n/0.6;
	    		printf("\r\nThe wind speed = %d km/h", wind_speed);
                i = 0;
                n = 0;
	    		}

	    	else{
	            i++;
	    	}

	    	/*
	    	 * Print Raw values
	    	 */

	    	//printf("\r\nADC1 value [Raw] = %d", windspeed_buffer[1]);


    		leds_off(LEDS_PURPLE);


    		etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	    }
    }

	PROCESS_END ();
}
