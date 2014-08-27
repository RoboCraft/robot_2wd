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

	// ���������
	void Lock();
	// ����������
	void Unlock();

	// �������� ���������� ��������
	MutexType get() const { return mutex; }

private:
	MutexType mutex;

	// ����������� � ������������ �� �����������.  ������������ �� �������������,
    // ������� ��������������� ������ ���������.
    Mutex( const Mutex & );
    Mutex & operator=( const Mutex & );
};

//
// �������� ��� �������-������������ ��������
//
// ( ������� ������ �������� "��������� ������� � ���� �������������" (RAII) )
// http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
//
class MutexAutoLock {
public:

	// ������ ������� ��� ��������������� �������
	MutexAutoLock(Mutex &_mutex):
	  mutex(_mutex)
	{
		// ��������� �������
		mutex.Lock();
	}
	~MutexAutoLock()
	{
		// ���������� �������
		mutex.Unlock();
	}

private:
	Mutex& mutex;

	// ����������� � ������������ �� �����������.  ������������ �� �������������,
    // ������� ��������������� ������ ���������.
    MutexAutoLock( const MutexAutoLock & );
    MutexAutoLock & operator=( const MutexAutoLock & );
};

#endif //#ifndef _MUTEX_H_
