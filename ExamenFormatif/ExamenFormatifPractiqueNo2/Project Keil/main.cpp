#include "mbed.h"
#include "rtos.h"

DigitalOut led1(LED1);
int j = 0;

void led_thread()
{
	int i =0;
	while(1)
	{
		Thread::signal_wait(0x1);
		i++;
		j++;
		led1 = !led1;
	}
}

int main()
{
	Thread thread;
	thread.start(led_thread);
	
	while(1)
	{
		Thread::wait(1000);
		thread.signal_set(0x1);
	}
}