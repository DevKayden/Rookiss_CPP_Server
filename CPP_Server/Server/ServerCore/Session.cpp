#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

/*----------------
	Session
----------------*/


Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
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
}