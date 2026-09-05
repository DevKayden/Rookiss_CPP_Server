#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*----------------
	Session
----------------*/


Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	// 생각할 문제
	// 1) 버퍼 관리?
	// 2) sendEvent 관리? 단일? 여러개? WSASend 중첩?

	// TEMP
	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this(); // ADD_REF
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	// 기존의 값이 false면, 실행하지 않고 리턴.
	// 기존의 값이 true면 Disconnect실행
	if (_connected.exchange(false) == false)
		return;

	// TEMP
	// WCHAR라서 wcout이라고 한다.
	wcout << "Disconnect : " << cause << endl;

	OnDisconnected(); // 컨텐츠 코드에서 오버로딩
	GetService()->ReleaseSession(GetSessionRef()); // 서비스에서 해당 세션 날리기
	
	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	// GetHandle()은 IocpObject가 관리하는 소켓의 핸들을 반환한다.
	// reinterpret_cast를 사용해서 SOCKET을 HANDLE로 변환한다.
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	// TODO : iocpEvent가 어떤 type인지에 따라서 구분해서 처리를 해주게 된다.

	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;
	default:
		break;

	}


}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	// 서비스가 ClientService일때만 Connect를 등록할거니까 처리
	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0/*남는거*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this(); // ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this(); // ADD_REF

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;
}

/*
	커넥트가 되었으면 리스브를 먼저 걸어줘야한다. 즉, 리시브를 걸어주는 함수이다.
*/
void Session::RegisterRecv()
{
	// 어떤 사유로 연결이 끊겼으면 리턴
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	// OVERLAPPED구조체를 상속받은 RecvEvnet를 넣어준다.
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; // RELEASE_REF
			// pending이 아니면, Iocp에 성공적으로 완료 통지가 뜨지 않아서, 여기서 처리하지 않으면
			// 레퍼런스 카운트가 영영 줄어들지 않아서 이 세션을 영영 사라지지 않는 메모리 릭 현상을 일으킨다.
		
			/*
				이후에는 리시브가 완료되면, workerThread들이 GetQueuedCompletionStatus()에서 대기하고 있다,
				Session::Dispatch로 넘어오게 된다. 우리가 여기서 직접적으로 호출해주지 않아서 흐름이 잘 안보일 수 있지만,
				내부적으로 iocp가 그렇게 동작한다. 
			*/
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false) // 연결상태 체크
		return;

	// WSASend할때 사용해야해니까 선언하고 값을 채워준다.
	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			sendEvent->owner = nullptr; //RELEASE_REF
			xdelete(sendEvent);
		}
	}
	// 성공을 하면 IOCP에 등록이 되고, 완료가 되면 IocpCore::Dispatch에서 Session::Dispatch를 호출
	// 그리고 ProcessSend가 호출되는 흐름.
}


void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr; // RELEASE_REF

	_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버라이딩해서 사용하면 꽤 중요한 역할을 하게 될 것이다.
	OnConnected(); // 현재는 별 역할을 하지 않는다.

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr; // RELEASE_REF
}

void Session::ProcessRecv(int32 numOfBytes)
{
	// 여기 들어왔다는 건 예약이 걸려있는 리시브가 없으니까 레퍼런스 카운트 감소한다.
	_recvEvent.owner = nullptr; // RELEASE_REF
	// 이거 안해주면 세션이 들고 있는 RecvEvent의 레퍼런스 카운트가 감소하지 않아서
	// 클라이언트를 종료해도 세션 소멸자에 브레이크 포인트가 잡히지 않는다.

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	// 예외처리 및 OnRecv호출
	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize); // 컨텐츠 코드에서 재정의
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	// 커서 정리
	_recvBuffer.Clean();

	// 수신 등록
	// 리시브에 대한 처리를 했으니, 다시 리시브 등록
	// 물고기가 낚였으니, 다시 미끼 달아서 던져놓는 느낌.
	RegisterRecv();
}


void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr; // RELEASE_REF
	xdelete(sendEvent);

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	// 컨텐츠 코드에서 오버로딩
	OnSend(numOfBytes);
}



void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		// 콘솔에 로그를 찍는 것도 결국에는 쓰레드의 컨텍스트 스위칭이 필요하기 때문에
		// 로그를 찍는 전문 쓰레드한테 맡기도록 만드는 경우가 많음. 일단은 킵
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}