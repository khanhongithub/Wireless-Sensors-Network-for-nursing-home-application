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
#include "lib/sensors.h"
#include "net/rime/rime.h"     // Establish connections.
#include "net/netstack.h"      // Wireless-stack definitions
#include "dev/leds.h"          // Use LEDs.
#include "dev/button-sensor.h" // User Button
#include "dev/adc-zoul.h"      // ADC
#include "dev/zoul-sensors.h"  // Sensor functions
#include "dev/sys-ctrl.h"
#include "platform/zoul/dev/weather-meter.h"
#include "dev/gpio.h"
#include "cpu.h"
#include "dev/ioc.h"
#include "sys/timer.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
// Standard C includes:
#include <stdio.h>      // For printf.
#include <stdint.h>

// Reading frequency in seconds.
#define TEMP_READ_INTERVAL CLOCK_SECOND*2
#define ANEMOMETER_THRESHOLD_TICK   13  /**< 16 Km/h */
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
static void
wind_speed_callback(uint16_t value)
{
  /* This checks for instant wind speed values (over a second), the minimum
   * value is 1.2 Km/h (one tick), as the reference is 2.4KM/h per rotation, and
   * the anemometer makes 2 ticks per rotation.  Instant speed is calculated as
   * multiples of this, so if you want to check for 16Km/h, then it would be 13
   * ticks
   */
  printf("*** Wind speed over threshold (%u ticks)\n", value);
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
AUTOSTART_PROCESSES (&ext_sensors_process);

static struct etimer et;
//------------------------ PROCESS' THREAD ------------------------


PROCESS_THREAD (ext_sensors_process, ev, data) {

	/* variables to be used */

	PROCESS_BEGIN ();

	static uint16_t wind_speed;
	static uint16_t wind_dir;
	static uint16_t wind_dir_avg_2m;
	static uint16_t wind_speed_avg;
	static uint16_t wind_speed_avg_2m;
	static uint16_t wind_speed_max;

	printf("Weather meter test example, integration period %u\n",
	         WEATHER_METER_AVG_PERIOD);

	/* Register the callback handler when thresholds are met */
	  WEATHER_METER_REGISTER_ANEMOMETER_INT(wind_speed_callback);

	  /* Enable the sensors, this has to be called before any of the interrupt calls
	     * like the ones below
	     */
	  SENSORS_ACTIVATE(weather_meter);

	  /* And the upper threshold value to compare and generate an interrupt */
	  weather_meter.configure(WEATHER_METER_ANEMOMETER_INT_OVER, ANEMOMETER_THRESHOLD_TICK);

	/*
	 * set your group's channel
	 */
	NETSTACK_CONF_RADIO.set_value(RADIO_PARAM_CHANNEL,14);

	/*
	 * open the connection
	 */
	broadcast_open(&broadcastConn,129,&broadcast_callbacks);


	 etimer_set(&et, TEMP_READ_INTERVAL);

	  while(1) {
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	    wind_speed = weather_meter.value(WEATHER_METER_ANEMOMETER);
	    wind_dir = weather_meter.value(WEATHER_METER_WIND_VANE);
	    wind_dir_avg_2m = weather_meter.value(WEATHER_METER_WIND_VANE_AVG_X);
	    wind_speed_avg = weather_meter.value(WEATHER_METER_ANEMOMETER_AVG);
	    wind_speed_avg_2m = weather_meter.value(WEATHER_METER_ANEMOMETER_AVG_X);
	    wind_speed_max = weather_meter.value(WEATHER_METER_ANEMOMETER_MAX);


	    printf("Wind dir: %u.%01u deg, ", (wind_dir / 10), (wind_dir % 10));
	    printf("(%u.%01u deg avg)\n", (wind_dir_avg_2m / 10), (wind_dir_avg_2m % 10));
	    printf("Wind speed: %u m/h ", wind_speed);
	    printf("(%u m/h avg, %u m/h 2m avg, %u m/h max)\n\n", wind_speed_avg,
	                                                          wind_speed_avg_2m,
	                                                          wind_speed_max);
	    etimer_reset(&et);
	  }


	PROCESS_END ();
}

