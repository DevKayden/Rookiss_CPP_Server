#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

class Service;

/*----------------
	Session
----------------*/

/*
	클라이언트 소켓부터 클라이언트의 정보는 모두 Session클래스에서 관리한다.
	세션도 IocpObject를 상속받고, IocpCore에 등록해서 유심히 지켜보게 된다.

	Listener를 통해서 accept가 되면, 리시브를 걸어서 패킷을 받을 준비를 해야한다.

*/

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:
						/* 외부에서 사용 */
	void				Send(BYTE* buffer, int32 len);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);

	shared_ptr<Service>	GetService() { return _service.lock(); }
	void				SetService(shared_ptr<Service> service) { _service = service; }

public:
						/* 정보 관련 */
	void				SetNetAddress(NetAddress netAddress) { _netAddress = netAddress; }
	NetAddress			GetNetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }


private:
						/* 인터페이스 구현 */
	// 리스너 소켓처럼 Session도 IocpObject를 상속받았기 때문에 GetHandle()과 Dispatch()를 구현해야 한다.
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;


private:
						/* 전송 관련 */
	bool				RegisterConnect(); // 지금 당장은 안쓴다. 클라서비스로 동작을 하게 되면 ConnectEx함수를 통해서 IOCP에 등록하는 함수
	bool				RegisterDisconnect();
	void				RegisterRecv(); 
	void				RegisterSend(SendEvent* sendEvent);

	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

	void				HandleError(int32 errorCode);


protected:
						/* 컨텐츠 코드에서 오버로딩해서 사용할 함수들 */
	virtual void		OnConnected() {}
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) {}
	virtual void		OnDisconnected() {}


public:
	//TEMP 임시로 리시브버퍼를 여기서 대충 만든다.
	BYTE _recvBuffer[1000];


private:
	weak_ptr<Service>	_service; // 내부적으로 서비스의 존재를 알아야지만 서비스에 등록할 수 있으니, 들고 있음. 순환을 줄이기 위해서 weakptr이용
	SOCKET			_socket = INVALID_SOCKET; // 클라이언트 소켓
	NetAddress		_netAddress = {}; // 클라이언트 주소
	Atomic<bool>	_connected = false; // 클라이언트 접속 여부

private:
	USE_LOCK;

	/* 수신 관련 */

	/* 송신 관련 */

private:
						/* IocpEvent 재사용 */
	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
};

