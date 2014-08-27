//
// OpenRoboVision
//
// base class for create threads
//
// базовый класс дл€ создани€ потоков
//
//
// robocraft.ru
//

#ifndef _THREAD_H_
#define _THREAD_H_

#if defined(WIN32)

# include <windows.h>
# include <process.h>

typedef HANDLE ThreadType;

#elif defined(LINUX)

# include <unistd.h>
# include <pthread.h>

typedef pthread_t ThreadType;
typedef void *(*pthread_callback)(void *);

#endif

//
// базовый класс дл€ создани€ потоков
//
class BaseThread 
{
public:
	BaseThread();
	virtual ~BaseThread(); 

	// будет переопредел€тьс€ в наследующих классах
	virtual void Execute() = 0;

	// запуск потока
	bool Start();

	// завершает работу потока
	int Kill(bool critical = false);

	// ѕрисоединение к потоку.
	// функци€ вернет управление только когда поток завершит работу.
	void Join();

	// идентификатор нити
	ThreadType get() const {return thread;}

protected:
	// флаг указывающий потоку, что пора завершать свою работу корректно.
	bool is_terminated;

private:
	ThreadType thread;

	//  опирование и присваивание не реализовано.  ѕредотвратим их использование,
    // объ€вив соответствующие методы закрытыми.
    BaseThread( const BaseThread & );
    BaseThread & operator=( const BaseThread & );
};

#endif //#ifndef _THREAD_H_
