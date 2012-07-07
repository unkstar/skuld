#include "global.h"
#include <errno.h>

Mutex::Mutex()
{
	pthread_mutex_init(&mutex_, 0);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex_);
}

void Mutex::Lock()
{
	pthread_mutex_lock(&mutex_);
}

void Mutex::Unlock()
{
	pthread_mutex_unlock(&mutex_);
}

Semaphore::Semaphore(long initialCount /* = 0 */)
{
  sem_init(&sem_, 0, initialCount);
}

Semaphore::~Semaphore()
{
	sem_destroy(&sem_);
}

void Semaphore::release()
{
	sem_post(&sem_);
}

bool Semaphore::acquire()
{
  int ret;
  while ((ret = sem_wait(&sem_)) == -1 && errno == EINTR);
  return 0 == ret;
}
