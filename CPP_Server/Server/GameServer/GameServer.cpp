#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <mutex>
#include <windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

class TestLock
{
	USE_LOCK;

public:
	int32 TestRead()
	{
		READ_LOCK;

		if (_queue.empty())
			return -1;

		return _queue.front();
	}

	void TestPush()
	{
		WRITE_LOCK;

		_queue.push(rand() % 100);
	}

	void TestPop()
	{
		WRITE_LOCK;

		if (_queue.empty() == false)
			_queue.pop();
	}

private:
	queue<int32> _queue;
};

TestLock testLock;

void ThreadWrite()
{
    while (true)
    {
		testLock.TestPush();
		this_thread::sleep_for(1ms);
		testLock.TestPop();
    }
}

void ThreadRead()
{
	while (true)
	{
		int32 value = testLock.TestRead();
		cout << value << endl;
	}
	
}

int main()
{
    for (int32 i = 0; i < 2; i++)
    {
        GThreadManager->Launch(ThreadWrite);
    }
	
	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch(ThreadRead);
	}


    /*
        GThreadManager는 CoreGlobal에서 전역으로 선언되어 있는
        ThreadManager의 포인터 변수이다.
    */
    GThreadManager->Join();
}
