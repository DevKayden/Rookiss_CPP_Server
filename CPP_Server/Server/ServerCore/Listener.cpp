#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

/*----------------
	Listener
----------------*/

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		//TODO

		xdelete(acceptEvent);
	}
}

// 리슨 소켓을 생성하고, Completion Port에 등록하고, 소켓 옵션을 설정하는 함수
bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	// 소켓을 Completion Port에 등록한다. 실패하면 false를 리턴한다.
	if (_service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	// 이 설정을 안해주면 주소가 겹쳐서 서버가 실행이 안될 수 있다. (특히 서버를 재시작할 때)
	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;
	
	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	// 해당 소켓에 IP주소를 바인딩한다. 실패하면 false를 리턴한다.
	if (SocketUtils::Bind(_socket, _service->GetNetAddress()) == false)
		return false;

	// 소켓을 리슨 상태로 만든다. 실패하면 false를 리턴한다.
	if (SocketUtils::Listen(_socket) == false)
		return false;

	// 많은 동접이 있을때 AcceptEx를 여러개 예약해두면, 동접이 많아도 AcceptEx가 바로바로 처리할 수 있다.
	const int32 acceptCount = _service->GetMaxSessionCount(); // 일단은 한개로만 만들어둔다.
	for (int32 i = 0; i < acceptCount; i++)
	{
		// 여기서 AcceptEx를 호출해서 실질적으로 예약을 해줘야 한다.
		// AcceptEvent를 생성하고, RegisterAccept를 호출해서 AcceptEx를 예약한다.
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		// 내부적으로 자신의 shared_ptr을 추출하려면 enable_shared_from_this를 상속받아야 한다.
		// 즉, IocpObject가 enable_shared_from_this를 상속받게 한다.
		acceptEvent->owner = shared_from_this(); // AcceptEvent를 소유한 IocpObject를 Listener로 설정한다.
		_acceptEvents.push_back(acceptEvent); // 나중에 삭제할 수 있도록 Vector에서 관리
		RegisterAccept(acceptEvent); // 이 함수의 바디에서 실제 AcceptEx를 호출한다.
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	// GetHandle()은 IocpObject가 관리하는 소켓의 핸들을 반환한다.
	// reinterpret_cast를 사용해서 SOCKET을 HANDLE로 변환한다.
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	// EvnetType이 Accept인지 확인하고, Accept이면 ProcessAccept를 호출한다.
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent); // AcceptEx가 완료되었으므로, ProcessAccept()를 호출해서 세션을 연결해주게 된다.
	
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	// Accept이벤트를 걸어줘서 실질적으로 iocp쪽에서 처리할 수 있도록 일감을 예약한다.
	// 이게 가장 핵심인 부분이다. AcceptEx를 호출해서 예약을 해줘야 한다.

	// 리스너에서 RegisterAccept를 호출하면서 Session을 생성한다.
	// 세션은 풀링해서 미리준비된 애를 꺼내써도 되지만, 당장은 동적할당해서 사용한다.
	
	SessionRef session = _service->CreateSession();
	// AcceptEvent에다가 Session을 연결해준다. 그래야지만 나중에 Dispatch를 해서 뽑았을때
	// 어떤 세션을 넘겨줬는지를 알 수 있다.

	acceptEvent->Init();
	acceptEvent->session = session; //acceptEvent에 Session을 연결해준다.

	DWORD bytesReceived = 0;
	// 사실 여기서 중요한 인자는 첫번째 인자인 리슨소켓, 두번째 인자인 세션의 소켓, 세번째 인자인 세션의 수신버퍼, 
	// 마지막 인자인 acceptEvent의 OVERLAPPED 구조체의 포인터이다.
	// IocpEvnet는 모두 OVERLAPPED 구조체를 상속받았기 때문에, acceptEvent를 static_cast로 LPOVERLAPPED로 변환해서 넣어주면 된다.
	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING)
		{
			// AcceptEx가 실패했을 때, ERROR_IO_PENDING이 아닌 경우는 실제로 오류가 발생한 경우이다.
			// 이 경우에는 AcceptEx를 다시 호출해서 예약을 해주어야 한다.
			RegisterAccept(acceptEvent); // 재귀적으로 다시 예약을 해준다.

			// RegisterAccept가 소위 낚시대를 던지는 행위이고, 입질이 와서 고기가 물리면 Dispatch로 실행을 해주게 된다.
			// 근데 여기에 들어왔다는 건 낚시대를 던지다가 미끄러져서 낚시대를 던지는데 실패했다는 의미이다.
			// 그래서 다시 낚시대를 던져야 한다. 그래서 재귀적으로 RegisterAccept를 호출해서 다시 낚시대를 던지게 된다.
		}

		/*
			당장 실행이 되지 않더라도 ERROR_IO_PENDING이면 Completion Port에서 나중에 처리해주기 때문에, 여기서는 아무것도 하지 않아도 된다.
			나중에 뭔가 완료가 되면, IocpCore::Distpatch안에서 GetQueuedCompletionStatus()를 통해서 IocpObject::Dispatch()를 호출하게 되고, 그 안에서 Listener::Dispatch()가 호출되게 된다.
			그러면 다시 리스너의 Dispatch()에서 AcceptEvent를 처리하게 된다. 그러면 AcceptEx가 완료되었으므로, ProcessAccept()를 호출해서 세션을 연결해주게 된다.
		*/
	}
}

// 함수 Body를 보면 알겠지만, 처음 생성해준 AcceptEvent를 계속 재사용하고 있다. 
// AcceptEx가 완료되면, ProcessAccept()에서 다시 RegisterAccept()를 호출해서 AcceptEx를 예약해주고 있다.
void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	// 어떤 세션에서 AcceptEx가 완료되었는지 알 수 있도록 AcceptEvent에서 Session을 가져온다.
	SessionRef session = acceptEvent->session;

	// AcceptEx가 완료되었으므로, 이제 세션의 소켓을 업데이트해주어야 한다.
	// AcceptEx를 호출할 때, 세션의 소켓을 업데이트해주지 않으면, 세션의 소켓은 INVALID_SOCKET 상태로 남아있게 된다.
	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket)) // SetUpdateAcceptSocket()는 AcceptEx를 호출할 때, AcceptEx가 accept한 소켓을 업데이트해주는 옵션이다.
	{
		// 실패하더라도 AcceptEx를 다시 호출해서 예약을 해주어야 한다. 그래야지만 다음 클라이언트의 접속을 받을 수 있다.
		RegisterAccept(acceptEvent); 	
		return;
	}
	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetNetAddress(NetAddress(sockAddress));

	cout << "Client Connected!" << endl;

	// AcceptEx가 완료되었으므로, 다시 AcceptEx를 호출해서 예약을 해주어야 한다. 
	// 그래야지만 다음 클라이언트의 접속을 받을 수 있다.
	// Iocp 특성상 예약을 해주지 않으면, 다음 클라이언트의 접속을 받을 수 없게 된다.
	RegisterAccept(acceptEvent);


}
