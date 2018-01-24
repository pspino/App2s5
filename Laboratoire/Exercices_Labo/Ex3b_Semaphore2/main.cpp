#include "mbed.h"
#include "rtos.h"

Semaphore nb_sem(1); 

void notify(const char* name, int state) {
   nb_sem.wait();
   printf("%s: %d\n\r", (const char*)name, state);
   Thread::wait(100);
   nb_sem.release();    
}

void test_thread(void const *name) {
    while (true) {
       notify((const char*)name, 0);
       Thread::wait(10 + rand() % 200 );
       notify((const char*)name, 1);
       Thread::wait(10 + rand() % 200 );
    }
}

int main (void) {
    Thread t2;
    Thread t3;
	 
	  t2.start(callback(test_thread, (void *)"Le Thread # 2"));
	  t3.start(callback(test_thread, (void *)"Le Thread # 3"));
    
    test_thread((void *)"Le Thread # 1");
}