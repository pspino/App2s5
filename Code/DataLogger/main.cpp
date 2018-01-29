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

void ping()
{
	uint8_t bit0 = 0;
	uint8_t bit1 = 0;
	bool keepValue = false;
	while (true) 
	{
		int count1 = 0;
		int count2 = 0;
		while(count1 < 10 || count2 < 10)
		{	
			uint8_t currentReading1 = in1 == 1 ? 1 : 0;
			uint8_t currentReading2 = in2 == 1 ? 1 : 0;
			
			currentReading2 = currentReading2 << 1;
			if(currentReading1 != bit0)
			{
				keepValue = true;
			}
			if(currentReading2 != bit1)
			{
				keepValue = true;
			}
			uint8_t pins = (bit0 | bit1);
			count1++;
			count2++;
			Thread::wait(5);
		}
	}
}

void numericSend()
{
	while(1)
	{
		//Sending part, must be in 100ms rate
		fifoMutex.lock();
		Event numEvent = {pins,0x0000,ms};
		fifo.put(&numEvent,ms);
		fifoMutex.unlock();
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
		int pins[4] = {0,0,0,0};
		osEvent fifoEvent = fifo.get();
		if(fifoEvent.status == osEventMessage)
		{
				Event* event = (Event*)fifoEvent.value.p;
				int upPins = event->pins;
				int step = 1000;
				while(upPins != 0)
				{
						int remainder = upPins%2;
						upPins /= 2;
						pins[step%10] = remainder*step;
						step /= 10;
				}
		}
		led4 = pins[1];
		led3 = pins[0];
		fifoMutex.unlock();
		Thread::wait(50);
	}
}
