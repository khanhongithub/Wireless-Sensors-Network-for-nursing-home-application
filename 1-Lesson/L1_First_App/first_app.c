#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

PROCESS(first_process, "Main process of the first WSN lab application");

AUTOSTART_PROCESSES(&first_process);

PROCESS_THREAD(first_process, ev, data)
{
  PROCESS_BEGIN();
  leds_off(LEDS_ALL);
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event) {
         if(data == &button_sensor) {
           if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
               BUTTON_SENSOR_PRESSED_LEVEL) {
             leds_on(LEDS_RED);
           }
           else if (button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
        		   BUTTON_SENSOR_RELEASED_LEVEL){
        	 leds_off(LEDS_RED);
           }
    } }
}
  PROCESS_END();
}
