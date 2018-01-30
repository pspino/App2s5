//Jacob Fontaine fonj1903 Philippe Spino spip2401
#include "mbed.h"
#include "rtos.h"

//Les pins en entrée
DigitalIn in1(p21);
DigitalIn in2(p22);
AnalogIn an1(p19);
AnalogIn an2(p20);

//Les leds de test
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

//Structure d'un évènement qui contient les pins allumés, leur valeur, et le temps de l'évènement
typedef struct 
{
	uint8_t				pins; 				//7:4 reservé -> 3:0 pins.
	uint32_t    	value;
  time_t    	timeStamp;
}Event;

//Une Queue pour recevoir le timestamp du Thread RTC
Queue<time_t,10> timeQueue;
//Une Queue pour contenir les évènements
Queue<Event,50> fifo;
//Les 2 mutex pour bloquer les sections critiques
Mutex fifoMutex;
Mutex timeMutex;

//Thread pour les évènements numérique
void NumericP1()
{
	uint8_t bit0 = 0;
	uint8_t bit1 = 0;
	uint8_t lastReading0 = 0;
	uint8_t lastReading1 = 0;
	bool newValue = false;
	while (true) 
	{
		int count0 = 0;
		int count1 = 0;
		uint8_t idx = 0;
		uint8_t currentReading0 = 0;
		uint8_t currentReading1 = 0;
		//On boucle 20 fois avec un wait de 5ms pour créer une lecture au 100 ms
		while(idx < 20)
		{
			//Si on compte 10 fois la même valeur, on accepte l'évènement
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
			
			//Si on lis le même bit que le précédent, on incremente, sinon on repars le compteur
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
			//On lock les mutex car nous allons lire dans les queues
			fifoMutex.lock();
			timeMutex.lock();
			//On appelle le thread RTC pour qu'il nous donne l'heure actuelle.
			osEvent timeEvent = timeQueue.get(5);
			timeMutex.unlock();
			time_t* tm = (time_t*)timeEvent.value.p;
			struct tm * time;
			time = localtime(tm);
			newValue = false;
			uint8_t bit = bit0 | (bit1 << 1);
			//On crée un évènement et on le met dans la queue du event
			Event event = {bit, 0x0000, *tm};
			fifo.put(&event);
		}
		//On libère le mutex
		fifoMutex.unlock();
	}
}

void analogRead() 
{
	uint16_t an1CurrentMean =0;
	uint16_t an2CurrentMean =0;
	while (true) 
	{
		uint16_t an1Mean =0;
		uint16_t an2Mean =0;
		uint8_t analogPins =0;
		//On veux lire 5 échantillions sur 250ms, donc on wait 50ms
		for(uint8_t index =0;index<5;index++)
		{
			an1Mean +=an1.read_u16();
			an2Mean +=an2.read_u16();
			Thread::wait(50);
		}
		//On calcul la moyenne et on véririe si la nouvelle moyenne dépasse de 12,5% l'ancienne
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
			//Si on a un nouvel évènement, on lock les mutex, on va lire l'heure avec RTC, et on remplis la queue d'un new Event.
			fifoMutex.lock();
			timeMutex.lock();
			osEvent timeEvent = timeQueue.get(5); 
			timeMutex.unlock();
			time_t* tm = (time_t*)timeEvent.value.p;
			uint32_t analogValue = an1CurrentMean | an2CurrentMean << 16;
			Event analogEvent = {analogPins,analogValue,*tm};
			fifo.put(&analogEvent);
		}
		fifoMutex.unlock();
	}
}

void RTCRead()
{
	//On lit l'heure actuelle au 50ms et on le met dans la queue qui gère le timestamp
	while (true) 
	{	
		//	Thread::signal_wait(0);
		  time_t timeStamp;
			time(&timeStamp );
			timeQueue.put(&timeStamp);
			Thread::wait(50);
	}
}

int main()
{	
	//On vient mettre un temps par défaut
	set_time(1517323190);
	
	//Création des 4 threads nécessaire
	osThreadId mainId = osThreadGetId();
	osThreadSetPriority(mainId, osPriorityHigh);
	
	Thread numReadThread1;
	numReadThread1.start(NumericP1);
	//Numérique est le plus prioritaire
	numReadThread1.set_priority(osPriorityAboveNormal);
	
	Thread analogReadThread;
	analogReadThread.start(analogRead);
	//Analog est d'une priorité normale
	analogReadThread.set_priority(osPriorityNormal);
	
	Thread RTCThread;
	RTCThread.start(RTCRead);
	//Le RTC est le thread le moins prioritaire
	RTCThread.set_priority(osPriorityLow);
	
	osThreadSetPriority(mainId, osPriorityNormal);
	while(true)
	{
		//On vient chercher le dernier évènement pour l'affichage
		int pins[4] = {0,0,0,0};
		osEvent fifoEvent = fifo.get(5);
		if(fifoEvent.status == osEventMessage)
		{
			//On lock le mutex pour que les valeurs ne change pas
			fifoMutex.lock();
			Event* event = (Event*)fifoEvent.value.p;
			int upPins = event->pins;
			int step = 0x0;
			//On vient chercher le paramètre de pins pour savoir quel est le type d'évènement
			while(upPins != 0)
			{
					int remainder = upPins%2;
					upPins /= 2;
					pins[step] = remainder;
					step += 1;
			}
			//pins 19 et 20
			if(event->pins == 0b00001100)
			{
				//permet de séparer un 32 bit en 2 16 bits
				 int pin19 = 0;
				 int pin20 = 0;				
				pin19 = 0xFF & event->value;
				pin20 = 0xFF000000 & event->value;
			//	pin20 = pin20 >> 16;
				//On vient mettre la date dans le bon format
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Analogique, valeur p19: %f, valeur p20: %f Timestamp: %s\n",((pin19/100)*3.3),((pin20/100)*3.3), buffer);

			}
			//pin 19
			else if(event->pins == 0b00000100)
			{
				//permet de séparer un 32 bit en 2 16 bits
				int pin19 = 0;
				pin19 = 0x00FF & event->value;			
				//On vient mettre la date dans le bon format
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Analogique, valeur p19: %f Timestamp: %s\n",((pin19/100)*3.3), buffer);

			}
			//pin 20
			else if(event->pins == 0b000001000)
			{
				//permet de séparer un 32 bit en 2 16 bits
				int pin20 = 0;
				pin20 = 0xFF000000 & event->value;		
		//		pin20 = pin20 >> 16;
				//On vient mettre la date dans le bon format
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Analogique, valeur p20: %f Timestamp: %s\n",((pin20/100)*3.3), buffer);

			}
			//pin 21
			else if(event->pins == 0b00000001)
			{
				//On vient mettre la date dans le bon format
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Numerique, une valeur a ete detecter sur la p21 Timestamp: %s\n", buffer);

			}
			//pin 22
			else if(event->pins == 0b00000010)
			{
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Numerique, une valeur a ete detecter sur la p22 Timestamp: %s\n", buffer);

			}
			//pins 21 et 22
			else if(event->pins == 0b00000011)
			{
				struct tm * time;
				time = localtime(&event->timeStamp);
				char buffer[20];
				strftime(buffer, 20, "%Y:%m:%d:%H:%M:%S", time);
				printf("Numerique, une valeur a ete detecter sur la p21 et p22 Timestamp: %s\n", buffer);
			}
			fifoMutex.unlock();
		}
		//On valid l'évènement en affichant les leds en conséquence
		led4 = pins[1];
		led3 = pins[0];
		led2 = pins[3];
		led1 = pins[2];
		//On limite la vitesse d'affichage.
		Thread::wait(100);
	}
}
