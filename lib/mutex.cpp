//
// OpenRoboVision
//
// Mutex - object for threads synchronization
//
// Mutex (мьютекс) - объект для синхронизации потоков
//
//
// robocraft.ru
//

#include "mutex.h"

Mutex::Mutex()
{
	// initialization
	// инициализация
#if defined(WIN32)

	mutex = CreateMutex(NULL, FALSE, NULL);

#elif defined(LINUX)

#if 1
	pthread_mutex_init( &mutex, NULL);
#else
	// для использования рекурсивного мьютекса
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &attr);
	pthread_mutexattr_destroy(&attr);
#endif

#endif
}

Mutex::~Mutex()
{
	// destroy mutex
	// уничтожение мьютекса
#if defined(WIN32)

	CloseHandle(mutex);

#elif defined(LINUX)

	pthread_mutex_destroy( &mutex );

#endif
}

// захватить
void Mutex::Lock()
{
#if defined(WIN32)

	WaitForSingleObject( mutex, INFINITE );
	
#elif defined(LINUX)

	pthread_mutex_lock(&mutex);
	
#endif
}

// освободить
void Mutex::Unlock()
{
#if defined(WIN32)

	ReleaseMutex(mutex);

#elif defined(LINUX)

	pthread_mutex_unlock(&mutex);
	
#endif
}
