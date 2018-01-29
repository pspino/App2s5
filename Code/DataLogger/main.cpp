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

typedef struct 
{
	uint8_t				pins; 				//7:4 reserved -> 3:0 pins.
	uint32_t    	value;
  time_t    		timeStamp;
}Event;

Queue<Event,500> fifo;
Mutex fifoMutex;



void numericRead()
{
	while (true) 
	{
		time_t ms = time(NULL);
		led3 = in1;
		led4 = in2;
		unsigned int bit0 = in1 == 1 ? 1 : 0;
		unsigned int bit1 = in2 == 1 ? 1 : 0;
		bit1 = bit1 << 1;
		int pins = (bit0 | bit1);
		
		//Sending part, must be in 100ms rate
		fifoMutex.lock();
		Event numEvent = {pins,0x0000,ms};
		fifo.put(&numEvent,ms);
		fifoMutex.unlock();
		
		Thread::wait(100);
	}
}

void analogRead() 
{
	uint16_t an1CurrentMean =0;
	uint16_t an2CurrentMean =0;
	while (true) 
	{
		fifoMutex.lock();
		time_t ms = time(NULL);
		uint16_t an1Mean =0;
		uint16_t an2Mean =0;
		uint8_t analogPins =0;
		for(uint8_t index =0;index<5;index++)
		{
			an1Mean +=an1.read_u16();
			an2Mean +=an2.read_u16();
			Thread::wait(50);
		}
		an1Mean/=5;
		an2Mean/=5;
		if(an1Mean > (an1CurrentMean*1.125) || an1Mean < (an1CurrentMean*0.875))
		{
			analogPins = 0b0100;
			an1CurrentMean =an1Mean;
		}
		if(an2Mean > (an2CurrentMean*1.125) || an2Mean < (an2CurrentMean*0.875))
		{
			analogPins |= 0b1000;
			an2CurrentMean =an2Mean;			
		}
		if(analogPins!=0)
		{
			uint32_t analogValue = an1CurrentMean | an2CurrentMean << 16;
			Event analogEvent = {analogPins,analogValue,ms};
			fifo.put(&analogEvent,ms);
		}
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
			if(fifoEvent.status == osEventMessage)
			{
				Event* event = (Event*)fifoEvent.value.p;
				printf("%d",event->pins);
			}
//			unsigned int* prtValue = (unsigned int*)fifoEvent.value.v;
//			int bin[2] = {0,0};
//			int decValue = *prtValue;
//			int step = 10;
//			while(decValue !=0)
//      {
//					int remainder = decValue%2;
//					decValue /= 2;
//					bin[step%10] = remainder*step;
//					step /= 10;
//			}
//			led1 = bin[0];
//			led2 = bin[1];
		}
		fifoMutex.unlock();
		Thread::wait(50);
	}
}
