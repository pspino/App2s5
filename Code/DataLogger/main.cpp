#include "mbed.h"
#include "rtos.h"

DigitalIn in1(p21);
DigitalIn in2(p22);
AnalogIn an1(p19);
AnalogIn an2(p20);

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

Queue<Event,50> fifo;
Mutex fifoMutex;

void NumericP1()
{
	uint8_t bit0 = 0;
	uint8_t bit1 = 0;
	uint8_t lastReading0 = 0;
	uint8_t lastReading1 = 0;
	bool newValue = false;
	while (true) 
	{
		fifoMutex.lock();
		int count0 = 0;
		int count1 = 0;
		uint8_t idx = 0;
		uint8_t currentReading0 = 0;
		uint8_t currentReading1 = 0;
		while(idx < 20)
		{
			if(count0 >= 10 && bit0 != currentReading0)
			{
				bit0 = currentReading0;
				newValue = true;
			}
			if(count1 >= 10 && bit1 != currentReading1)
			{
				bit1 = currentReading1;
				newValue = true;
			}
			
			currentReading0 = in1 == 1 ? 1 : 0;
			currentReading1 = in2 == 1 ? 1 : 0;
			
			if(currentReading0 == lastReading0)
				count0++;
			else 
			{
				lastReading0 = currentReading0;
				count0 = 0;
			}
			
			if(currentReading1 == lastReading1)
				count1++;
			else 
			{
				lastReading1 = currentReading1;
				count1 = 0;
			}
			idx++;
			Thread::wait(5);
		}
		if(newValue)
		{
			time_t ms = time(NULL);
			newValue = false;
			uint8_t bit = bit0 | (bit1 << 1);
			Event event = {bit, 0x0000, ms};
			fifo.put(&event);
		}
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
	
	Thread numReadThread1;
	numReadThread1.start(NumericP1);
	numReadThread1.set_priority(osPriorityAboveNormal);
	
	Thread analogReadThread;
	analogReadThread.start(analogRead);
	analogReadThread.set_priority(osPriorityNormal);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		int pins[4] = {0,0,0,0};
		osEvent fifoEvent = fifo.get(5);
		if(fifoEvent.status == osEventMessage)
		{
			fifoMutex.lock();
			Event* event = (Event*)fifoEvent.value.p;
			int upPins = event->pins;
			int step = 0x0;
			while(upPins != 0)
			{
					int remainder = upPins%2;
					upPins /= 2;
					pins[step] = remainder;
					step += 1;
			}
			fifoMutex.unlock();
		}
		led4 = pins[1];
		led3 = pins[0];
		led2 = pins[3];
		led1 = pins[2];
	}
}
