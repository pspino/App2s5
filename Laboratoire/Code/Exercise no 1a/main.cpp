#include "mbed.h"
#include "rtos.h"

DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut sig1(p21);
DigitalOut sig2(p22);
DigitalOut sig3(p23);

void thread_LED2(void)
{
   while(1) {
      myled2=!myled2;
	  	sig2 = !sig2;
    }
}

void thread_LED3(void)
{
   while(1) {
      myled3=!myled3;
	  	sig3 = !sig3;
		osDelay(100);
	 }
}


int main()
{
   Thread thread2;
   Thread thread3;

	thread2.start(thread_LED2); 
  //thread2.set_priority(osPriorityAboveNormal);
	thread3.start(thread_LED3); 
	thread3.set_priority(osPriorityAboveNormal);

   while(1) {
      myled1 = !myled1;;
	   sig1 = !sig1;
    }
}
