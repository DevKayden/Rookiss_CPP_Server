#include "pch.h"
#include "Lock.h"
#include "DeadLockProfiler.h"


/*
	WriteLock은 내가 재귀적으로 잡고 있거나, Read도 없고, Write도 없는 경우에만 소유권 획득가능하다.
*/

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 아무도 소유 및 공유하고 있지 않을 때, 경합해서 소유권을 얻는다.


	// 동일한 쓰레드가 소유하고 있다면 무조건 성공하게 만들어야 한다?  -> 왜지?
	// 락을 잡고 있는 쓰레드의 ID를 가져온다.
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}
	

	// 너무 오랫동안 while문을 돌고 있으면 의도적으로 크래시를 낸다. 시간을 측정해서 그걸 넘으면
	const int64 beginTick = ::GetTickCount64();
	
	// 내 쓰레드 ID를 16비트 쉬프트 라이트 하고 Mask와 And연산해서 desired에 저장
	// 이 값을 _lockFlag에 넣으면 소유권을 가진거지
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);

	while (true)
	{
		// 5000번 동안 경쟁을 시도할 기회를 준다.
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{

			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				// 여기까지 들어왔으면 경합에서 이겨서 소유권을 얻은 상황이다.
				_writeCount++;
				
				/* _
					writeCount를 둬서 내가 WriteLock을 잡고 있는 상태에서
					다시한번 writeLock을 호출하면 표준 mutex처럼 크래시가 나는게 아니라
					_writeCount를 증가시키고 재귀적으로 Lock을 잡는걸 허용해준다.

					또한 _writeCount로 재귀적으로 몇번이나 lock을 잡았는지 추적도 가능
				*/

				return;
			}

		}

		// timeout tick보다 커지면 의도적 크래시
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("Lock_Timeout");

		// 5000번 돌았는데 실패했으면 cpu 놓고, 다시 할당 받으면 다시 5000번 시작
		this_thread::yield();

	}

}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	//ReadLock 다 풀기 전에는 WriteUnlock 불가능.
	//ReadLock 걸려 있는 상태인데 WriteUnlock()하려고하면 의도적으로 크래시를 낸다.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");


	// _writeCount를 1 줄여주고, 만약 그게 0이면 재귀적으로 Write한게 끝난거니
	// EMPTY_FLAG로 밀어준다.
	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
	{
		_lockFlag.store(EMPTY_FLAG);
	}
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 WriteLock을 소유하고 있다면 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	// 아무도 소유하고 있지 않을때 경합해서 공유 카운트를 올린다.
	// 누구도 WriteLock을 잡고 있지 않으면
	 
	const int64 beginTick = ::GetTickCount64(); // 너무 오랫동안 while문을 돌고 있으면 의도적으로 크래시를 낸다. 시간을 측정해서 그걸 넘으면

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		// timeout tick보다 커지면 의도적 크래시
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("Lock_Timeout");

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE UNLOCK");
}
