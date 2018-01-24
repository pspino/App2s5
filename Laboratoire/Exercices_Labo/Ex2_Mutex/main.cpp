#include "mbed.h"
#include "rtos.h"

Mutex stdio_mutex; 

void notify(const char* name, int state) {
    stdio_mutex.lock();        
    printf("%s: %d\n\r", name, state); 
	 Thread::wait(10);        
    stdio_mutex.unlock();  
}

void test_thread(void const *args) {
	osStatus err;
    while (true) {
       notify((const char*)args, 0); 
       Thread::wait(10 + rand() % 200 );
       notify((const char*)args, 1); 
		 Thread::wait(10 + rand() % 200 );
    }
}

int main() {
    Thread t2;
    Thread t3;
	 
	  t2.start(callback(test_thread, (void *)"Le Thread # 2"));
	  t3.start(callback(test_thread, (void *)"Le Thread # 3"));
    
    test_thread((void *)"Le Thread # 1");
}
