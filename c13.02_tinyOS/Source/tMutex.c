#include "tinyOS.h"

#if TINYOS_ENABLE_MUTEX == 1
void tMutexInit (tMutex * mutex)
{
	tEventInit(&mutex->event, tEventTypeMutex);
	
	mutex->lockedCount = 0;
	mutex->owner = (tTask *)0;
	mutex->ownerOrignalPrio = TINYOS_PRO_COUNT;
}

uint32_t tMutexWait (tMutex * mutex, uint32_t waitTicks)
{
	uint32_t status = tTaskEnterCritical();
	if (mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOrignalPrio = currentTask->prio;
		mutex->lockedCount++;
		
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else
	{
		if (mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		else
		{
			if (currentTask->prio < mutex->owner->prio)
			{
				tTask * owner = mutex->owner;
				if (owner->state == TINYOS_TASK_STATE_RDY)
				{
					tTaskSchedUnRdy(owner);
					owner->prio = currentTask->prio;
					tTaskSchedRdy(owner);
				}
				else
				{
					owner->prio = currentTask->prio;
				}
			}
			tEventWait(&mutex->event, currentTask, (void *)0, tEventTypeMutex, waitTicks);
			tTaskExitCritical(status);
			
			tTaskSched();
			return currentTask->waitEventResult;
		}
	}
}

uint32_t tMutexNoWaitGet (tMutex * mutex)
{
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount <= 0)
	{
		mutex->owner = currentTask;
		mutex->ownerOrignalPrio = currentTask->prio;
		mutex->lockedCount++;
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	else 
	{
		if (mutex->owner == currentTask)
		{
			mutex->lockedCount++;
			tTaskExitCritical(status);
			return tErrorNoError;
		}
		tTaskExitCritical(status);
		return tErrorResourceUnavaliable;
	}
}

uint32_t tMutexNotify (tMutex * mutex)
{
	uint32_t status = tTaskEnterCritical();
	if (mutex->lockedCount <= 0)
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	if (mutex->owner != currentTask)
	{
		tTaskExitCritical(status);
		return tErrorOwner;
	}
	
	if (--mutex->lockedCount> 0)
	{
		tTaskExitCritical(status);
		return tErrorNoError;
	}
	
	if (mutex->ownerOrignalPrio != mutex->owner->prio)
	{
		if (mutex->owner->state == TINYOS_TASK_STATE_RDY)
		{
			tTaskSchedRdy(mutex->owner);
			currentTask->prio = mutex->ownerOrignalPrio;
			tTaskSchedRdy(mutex->owner);
		}
		else
		{
			currentTask->prio = mutex->ownerOrignalPrio;
		}
	}
	
	if (tEventWaitCount(&mutex->event) > 0)
	{
		tTask * task = tEventWakeUp(&mutex->event, (void *)0, tErrorNoError);
		
		mutex->owner = task;
		mutex->ownerOrignalPrio = task->prio;
		mutex->lockedCount++;
		
		if (task->prio < currentTask->prio)
		{
			tTaskSched();
		}
	}
	tTaskExitCritical(status);
	return tErrorNoError;
}

uint32_t tMutexDestroy (tMutex * mutex)
{
	uint32_t count = 0;
	uint32_t status = tTaskEnterCritical();
	
	if (mutex->lockedCount>0)
	{
		if (mutex->ownerOrignalPrio != mutex->owner->prio)
		{
			if (mutex->owner->stack == TINYOS_TASK_STATE_RDY)
			{
				tTaskSchedUnRdy(mutex->owner);
				mutex->owner->prio = mutex->ownerOrignalPrio;
				tTaskSchedRdy(mutex->owner);
			}
			else
			{
				mutex->owner->prio = mutex->ownerOrignalPrio;
			}
		}
		count = tEventRemoveAll(&mutex->event, (void *)0, tErrorDel);
			if (count > 0)
			{
				tTaskSched();
			}
	}
		tTaskExitCritical(status);
		return count;
	
}

void tMutexGetInfo (tMutex * mutex, tMutexInfo * info)
{
	uint32_t status = tTaskEnterCritical();
	
	info->taskCount = tEventWaitCount(&mutex->event);
	info->ownerPrio = mutex->ownerOrignalPrio;
	if (mutex->owner != (tTask *)0)
	{
		info->inheritedPrio = mutex->owner->prio;
	}
	else
	{
		info->inheritedPrio = TINYOS_PRO_COUNT;
	}
	info->owner = mutex->owner;
	info->lockedCount = mutex->lockedCount;
	
	tTaskExitCritical(status);
}

#endif
