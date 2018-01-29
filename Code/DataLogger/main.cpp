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

typedef struct {
   uint16_t    value;
   time_t    timeStamp;
}Event;

Queue<Event,50> fifo;
Mutex fifoMutex;



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
		fifo.put(&numEvent,ms);
		fifoMutex.unlock();
		Thread::wait(100);
	}
}

void analogRead() 
{
	uint16_t currentMean =0;
	while (true) 
	{
		time_t ms = time(NULL);
		fifoMutex.lock();
		uint16_t analogMean =0;
		for(uint8_t index =0;index<5;index++)
		{
			analogMean +=an1.read_u16();
			Thread::wait(50);
		}
		analogMean/=5;
		if(analogMean > (currentMean*1.125) || analogMean < (currentMean*0.875))
		{
			currentMean =analogMean;
		}
		Event analogEvent = {currentMean,ms};
		fifo.put(&analogEvent,ms);
		fifoMutex.unlock();
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
