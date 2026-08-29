#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "Service.h"
#include "Session.h"



/*
	진행 흐름 중요!

	1. Listener::StartAccept()에서 AcceptEx를 호출해서 예약을 해준다. 
	그러면 CP가 AcceptEx가 완료되면 Listener::Dispatch()를 호출하게 된다.

	2. 쓰레드를 여러개 생성해서 GIocpCore.Dispatch()를 호출하게 한다. 
	그럼 GetQueuedCompletionStatus()에서 대기하다가, AcceptEx가 완료되면 Listener::Dispatch()를 호출하게 된다.

	3. Listener::Dispatch()에서 AcceptEvent를 가져와서 ProcessAccept()를 호출한다.

	4. ProcessAccept()에서 세션을 연결해주고, 다시 RegisterAccept()를 호출해서 AcceptEx를 예약해준다.

	5. 쓰레드들은 계속 GIocpCore.Dispatch()를 호출하고 있으므로, AcceptEx가 완료되면 Listener::Dispatch()를 호출하게 된다.
*/

class GameSession : public Session
{

};

int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<Session>, // 현재는 그냥 세션을 쉐어드포인터 생성 함수지만, 나중에는 세션 매니저등 활용
		100);

	
	ASSERT_CRASH(service->Start());


	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	GThreadManager->Join();

}