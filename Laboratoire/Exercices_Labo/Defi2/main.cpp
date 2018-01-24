/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Priority Inversion Example

*----------------------------------------------------------------------------*/

#include "mbed.h"
#include "cmsis_os.h"


DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

// Pour permettre de visualiser les moments d'activités des tâches

DigitalOut pA(p21);
DigitalOut pB(p22);
DigitalOut pC(p23);
DigitalOut pD(p24);

void phaseA (void const *argument);
void phaseB (void const *argument);
void phaseC (void const *argument);
void phaseD (void const *argument); 
void Delay(__IO uint32_t nCount);

osThreadDef(phaseA, osPriorityNormal, DEFAULT_STACK_SIZE);
osThreadDef(phaseB, osPriorityNormal, DEFAULT_STACK_SIZE);
osThreadDef(phaseC, osPriorityNormal, DEFAULT_STACK_SIZE);
osThreadDef(phaseD, osPriorityNormal, DEFAULT_STACK_SIZE);

osThreadId t_main,t_phaseA,t_phaseB,t_phaseC,t_phaseD;

// Phase A Priorité haute
void phaseA (void const *argument) 
{
	for (;;) 
	{
		led3=1;
		pA = 1;
		osDelay(10);
		osSignalSet(t_phaseD,0x0001);						    	//Réveille Phase D.
		osSignalWait(0x02,osWaitForever);						//ERREUR!! attend une réponse de Phase D pour terminer
		pA = 0;
	}
}

// Phase B Priorité moyenne
void phaseB (void const *argument) 
{
	for (;;) 
	{
		pB = 1;
		Delay(100);
		led2=1; 
		Delay(100);
		osSignalSet(t_phaseC,0x01);	 
		pB = 0;		
	}
}

// Phase C Priorité moyenne
void phaseC (void const *argument) 
{
	for (;;) 
	{
		pC = 1;
		osSignalWait(0x01,osWaitForever);  
		led2=0;
		pC = 0;		
	}
}

// Phase D Priorité basse
 void phaseD (void const *argument) 
{
	for (;;) 
	{
		pD = 1;
		osSignalWait(0x01,osWaitForever);
		Delay(10);
		led3=0; 
		osSignalSet(t_phaseA,0x02);   
		pD = 0;
	}
}

int main(void)
{

	t_main = osThreadGetId ();
	osThreadSetPriority(t_main,osPriorityHigh);          // Ceci assure que le main va s'exécuter (la tâche haute priorité pourrait l'arrêter
	t_phaseA = osThreadCreate(osThread(phaseA), NULL);
	t_phaseB = osThreadCreate(osThread(phaseB), NULL);
	t_phaseC = osThreadCreate(osThread(phaseC), NULL);
	t_phaseD = osThreadCreate(osThread(phaseD), NULL);
	osThreadTerminate(t_main);										// Pour éviter d'avoir une autre tâche au niveau moyen, on termine le main.
	
	while(1) 														   // ne devrait jamais s'exécuter puisque le main est arrêté
	{
		led1 = !led1;
	}
}

// Cette fonction exécute quelque chose. Les tâches ne sont donc pas en attente.
 void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}



