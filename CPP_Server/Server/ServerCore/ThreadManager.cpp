#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"


/*-------------------------
		ThreadManager
-------------------------*/


ThreadManager::ThreadManager()
{
	// Main Thread
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

/*
	Launch 함수 설명:

	TLS를 따로 번호를 부여하고 관리를 하기 위해서 ThreadManager를 만든 것 같음.
	원래 thread를 생성해서 함수를 넘겨주는 방식으로 만들었지만, TLS 관리를 같이 하려고
	Launch라는 함수를 만들어서

	1) TLS 초기화
	2) callback 함수 호출
	3) TLS 삭제

	과정을 거치는 것 같음. 근데 이러면 쓰레드를 생성해서 callback함수를 호출한게 아니지 않나?
	병렬적으로 실행이 되나?

	아니네 push_back 인자로 thread를 생성하면서 위의 세가지를 해당 쓰레드가 하는듯.
	사실상 1, 3번을 해주기 위해서 만들어진 거다.

	인자 값에 대한 설명:
	function<void(void)> callback

	리턴값, 인자값이 void는 함수를 받아주는 것 같음. 

	
	*/

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	_threads.clear();

}

/*
	쓰레드 Id를 부여하는 ID발급기 역할
*/

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
	// fetch_add는 인자로 들어온만큼 더해준다. 
	// 하지만 리턴 값은 더하기 전의 값을 리턴함.

}

void ThreadManager::DestroyTLS()
{
	// 나중에 동적으로 할동하는 것을 관리하는 코드 작성
}
