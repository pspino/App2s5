#include "mbed.h"
#include "rtos.h"

DigitalOut led(LED1);

void led_thread(void) {
    while (true) {
       Thread::signal_wait(0x1);		
       led = !led;		
    }
}

int main (void) {
    Thread thread;

	  thread.start(callback(led_thread));
    
    while (true) {
        Thread::wait(1000);
        thread.signal_set(0x1);
    }
}
