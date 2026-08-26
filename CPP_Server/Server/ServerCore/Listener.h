#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

/*----------------
	Listener
----------------*/

// 	Listener 클래스는 AcceptEx를 이용해서 비동기적으로 클라이언트의 접속을 받아주는 역할을 한다.
// 사실상 문지기 같은 역할

/*
	Listener는 IocpObject를 상속받고, IocpCore에 등록해서 유심히 지켜보게 된다.
*/
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/* 외부에서 사용 */
	// 리스너가 리슨을 시작하도록 하는 함수
	bool StartAccept(NetAddress netAddress);
	void CloseSocket();
	

public:
	/* 인터페이스 구현 */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 수신 관련 코드 */
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET; // 리슨 소켓
	Vector<AcceptEvent*> _acceptEvents;


};

