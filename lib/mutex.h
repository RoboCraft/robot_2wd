//
// OpenRoboVision
//
// Mutex - object for threads synchronization
//
//
//
// robocraft.ru
//

#ifndef _MUTEX_H_
#define _MUTEX_H_

#if defined(WIN32)
# include <windows.h>
# include <process.h>
typedef HANDLE MutexType;
#elif defined(LINUX)
# include <pthread.h>
typedef pthread_mutex_t MutexType;
#endif //#ifdef WIN32

class Mutex
{
public:
	Mutex();
	~Mutex();

	// захватить
	void Lock();
	// освободить
	void Unlock();

	// получить дескриптор мьютекса
	MutexType get() const { return mutex; }

private:
	MutexType mutex;

	// Копирование и присваивание не реализовано.  Предотвратим их использование,
    // объявив соответствующие методы закрытыми.
    Mutex( const Mutex & );
    Mutex & operator=( const Mutex & );
};

//
// оболочка для захвата-освобождения мьютекса
//
// ( частный случай методики "Выделение ресурса — есть инициализация" (RAII) )
// http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
//
class MutexAutoLock {
public:

	// захват мютекса при конструировании объекта
	MutexAutoLock(Mutex &_mutex):
	  mutex(_mutex)
	{
		// захватить мьютекс
		mutex.Lock();
	}
	~MutexAutoLock()
	{
		// освободить мьютекс
		mutex.Unlock();
	}

private:
	Mutex& mutex;

	// Копирование и присваивание не реализовано.  Предотвратим их использование,
    // объявив соответствующие методы закрытыми.
    MutexAutoLock( const MutexAutoLock & );
    MutexAutoLock & operator=( const MutexAutoLock & );
};

#endif //#ifndef _MUTEX_H_
