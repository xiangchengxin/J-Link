#include "tinyOS.h"
#include "ARMCM3.h"

tTask * currentTask;
tTask * nextTask;
tTask * idleTask;
tTask * taskTable[2];

void tTaskInit (tTask * task, void (*entry)(void *), void * param, tTaskStack * stack)
{
	*(--stack) = (unsigned long)(1 << 24);
	*(--stack) = (unsigned long)(entry);
	*(--stack) = (unsigned long)0x14; //R14
	*(--stack) = (unsigned long)0x12; //R12
	*(--stack) = (unsigned long)0x3; //R3
	*(--stack) = (unsigned long)0x2; //R2
	*(--stack) = (unsigned long)0x1; //R1
	*(--stack) = (unsigned long)param; //R0
	
	//above: automatic
	
	*(--stack) = (unsigned long)0x11;
	*(--stack) = (unsigned long)0x10;
	*(--stack) = (unsigned long)0x9;
	*(--stack) = (unsigned long)0x8;
	*(--stack) = (unsigned long)0x7;
	*(--stack) = (unsigned long)0x6;
	*(--stack) = (unsigned long)0x5;
	*(--stack) = (unsigned long)0x4;
	
	task->stack = stack; //https://blog.csdn.net/faihung/article/details/79190039
	task->delayTicks = 0;
}

void tTaskSched ()
{
	if (currentTask == idleTask)
	{
		if(taskTable[0]->delayTicks == 0)
		{
			nextTask = taskTable[0];
		}
		else if (taskTable[1]->delayTicks == 0)
		{
			nextTask = taskTable[1];
		}
		else 
		{
			return;
		}
	}
	else
	{
		if (currentTask == taskTable[0])
		{
			if (taskTable[1]->delayTicks == 0)
			{
				nextTask = taskTable[1];
			}
			else if (currentTask->delayTicks != 0)
			{
				nextTask = idleTask;
			}
			else
			{
				return;
			}
		}
		else if (currentTask == taskTable[1])
		{
			if (taskTable[0]->delayTicks == 0)
			{
				nextTask = taskTable[0];
			}
			else if (currentTask->delayTicks !=0)
			{
				nextTask = idleTask;
			}
			else
			{
				return;
			}
		}
	}
	
	tTaskSwitch();
}

void tTaskSystemTickHandler ()
{
	int i;
	for (i = 0; i<2; i++)
	{
		if (taskTable[i]->delayTicks >0)
		{
			taskTable[i]->delayTicks--;
		}
	}
	
	tTaskSched();
}

void tTaskDelay (uint32_t delay)
{
	currentTask->delayTicks = delay;
	tTaskSched();
}
void tSetSysTickPeriod (uint32_t ms)
{
	SysTick->LOAD = ms *SystemCoreClock/1000 -1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) -1);
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | 
									SysTick_CTRL_TICKINT_Msk |
									SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler ()
{
	tTaskSystemTickHandler ();
}

void delay(int count)
{
	while(--count >0);
}




int task1Flag;

void task1Entry (void * param)
{
	
	tSetSysTickPeriod(10);
	for(;;)
	{
		task1Flag = 0;
		delay(100);
		task1Flag = 1;
		delay(100);
	}
	
}

int task2Flag;
void task2Entry (void * param)
{
	for(;;)
	{
		task2Flag = 0;
		delay(100);
		task2Flag = 1;
		delay(100);
	}
	
}

tTask tTask1;
tTask tTask2;

tTaskStack task1Env[1024];
tTaskStack task2Env[1024];

tTask tTaskIdle;
tTaskStack idleTaskEnv[1024];

void idleTaskEntry (void * param)
{
	for(;;)
	{
	}
}

int main()
{
	tTaskInit(&tTask1, task1Entry, (void *) 0x11111111, &task1Env[1024]);
	tTaskInit(&tTask2, task2Entry, (void *) 0x22222222, &task2Env[1024]);
	
	
	taskTable[0] = &tTask1;
	taskTable[1] = &tTask2;

	tTaskInit(&tTaskIdle, idleTaskEntry, (void *)0, &idleTaskEnv[1024]);
	idleTask= &tTaskIdle;
	
	nextTask = taskTable[0];
	
	tTaskRunFirst();
	
	return 0;
}
