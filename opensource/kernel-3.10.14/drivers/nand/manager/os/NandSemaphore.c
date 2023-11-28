#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include "NandSemaphore.h"
#include <asm-generic/errno-base.h>

//val 1 is unLock val 0 is Lock
void __InitSemaphore(NandSemaphore *sem,int val)
{
	struct semaphore *__sem = vmalloc(sizeof(struct semaphore));
	sema_init(__sem,val);
	*sem = (NandSemaphore)__sem;
}

void __DeinitSemaphore(NandSemaphore *sem)
{
	int ret;
	struct semaphore *__sem = (struct semaphore *)(*sem);
	up(__sem);
	ret = down_killable(__sem);
	if (ret == -EINTR)
		printk("waring: %s, %d\n", __func__, __LINE__);
	vfree(__sem);
}

void __Semaphore_wait(NandSemaphore *sem)
{
	down((struct semaphore *)(*sem));
}

//timeout return < 0
int __Semaphore_waittimeout(NandSemaphore *sem,long jiffies)
{
	return down_timeout((struct semaphore *)(*sem),jiffies);
}

void __Semaphore_signal(NandSemaphore *sem)
{
	up((struct semaphore *)(*sem));
}

//#define DEBUG_NDMUTEX
void __InitNandMutex(NandMutex *mutex)
{
	struct mutex *__mutex = vmalloc(sizeof(struct mutex));
	mutex_init(__mutex);
	*mutex = (NandMutex)__mutex;
}

void __DeinitNandMutex(NandMutex *mutex)
{
	struct mutex *__mutex = (struct mutex *)(*mutex);
	mutex_destroy(__mutex);
	vfree(__mutex);
}

void __NandMutex_Lock(NandMutex *mutex)
{
	mutex_lock((struct mutex *)(*mutex));
}

void __NandMutex_Unlock(NandMutex* mutex)
{
	mutex_unlock((struct mutex *)(*mutex));
}

int __NandMutex_TryLock(NandMutex *mutex)
{
	return mutex_trylock((struct mutex *)(*mutex));
}

typedef struct __ndspinlock {
	spinlock_t lock;
	unsigned long flags;
} ndspinlock;

void __InitNandSpinLock(NandSpinLock *lock)
{
	ndspinlock *ndlock = vmalloc(sizeof(ndspinlock));
	spin_lock_init(&(ndlock->lock));
	*lock = (NandSpinLock)ndlock;
}

void __DeInitNandSpinLock(NandSpinLock *lock)
{
	ndspinlock *ndlock = (ndspinlock *)(*lock);
	vfree(ndlock);
}

void __NandSpinLock_Lock(NandSpinLock *lock)
{
	ndspinlock *ndlock = (ndspinlock *)(*lock);
	//spin_lock_irqsave(&(ndlock->lock), ndlock->flags);
	spin_lock(&(ndlock->lock));
}

void __NandSpinLock_Unlock(NandSpinLock *lock)
{
	ndspinlock *ndlock = (ndspinlock *)(*lock);
	//spin_unlock_irqrestore(&(ndlock->lock), ndlock->flags);
	spin_unlock(&(ndlock->lock));
}
