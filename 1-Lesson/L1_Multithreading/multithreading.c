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

   LESSON 1: Multithreading
*/

// Contiki-specific includes:
#include "contiki.h"
#include "dev/leds.h" // Enables use of LEDs.

// Standard C includes:
#include <stdio.h>    // For printf.


//--------------------- PROCESS CONTROL BLOCK ---------------------
// Exercise 3.3.3 Ex 5 Rewrite code
PROCESS(multithreading_proccess, "Lesson 1: Multithreading");
PROCESS(blue_blink, "blue blink");
PROCESS(green_blink, "green blink");
AUTOSTART_PROCESSES(&multithreading_proccess, &blue_blink, &green_blink);

//------------------------ PROCESS' THREAD ------------------------
PROCESS_THREAD(multithreading_proccess, ev, data){

	static struct etimer timerRed;

	PROCESS_BEGIN();

	printf("Timers set!\r\n ");
	/*
	 * Set timers
	 */
	etimer_set(&timerRed, CLOCK_SECOND);

	while(1) {
		PROCESS_WAIT_EVENT();
		if(etimer_expired(&timerRed)) {
			printf("Timer expired for RED...\r\n");
			leds_toggle(LEDS_RED);
			etimer_reset(&timerRed);
		}
	}
	PROCESS_END();
}

PROCESS_THREAD(green_blink, ev, data){
	PROCESS_EXITHANDLER( printf("green blink terminated!\n"); )
	PROCESS_BEGIN();

	static struct etimer freq_timer;
	etimer_set(&freq_timer, 0.5 * CLOCK_SECOND);

	while (1){
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&freq_timer));
	    	/* If timer expired, toggle LED*/
		leds_toggle(LEDS_BLUE);
			/* Reset Timer */
		etimer_reset(&freq_timer);
	}

	PROCESS_END();
}

PROCESS_THREAD(blue_blink, ev, data){
	PROCESS_EXITHANDLER( printf("blue blink terminated!\n"); )
	PROCESS_BEGIN();

	static struct etimer freq_timer;
	etimer_set(&freq_timer, 0.25 * CLOCK_SECOND);

	while (1){
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&freq_timer));
	    	/* If timer expired, toggle LED*/
		leds_toggle(LEDS_BLUE);
			/* Reset Timer */
		etimer_reset(&freq_timer);
	}

	PROCESS_END();
}

