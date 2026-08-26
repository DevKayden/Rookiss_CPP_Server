#include "pch.h"
#include "SocketUtils.h"


/*-----------------
	SocketUtils
-----------------*/

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;


void SocketUtils::Init()
{
	WSADATA wsaData;
	// WSAStartup을 통해 WinSock 초기화. 실패하면 의도적 크래시 발생
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0);

	/* 런타임에 주소 얻어오는 API */
	SOCKET dummySocket = CreateSocket();
	// 첫번째 인자인 소켓은 더미 소켓을 넣어주면 된다.
	// 두번째 인자인 GUID는 얻고자 하는 함수의 GUID를 넣어주면 된다.
	// 세번째 인자인 LPVOID* fn은 얻고자 하는 함수 포인터를 넣어주면 된다.

	// ConnectEx, DisconnectEx, AcceptEx 함수 포인터를 얻어오는 과정에서 실패하면 의도적 크래시 발생
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	Close(dummySocket);
	// Bind해줬으니 더미소켓은 닫아준다.
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

// 아래는 소켓 옵션 관련 함수들
// 모두 bool형을 리턴한다. 성공하면 true, 실패하면 false를 리턴한다.
// 아래 함수들은 헤더파일에 만들어준 템플릿 함수를 바디에서 호출한다.

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

// ListenSocket의 특성을 ClientSocket에 그대로 적용
// AcceptEx를 호출할 때, AcceptEx가 accept한 소켓을 업데이트해주는 옵션
bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}



// 아래의 네 가지 함수는 성공여부를 반환하기 때문에 모두 bool형을 리턴한다.


bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	// INADDR_ANY는 바인딩하는 주소를 os가 알아서 선택하도록 하는 상수이다.
	// 즉, 서버가 여러개의 네트워크 인터페이스를 가지고 있을 때, 
	// 어떤 인터페이스를 통해 들어오는 연결이든지 상관없이 바인딩을 하겠다는 의미이다.
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	// bind 함수는 소켓에 특정 주소를 바인딩하는 함수이다.
	// 실패하면 SOCKET_ERROR를 반환하기에 조건문을 통해 성공 여부를 판단한다.
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

// 그냥 listen을 호출하는 함수. backlog는 기본값으로 SOMAXCONN을 넣어준다.
// 디폴트 파라미터를 넣어주면, listen을 호출할 때 backlog를 생략하면 SOMAXCONN이 들어가게 된다.
bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

void SocketUtils::Close(SOCKET& socket)
{
	// 두번 닫는 경우를 방지하기 위해 INVALID_SOCKET로 초기화 후, 닫기 전에 INVALID_SOCKET인지 확인
	if (socket != INVALID_SOCKET)
		::closesocket(socket);
	socket = INVALID_SOCKET;
}