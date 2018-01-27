#include "mbed.h"
#include "rtos.h"

DigitalIn in1(p13);
DigitalIn in2(p14);
AnalogIn an1(p15);
AnalogIn an2(p16);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

void numericRead()
{
	while(true)
	{
		if(in1 == 1)
		{
			led2 = 1;
		}
		else
		{
			led2 = 0;
		}
	}
}

void analogRead() 
{
	while (true) 
	{
		led3 = 1;
		Thread::wait(50);
		led3 = 0;
		Thread::wait(50);
	}
}

void collection()
{
	while(true)
	{
		wait(1);
	}
}

int main()
{
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread numReadThread;
	numReadThread.start(numericRead);
	numReadThread.set_priority(osPriorityHigh);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		led4 = 1;
		wait(1);
		led4 = 0;
		wait(1);
	}
}