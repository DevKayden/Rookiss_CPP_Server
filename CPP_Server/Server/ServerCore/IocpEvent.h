#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Accept,
	//PreRecv라는 리시브 전 단계에서 0byte를 이용하는 고급 기법을 쓰는게 있다는데 할건지 모른다고 함.
	Recv,
	Send,
};


/*----------------
	IocpEvent
----------------*/
/*
	IocpEvent는 Overlapped 구조체를 상속받은 구조체임.
	첫번째 멤버변수로 들고 있어도 되고, 상속을 받아도 된다.
	여기서는 상속을 받는 방식으로 구현

*/ 

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void Init();

public:
	EventType eventType;
	IocpObjectRef owner; // 이 IocpEvent를 소유한 IocpObject를 가리키는 포인터


};

// 아래에서 부터는 IocpEvent를 상속받은 클래스들이다.


/*----------------
	ConnetEvent
----------------*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { }
};


/*----------------
	AcceptEvent
----------------*/

/*
	리스너에서 RegisterAccept를 호출할때 AcceptEvent를 인자로 받아준다.
	RegisterAccept를 호출하면, 그안에서 Session을 생성하고, AcceptEvent에 Session을 연결해준다. 
	그래야지만 나중에 Dispatch를 해서 뽑았을때 어떤 세션인지 알 수 있다.

	즉 IocpEvent는 Overlapped 구조체를 상속받은 구조체이고, IocpObject를 shared_ptr로 들고 있는 구조체이다.
	AcceptEvent는 IocpEvent를 상속받은 구조체이며, Session을 포인터로 들고 있는 구조체이다.
*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }


public:
	// AcceptEvent가 어떤 Session을 연결했는지 알 수 있도록 Session 포인터를 들고 있음.
	SessionRef session = nullptr; 
};


/*----------------
	RecvEvent
----------------*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { }
};


/*----------------
	SendEvent
----------------*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { }

	//TEMP
	vector<BYTE> buffer;
};