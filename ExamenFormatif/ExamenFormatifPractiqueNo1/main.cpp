#include "mbed.h"
#include "rtos.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

Queue<int,1> fifo;
osThreadId t1Id;
osThreadId t2Id;
osThreadId t3Id;

void thread1()
{
	t1Id = osThreadGetId();
	while(1)
	{
		Thread::signal_wait(0x03);
		printf("recieved signal 0x03\n");
		led4 = !led4;
	}
}

void thread2()
{
	t2Id = osThreadGetId();
	while(1)
	{
		Thread::signal_wait(0x01);
		printf("recieved signal 0x01\n");
		int value = rand() %100;
		fifo.put(&value);
	}
}

void thread3()
{
	t3Id = osThreadGetId();
	while(1)
	{
		osEvent event = fifo.get();
		if(event.status == osEventMessage)
		{
			int* value = (int*)event.value.v;
			printf("%d was in the queue\n", *value);
			led3 = !led3;
			osSignalSet(t1Id,0x03);
		}
	}
}

void ticker1()
{
	osSignalSet(t2Id,0x01);
}

int main()
{
	Ticker ticker;
	ticker.attach(&ticker1,1.0);
	
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread t1;
	t1.start(thread1);
	t1.set_priority(osPriorityAboveNormal);
	
	Thread t2;
	t2.start(thread2);
	t2.set_priority(osPriorityAboveNormal);
	
	
	Thread t3;
	t3.start(thread3);
	t3.set_priority(osPriorityAboveNormal);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	
	while(1)
	{
		led1 = 0;
		Thread::wait(2000);
		led1 = 1;
		Thread::wait(2000);
	}
}