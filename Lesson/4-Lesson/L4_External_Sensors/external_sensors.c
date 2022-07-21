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

// Reading frequency in seconds.
#define TEMP_READ_INTERVAL CLOCK_SECOND*1
#define JOYSTICK_READ_INTERVAL CLOCK_SECOND*0.5

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
PROCESS (ext_joystick_process, "External Joystick Sensors process");
AUTOSTART_PROCESSES (&ext_sensors_process,&ext_joystick_process);


//------------------------ PROCESS' THREAD ------------------------

PROCESS_THREAD(ext_joystick_process, ev, data) {
	static struct etimer joystick_reading_timer;
	static uint16_t x_value, y_value;

	PROCESS_BEGIN ();

	printf("\r\nZolertia RE-Mote external Joystick sensors");
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


		etimer_set(&joystick_reading_timer, JOYSTICK_READ_INTERVAL);
		while (1) {

				PROCESS_WAIT_EVENT();  // let process continue

				/* If timer expired, pront sensor readings */
			    if(ev == PROCESS_EVENT_TIMER) {

			    	leds_on(LEDS_RED);

			    	/*
			    	 * Read ADC values. Data is in the 12 MSBs
			    	 */
			    	x_value = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;
			    	y_value = adc_zoul.value(ZOUL_SENSORS_ADC3) >> 4;

			    	/*
			    	 * Print Raw values
			    	 */

			    	//printf("\r\nx_value [Raw] = %d", x_value);
			        //printf("\r\ny_value [Raw] = %d", y_value);

			        if ((x_value>=1532)&&(x_value<=1580)&&(y_value>=800)&&(y_value<=840))
			        {
			        	printf("\r\nCenter");
			        }
			        else if (x_value<100)
			        {
			        	printf("\r\nLeft");
			        }
			        else if (x_value>2040)
			        {
			        	printf("\r\nRight");
			        }
			        else if (y_value>1600)
			        {
			        	printf("\r\nUp");
			        }
			        else if (y_value<20)
			        {
			       		printf("\r\nDown");
			       	}
			        else
			        {
			        	printf("\r\nUnknown");
			        }

		    		leds_off(LEDS_RED);

		    		etimer_set(&joystick_reading_timer, JOYSTICK_READ_INTERVAL);
			    }
		    }
		PROCESS_END ();

}





PROCESS_THREAD (ext_sensors_process, ev, data) {

	/* variables to be used */
	static struct etimer temp_reading_timer;
	static uint16_t adc1_value, adc3_value;

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

	while (1) {

		PROCESS_WAIT_EVENT();  // let process continue

		/* If timer expired, pront sensor readings */
	    if(ev == PROCESS_EVENT_TIMER) {

	    	leds_on(LEDS_PURPLE);

	    	/*
	    	 * Read ADC values. Data is in the 12 MSBs
	    	 */
	    	//adc1_value = adc_zoul.value(ZOUL_SENSORS_ADC1) >> 4;
	    	//adc3_value = adc_zoul.value(ZOUL_SENSORS_ADC3) >> 4;

	    	/*
	    	 * Print Raw values
	    	 */

	    	//printf("\r\nADC1 value [Raw] = %d", adc1_value);
	        //printf("\r\nADC3 value [Raw] = %d", adc3_value);


    		leds_off(LEDS_PURPLE);

    		int luminosity = getLightSensorValue(1.3691, 39.716, adc1_value);
    		printf("\r\nADC1 value [Reverted] = %d\n", luminosity);

    		etimer_set(&temp_reading_timer, TEMP_READ_INTERVAL);
	    }
    }

	PROCESS_END ();
}

