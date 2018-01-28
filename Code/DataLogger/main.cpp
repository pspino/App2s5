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

Queue<Event,50> fifo;
Mutex fifoMutex;

typedef struct {
   uint16_t    value;
   time_t    timeStamp;
} Event;

void numericRead()
{
	while (true) 
	{
		led3 = in1;
		led4 = in2;
		unsigned int bit0 = in1 == 1 ? 1 : 0;
		unsigned int bit1 = in2 == 1 ? 1 : 0;
		bit1 = bit1 << 1;
		unsigned int message = bit0 | bit1;
		time_t ms = time(NULL);
		fifoMutex.lock();
		Event numEvent = {message,ms};
		fifo.put(numEvent,ms);
		fifoMutex.unlock();
		Thread::wait(100);
	}
}

void analogRead() 
{
	while (true) 
	{
		button = an1;
		potentiometre = an2;
		time_t ms = time(NULL);
		fifoMutex.lock();
		Event analogEvent = {button.read_u16(),ms};
		fifo.put(numEvent,ms);
		fifoMutex.unlock();
		Thread::wait(100);
	}
}

int main()
{	
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread numReadThread;
	numReadThread.start(numericRead);
	numReadThread.set_priority(osPriorityAboveNormal);
	
	Thread analogReadThread;
	analogReadThread.start(analogRead);
	analogReadThread.set_priority(osPriorityNormal);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		fifoMutex.lock();
		osEvent fifoEvent = fifo.get();
		if(fifoEvent.status == osEventMessage)
		{
			unsigned int* prtValue = (unsigned int*)fifoEvent.value.v;
			int bin[2] = {0,0};
			int decValue = *prtValue;
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
