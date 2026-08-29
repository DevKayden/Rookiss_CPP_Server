#pragma once
#include "Types.h"


/*-----------------------
		RW SpinLock
-----------------------*/

/*--------------------------------------------------------------
우리가 구현한 RW SpinLock의 정책:
	
	W -> W (o)
	W -> R (o)  Write하다가 Read는 가능
	R -> W (x)  Read하다가 Write는 불가

	즉, ReadLock이 다 풀리기 전에는 WriteLock의 Unlock은 불가

--------------------------------------------------------------*/


class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		/*
			WRITE_THREAD_MASK는 아마 And 연산으로 상위비트만을
			뽑기위한 마스크이다.
		*/
		READ_COUNT_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

public:
	// read, write 별로 다른 락과 언락을 한다.

	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);


private:
	atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
	// _writeCount는 경합에서 성공한 애만 값을 바꾸기 때문에 atomic이 아니어도 된다.
};


/*-----------------------
		LockGuards
-----------------------*/

//RAII 구조로
class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(_name); };
	~ReadLockGuard() { _lock.ReadUnlock(_name); };

private:

	Lock& _lock;
	const char* _name;
};


class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(_name); };
	~WriteLockGuard() { _lock.WriteUnlock(_name); };

private:

	Lock& _lock;
	const char* _name;
};
