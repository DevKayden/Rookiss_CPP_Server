#pragma once
#include "NetAddress.h"

/*-----------------
	SocketUtils
-----------------*/

/*
	소켓 유틸이 해주는 역할

	WinSock 초기화
	비동기 connect, accept 등의 전처리

	이 SocketUtils도 어딘가에서는 Init과 Clear가 호출되어야 한다. 
	그건 전역 매니저를 관리하는 클래스인 CoreGlobal에서 호출해준다.

	!중요!
	어지간해서는 이런식으로 함수들을 매핑해서 사용하는 것이 좋음
	어차피 자주 사용하는 건 아니기에 인자들을 모두 외우는 대신에 진짜 필요한 인자만 넣어서
	사용할 수 있게 래핑을 해주는 것이기 때문이다.

*/
class SocketUtils
{
public:
	// 각각 ConnectEx, DisconnectEx, AcceptEx 함수를 호출하기 위해서는
	// WSAIoctl 함수를 통해서 함수 포인터를 얻어와야 한다.
	// ConnectEx, DisconnectEx, AcceptEx 함수 포인터를 얻어오는 함수
	 
	// 아래는 각 함수별 포인터이다. WSAIoctl 함수를 통해서 런타임에 아래 포인터에 값을 넣어주어야 한다.
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket(); // 소켓 생성을 위한 함수.

	// 아래는 소켓 옵션 관련 함수들이다. 
	// 소켓 옵션을 설정하는 함수들은 모두 bool형을 리턴한다.

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag); //네이글 알고리즘 관련
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket); // AcceptEx를 호출할 때, AcceptEx가 accept한 소켓을 업데이트해주는 옵션


	// 아래의 네 가지 함수는 성공여부를 반환하기 때문에 모두 bool형을 리턴한다.

	// 소켓에다가 특정 IP주소를 바인딩하는 함수.
	static bool Bind(SOCKET socket, NetAddress netAddr);
	// 소켓에다가 임의의 IP주소를 바인딩하는 함수. but 포트는 지정해줘야하니 인자로 받는다.
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);

};

/*
	아래 템플릿 함수는 소켓 옵션을 설정하는 함수이다.
	옵션 설정하는 게 하나의 함수로만 작동 가능해서 인자만 다르게 주면 되기 때문에
	템플릿으로 만들어서 재사용성을 높였다.

	위에서 소켓 옵션 관련 함수들은 결국 body에서 아래의 템플릿 함수인, SetSockOpt을 호출한다.
*/

template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}