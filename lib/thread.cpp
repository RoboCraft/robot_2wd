//
// OpenRoboVision
//
// base class for create threads
//
// ������� ����� ��� �������� �������
//
//
// robocraft.ru
//

#include "thread.h"

#include <stdio.h>

// ������ ����������� � �������� ���������, ��������� ��������, 
// ������� �������������� ��� ��������� �� 
// �����, ��� �������� ���������� ����� ThreadFunc()
static void ThreadProc(BaseThread* who) // ���������� ��������� �������
{
#if defined(LINUX)
	// ������ ����� "���������" ����� pthread_cancel.
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
	// ��� ����������� �������, �������� �����
	Kill(false);
}

// ������ ������
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

	if (!critical) // ������������� ����� � ����������
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

	if(exit_code == STILL_ACTIVE) // ���� ����� ��� �� �����������, "�����" ���
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

// ������������� � ������.
// ������� ������ ���������� ������ ����� ����� �������� ������.
void BaseThread::Join()
{
#if defined(WIN32)
	WaitForSingleObject(thread,  INFINITE);
#elif defined(LINUX)
	pthread_join(thread, 0);
#endif
}
