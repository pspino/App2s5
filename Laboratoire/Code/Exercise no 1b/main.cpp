#include "mbed.h"
 
Ticker flipper;
DigitalOut led1(LED1);
DigitalOut led2(LED2);
 
void flip() {
    led2 = !led2;
}
 
int main() {
    led2 = 1;
    flipper.attach(&flip, 2.0); // flip with 2 sec. interval
 
    while(1) {
        led1 = !led1;
        wait(0.2);
    }
}

