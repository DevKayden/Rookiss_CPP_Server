#include "pch.h"
#include "IocpEvent.h"


/*----------------
	IocpEvent
----------------*/

IocpEvent::IocpEvent(EventType type) : _type(type)
{
	Init();
}

void IocpEvent::Init()
{
	// OVERLAPPED 구조체 초기화
	// 모든 값을 0으로 초기화
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
	// OVERLAPPED 구조체의 멤버변수들을 초기화하는 이유는, 
	// IOCP에서 Overlapped 구조체를 사용할 때, 이전 작업의 상태가 남아있으면 문제가 발생할 수 있기 때문임. 
	// 따라서 새로운 작업을 시작하기 전에 항상 초기화하는 것이 안전함.
	// 참고로 hEvent는 오버랩드 모델의 이벤트 기반일때 우리가 직접 연동시켜주긴 하지만
	// IOCP에서는 사용하지 않으므로 0으로 초기화해도 됨.
	// 나머지 값들은 애초에 우리가 쓸 일도 없고, OS가 내부적으로 쓰는 값이므로 건드릴 일이 없음.
}
