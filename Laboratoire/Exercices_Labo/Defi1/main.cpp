#include "mbed.h"
#include "rtos.h"

Semaphore batterie(2); 
Semaphore radio(2); 

void utilisateurA(void) {
   while (true) {
		batterie.wait();
		radio.wait();
      printf("Utilisateur A écoute\n\r");
  	   Thread::wait(10 + rand() % 200 );
		radio.release();
		batterie.release();
   }
}

void utilisateurB(void) {
   while (true) {
		radio.wait();
		batterie.wait();
      printf("Utilisateur B écoute\n\r");
		Thread::wait(10 + rand() % 200 );
		batterie.release();
		radio.release();
   }
}

int main (void) {
    Thread threadA;
    Thread threadB;
	 
	 threadA.start(utilisateurA);
	 threadB.start(utilisateurB);
    
    Thread::wait(osWaitForever);
}