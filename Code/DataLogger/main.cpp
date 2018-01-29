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
		unsigned int bit0 = in1 == 1 ? 1 : 0;
		unsigned int bit1 = in2 == 1 ? 1 : 0;
		bit1 = bit1 << 1;			//This shift is to place the bool value to the respective bit.
		unsigned int message = bit0 | bit1;
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
			//Getting the first value on the queue (FIFO)
			unsigned int* prtValue = (unsigned int*)fifoEvent.value.v;
			int bin[2] = {0,0};
			int decValue = *prtValue;	//Obligatory operation since the returned value of the queue.get is a unsigned int*, better to store it once and then reused it locally.
			int step = 10;
			while(decValue !=0)
      {
					int remainder = decValue%2;
					decValue /= 2;
					bin[step%10] = remainder*step;
					step /= 10;
			}
			led1 = bin[0];
			led2 = bin[1];
		}
		fifoMutex.unlock();
		Thread::wait(100);
	}
}