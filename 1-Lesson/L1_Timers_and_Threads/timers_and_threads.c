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

   LESSON 1: Timers and Threads
*/


// Contiki-specific includes:
#include "contiki.h"
#include "dev/leds.h"			// Enables use of LEDs


// Standard C includes:
#include <stdio.h>		// For printf


PROCESS(timers_and_threads_process, "Lesson 1: Timers and Threads");
PROCESS(blue_blink, "Blue Blink");
AUTOSTART_PROCESSES(&timers_and_threads_process, &blue_blink);

//------------------------ PROCESS' THREAD ------------------------

// Main process:
PROCESS_THREAD(timers_and_threads_process, ev, data) {


	PROCESS_EXITHANDLER( printf("main_process terminated!\n"); )

    PROCESS_BEGIN();

	static struct etimer freq_timer;
	static struct timer stop_timer;  /*Exercise 3.3.2 Ex5 stop blink*/
	etimer_set(&freq_timer, CLOCK_SECOND);
	timer_set(&stop_timer, CLOCK_SECOND * 5);


    while (1){
    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&freq_timer));
    		 /* If timer expired, toggle LED*/
		leds_toggle(LEDS_RED);
			/* Reset Timer */
		etimer_reset(&freq_timer);
		if (timer_expired(&stop_timer)){
			PROCESS_EXIT();
		}
    }

    PROCESS_END();
}

PROCESS_THREAD(blue_blink, ev, data){
    /*Exercise 3.3.2 Ex4 blue blink*/
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
