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
// ������� ����� ��� �������� �������
//
class BaseThread 
{
public:
	BaseThread();
	virtual ~BaseThread(); 

	// ����� ���������������� � ����������� �������
	virtual void Execute() = 0;

	// ������ ������
	bool Start();

	// ��������� ������ ������
	int Kill(bool critical = false);

	// ������������� � ������.
	// ������� ������ ���������� ������ ����� ����� �������� ������.
	void Join();

	// ������������� ����
	ThreadType get() const {return thread;}

protected:
	// ���� ����������� ������, ��� ���� ��������� ���� ������ ���������.
	bool is_terminated;

private:
	ThreadType thread;

	// ����������� � ������������ �� �����������.  ������������ �� �������������,
    // ������� ��������������� ������ ���������.
    BaseThread( const BaseThread & );
    BaseThread & operator=( const BaseThread & );
};

#endif //#ifndef _THREAD_H_
