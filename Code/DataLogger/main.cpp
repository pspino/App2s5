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

Queue<unsigned int,50> fifo;
Mutex fifoMutex;

void numericRead()
{
	while (true) 
	{
		led3 = in1;
		led4 = in2;
		unsigned int message = (unsigned int)in2 << 1 || (unsigned int)in1 << 0;		// in1 == bit 0/ in2 == bit 1
		time_t ms = time(NULL);
		fifoMutex.lock();
		fifo.put(&message,ms);
		fifoMutex.unlock();
		Thread::wait(100);
	}
}

void analogRead() 
{
	
}

int main()
{	
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread numReadThread;
	numReadThread.start(numericRead);
	numReadThread.set_priority(osPriorityAboveNormal);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		fifoMutex.lock();
		osEvent fifoEvent = fifo.get();
		if(fifoEvent.status == osEventMessage)
		{
			unsigned int value = (unsigned int)fifoEvent.value.p;
			led1 = 1;
		}
		fifoMutex.unlock();
		Thread::wait(100);
	}
}