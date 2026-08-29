#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

/*-------------
	Service
--------------*/

Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: _type(type), _netAddress(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{

}

Service::~Service()
{
}

void Service::CloseService()
{
	// TODO
}

// 세션을 만들고, IocpCore에 등록을 해준다.
SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();

	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

// Set을 공용으로 사용하기 때문에 Lock을 잡아주는 것 같음.
void Service::AddSession(SessionRef session)
{
	// WriteLock을 잡고 세션카운트 증가, Set에 넣어주기
	WRITE_LOCK;
	_sessionCount++;
	_sessions.insert(session);
}

// Set을 공용으로 사용하기 때문에 Lock을 잡아주는 것 같음.
void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0); // 삭제했는데 문제가 일어났으면 크래시
	_sessionCount--; // 세션 카운트 감소
}

/*-----------------
	ClientService
------------------*/

ClientService::ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Client, targetAddress, core, factory, maxSessionCount)
{
}

bool ClientService::Start()
{
	// TODO
	return true;
}


/*-----------------
	ServerService
------------------*/

ServerService::ServerService(NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Server, address, core, factory, maxSessionCount)
{
}

// 여기서 리스너를 만들어준다. 이전처럼 GameServer.cpp인 콘텐츠 단에서 만들어주는 게 아니라.
// 만들어서 멤버로 가지고 있는 거지
bool ServerService::Start()
{
	if (CanStart() == false) // 아직 준비가 덜 된거인 false리턴
		return false;

	// 준비가 되었으면 여기서 리스너를 만들어준다.
	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;


	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	
	if (_listener->StartAccept(service) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{
	// TODO

	Service::CloseService();
}