#include "mbed.h"
#include "rtos.h"

DigitalOut LEDs[4] = {
    DigitalOut(LED1), DigitalOut(LED2), DigitalOut(LED3), DigitalOut(LED4)
};

void blink(void const *n) {
    LEDs[(int)n] = !LEDs[(int)n];
}

int main(void) {
    RtosTimer led_1_timer(callback(blink, (void *)0), osTimerPeriodic); 
    RtosTimer led_2_timer(callback(blink, (void *)1), osTimerPeriodic);
    RtosTimer led_3_timer(callback(blink, (void *)2), osTimerPeriodic);
    RtosTimer led_4_timer(callback(blink, (void *)3), osTimerPeriodic);
    
    led_1_timer.start(2000);
    led_2_timer.start(1000);
    led_3_timer.start(500);
    led_4_timer.start(250);
	
    Thread::wait(osWaitForever);
}