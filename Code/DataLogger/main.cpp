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

int message[50];
int lastMessage = 0;

void numericRead()
{
	while (true) 
	{
		led3 = in1;
		led4 = in2;
		message[lastMessage] = in2 << 1 && in1;
		lastMessage++;
		Thread::wait(100);
	}
}

void analogRead() 
{
	
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
	memset(message, '\0', 50);
	
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread numReadThread;
	numReadThread.start(numericRead);
	numReadThread.set_priority(osPriorityAboveNormal);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		if(message[0] != '\0')
		{
			if(message[0] == 0b00)
			{
				led1 = 0;
				led2 = 0;
			}
			else if (message[0] == 0b01)
			{
				led1 = 1;
				led2 = 0;
			}
			else if(message[0] == 0b10)
			{
				led1 = 0;
				led2 = 1;
			}
			else if(message[0] == 0b11)
			{
				led1 = 1;
				led2 = 1;
			}
			lastMessage--;
		}
	}
}