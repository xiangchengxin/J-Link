#include "tinyOS.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];

tMutex mutex;
int task1Flag;
void task1Entry (void * param)
{
	tSetSysTickPeriod(10);
	tMutexInit(&mutex);
	for(;;)
	{	
		tMutexWait(&mutex, 0);
		tMutexWait(&mutex, 0);
		
		task1Flag = 0;
		tTaskDelay(1);
		task1Flag = 1;
		tTaskDelay(1);
		
		tMutexNotify(&mutex);
		tMutexNotify(&mutex);
	}	
}

int task2Flag;
void task2Entry (void * param)
{
	
	for (;;)
	{
		tMutexWait(&mutex, 0);
		tMutexWait(&mutex, 0);
		
		task2Flag = 0;
		tTaskDelay(1);
		task2Flag = 1;
		tTaskDelay(1);
		
		tMutexNotify(&mutex);
		tMutexNotify(&mutex);
	}
}

int task3Flag;
void task3Entry (void * param)
{
	for(;;)
	{	
		task3Flag = 0;
		tTaskDelay(1);
		task3Flag = 1;
		tTaskDelay(1);
	}	
}

int task4Flag;
void task4Entry (void * param)
{

	for(;;)
	{		
		task4Flag = 0;
		tTaskDelay(1);
		task4Flag = 1;
		tTaskDelay(1);
	}	
}



void tInitApp (void)	
{
	tTaskInit(&tTask1, task1Entry, (void *) 0x11111111, 0,&task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *) 0x22222222, 1,&task2Env[1024]);
	tTaskInit(&tTask3, task3Entry, (void *) 0x33333333, 1,&task3Env[1024]);
	tTaskInit(&tTask4, task4Entry, (void *) 0x44444444, 1,&task4Env[1024]);
}


