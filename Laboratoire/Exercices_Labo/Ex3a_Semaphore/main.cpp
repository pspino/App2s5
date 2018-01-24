#include "mbed.h"
#include "rtos.h"

Semaphore nb_sem(3); 

void test_thread(void const *name) {
    while (true) {
        nb_sem.wait();
        printf("%s\n\r", (const char*)name);
        Thread::wait(1000);
        nb_sem.release();
    }
}

int main (void) {
    Thread t2;
    Thread t3;
	 
	  t2.start(callback(test_thread, (void *)"Le Thread # 2"));
	  t3.start(callback(test_thread, (void *)"Le Thread # 3"));
    
    test_thread((void *)"Le Thread # 1");
}