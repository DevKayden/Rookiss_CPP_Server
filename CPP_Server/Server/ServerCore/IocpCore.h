#pragma once

/*----------------
	IocpObject
----------------*/
// IocpObject를 상속을 해서 다른 클래스에서 사용하도록 설계
// 사실상 IocpObject의 역할이 Session의 역할이라는데. 

/*
		IocpEvent는 뭘까?

		이전에 Part3에서 IOCP를 다루었던 걸 생각해보면, 크게 두가지의 데이터를 넣어줬었음.

		첫 번째: 소켓을 등록할 때 넣어주는 키값 하나 -> CreateIoCompletionPort() 에서 인자로 넘김
		두 번쨰: Overlapped 구조체를 상속받은 구조체 하나 -> WSARecv(), WSASend(), GetQueuedCompletionStatus() 등에서 사용

		즉, 이렇게 정보를 두 개 건네줄 수 있었음.

		이제부터는 아래의 두가지를 건네줄거임.

		첫 번째: IocpObject를 상속받은 클래스
		두 번째: Overlapped 구조체를 상속을 받은 IocpEvent

		IocpEvent 내부에서 이게 Read인지 Write인지 구분을 해준다.
	*/

class IocpObject
{
public:
	// IocpObject를 상속받은 클래스는 반드시 GetHandle()과 Dispatch()를 구현해야 한다.
	// GetHandle()은 IocpObject가 관리하는 소켓의 핸들을 반환한다.
	virtual HANDLE GetHandle() abstract;
	// 이제 iocpEvent가 어떤 type인지에 따라서 구분해서 처리를 해주게 된다.
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
	
};

/*----------------
	IocpCore
----------------*/

class IocpCore
{
public:
	// 생성자에서 Completion Port를 생성하고, 소멸자에서 Completion Port를 닫아준다.
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() const { return _iocpHandle; }

	bool Register(class IocpObject* iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);
private:
	HANDLE _iocpHandle;
};

// TEMP 이건 임시라서 여기에 만들어준거고 나중에는 여기서 관리하지 않을 거임.
extern IocpCore GIocpCore;