#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

/*----------------
	Session
----------------*/

/*
	클라이언트 소켓부터 클라이언트의 정보는 모두 Session클래스에서 관리한다.

	세션도 IocpObject를 상속받고, IocpCore에 등록해서 유심히 지켜보게 된다.
*/

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	/* 정보 관련 */
	void SetNetAddress(NetAddress netAddress) { _netAddress = netAddress; }
	NetAddress GetNetAddress() { return _netAddress; }
	SOCKET GetSocket() { return _socket; }

public:
	/* 인터페이스 구현 */
	// 리스너 소켓처럼 Session도 IocpObject를 상속받았기 때문에 GetHandle()과 Dispatch()를 구현해야 한다.
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	//TEMP 임시로 리시브버퍼를 여기서 대충 만든다.
	char _recvBuffer[1000];

private:
	SOCKET			_socket = INVALID_SOCKET; // 클라이언트 소켓
	NetAddress		_netAddress = {}; // 클라이언트 주소
	Atomic<bool>	_conneted = false; // 클라이언트 접속 여부

};

