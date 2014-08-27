//
// OpenRoboVision
//
// base class for create threads
//
// базовый класс для создания потоков
//
//
// robocraft.ru
//

#include "thread.h"

#include <stdio.h>

// фукция указывается в качестве потоковой, принимает параметр, 
// который обрабатывается как указатель на 
// класс, для которого вызывается метод ThreadFunc()
static void ThreadProc(BaseThread* who) // глобальная потоковая функция
{
#if defined(LINUX)
	// Делаем поток "убиваемым" через pthread_cancel.
	int old_thread_type;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_thread_type);
#endif
	if(who)
		who->Execute();
}

BaseThread::BaseThread():
	thread(0),
	is_terminated(false)
{
}

BaseThread::~BaseThread()
{
	// при уничтожении объекта, завершим поток
	Kill(false);
}

// запуск потока
bool BaseThread::Start()
{
	if(thread) {
		return false;
	}

#if defined(WIN32)

	SIZE_T stackSize = 0;
	thread = CreateThread( NULL,	// default security attributes
				stackSize,			// stack size (default 1Mb)
				reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadProc), // thread function
                this,				// argument to thread function
                0,					// use default creation flags
				0);					// returns the thread identifier

	if(thread==NULL)
	{
		printf("[!][BaseThread] Error: create thread!\n");
		return false;
	}

#elif defined(LINUX)

	pthread_create (&thread, 0, reinterpret_cast<pthread_callback>(ThreadProc), this);

#endif

	return (thread != 0);
}

int BaseThread::Kill(bool critical) 
{
	if (thread == 0) 
		return -1;

	is_terminated = true;
	int res = -1;

#if defined(WIN32)
	DWORD exit_code = 0;

	if (!critical) // предупреждает поток о завершении
	{ 
		for(int i = 10; i > 0; i--)
		{
			GetExitCodeThread(thread, &exit_code);
			if(exit_code == STILL_ACTIVE)
				Sleep(25);
			else
				break;
		}
	}
	
	GetExitCodeThread(thread, &exit_code);

	if(exit_code == STILL_ACTIVE) // если поток сам не завершается, "убьем" его
		TerminateThread(thread, -1);

	GetExitCodeThread(thread, &exit_code);
	CloseHandle(thread);
	thread = 0;
	is_terminated = false;

	res = (int)exit_code;

#elif defined(LINUX)

	if (!critical) 
	{
		usleep(250*1000);
	}
	res = pthread_cancel(thread);
	thread = 0;
	is_terminated = false;

#endif

	return res;
}

// Присоединение к потоку.
// функция вернет управление только когда поток завершит работу.
void BaseThread::Join()
{
#if defined(WIN32)
	WaitForSingleObject(thread,  INFINITE);
#elif defined(LINUX)
	pthread_join(thread, 0);
#endif
}
