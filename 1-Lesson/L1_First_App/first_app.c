#include "contiki.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"


PROCESS(first_process, "Main process of the first WSN lab application");

AUTOSTART_PROCESSES(&first_process);

PROCESS_THREAD(first_process, ev, data){
    PROCESS_BEGIN();
    leds_off(LEDS_ALL);
    button_sensor.configure(BUTTON_SENSOR_CONFIG_TYPE_INTERVAL, CLOCK_SECOND*2);
    char* redon = "red.on";
    char* redoff = "red.off";
    char* greenon = "green.on";
    char* greenoff = "green.off";
    while(1) {
        PROCESS_WAIT_EVENT();

        if(ev == sensors_event) {
        	if(data == &button_sensor) {
        		if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
                   BUTTON_SENSOR_PRESSED_LEVEL) {
                leds_on(LEDS_RED);
                }
                else if(button_sensor.value(BUTTON_SENSOR_VALUE_TYPE_LEVEL) ==
        		    BUTTON_SENSOR_RELEASED_LEVEL) {
        	    leds_off(LEDS_ALL);
                }
            }
        }
        else if(ev == button_press_duration_exceeded){
           leds_toggle(LEDS_GREEN);
           leds_toggle(LEDS_RED);
        }

        else if(ev == serial_line_event_message){
        	if(strcmp(data, redon) == 0){
        		leds_on(LEDS_RED);
        	}
        	else if(strcmp(data, redoff) == 0){
        	    leds_off(LEDS_RED);
        	}
        	else if(strcmp(data, greenon) == 0){
        	    leds_on(LEDS_GREEN);
        	        	}
        	else if(strcmp(data, greenoff) == 0){
        	    leds_off(LEDS_GREEN);
        	}
        }
    }
  PROCESS_END();
}




