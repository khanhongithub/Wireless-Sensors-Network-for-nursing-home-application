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
PROCESS(multithreading_proccess, "Lesson 1: Multithreading");
AUTOSTART_PROCESSES(&multithreading_proccess);

//------------------------ PROCESS' THREAD ------------------------
PROCESS_THREAD(multithreading_proccess, ev, data){

	static struct etimer timerRed, timerGreen, timerBlue;

	PROCESS_BEGIN();

	printf("Timers set!\r\n ");
	/*
	 * Set timers
	 */
	etimer_set(&timerRed, CLOCK_SECOND);
	etimer_set(&timerGreen, CLOCK_SECOND/2);
	etimer_set(&timerBlue, CLOCK_SECOND/4);

	while(1) {
		PROCESS_WAIT_EVENT();
		if(etimer_expired(&timerRed)) {
			printf("Timer expired for RED...\r\n");
			leds_toggle(LEDS_RED);
			etimer_reset(&timerRed);
		}
		else if(etimer_expired(&timerGreen)) {
			printf("Timer expired for GREEN...\r\n");
			leds_toggle(LEDS_GREEN);
			etimer_reset(&timerGreen);
		}
		else if(etimer_expired(&timerBlue)) {
			printf("Timer expired for BLUE...\r\n");
			leds_toggle(LEDS_BLUE);
			etimer_reset(&timerBlue);
		}
	}
	PROCESS_END();
}



