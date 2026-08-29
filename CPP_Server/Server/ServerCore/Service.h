#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8
{
	Server,
	Client
};

/*-------------
	Service
--------------*/

using SessionFactory = function<SessionRef(void)>;

class Service : public enable_shared_from_this<Service>
{
public:

	Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~Service();

	// 서비스를 상속받은 클래스마다 각자 구현해줘야함. 
	virtual bool		Start() abstract;
	bool				CanStart() { return _sessionFactory != nullptr; }

	// 서비스를 상속받은 클래스마다 각자 구현해줘야함. 
	virtual void		CloseService();
	// 생성자로 받아주었긴 하지만 중간에 바꿀 수도 있으니 추가
	void				SetSessionFactory(SessionFactory func) { _sessionFactory = func; }


	// 세션을 새로 만들어주면서 IocpCore에 세션을 등록해주는 함수
	SessionRef			CreateSession();
	// 이미 만들어진 세션을 추가하는 함수
	void				AddSession(SessionRef session);
	// 세션을 꺼내기. 더이상 사용하지 않는 경우에 사용
	void				ReleaseSession(SessionRef session);
	// 현재 동접자 수 확인
	int32				GetCurrentSessionCount() { return _sessionCount; }
	// 최대 동접자 가능수 확인
	int32				GetMaxSessionCount() { return _maxSessionCount; }


public:

	// 서비스 타입 뱉어주기
	ServiceType			GetServiceType() { return _type; }
	NetAddress			GetNetAddress() { return _netAddress; }
	// 참조로 넘긴다. 굳이 외부에서 꺼내서 레퍼런스 카운트를 늘리기보다 참조로 넘기는게 나음.
	IocpCoreRef& GetIocpCore() { return _iocpCore; }


protected:

	USE_LOCK;
	ServiceType			_type;
	NetAddress			_netAddress = {};
	IocpCoreRef			_iocpCore;

	Set<SessionRef>		_sessions;
	int32				_sessionCount = 0;
	int32				_maxSessionCount = 0;
	SessionFactory		_sessionFactory;
};

/*-----------------
	ClientService
------------------*/

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool	Start() override;

};

/*-----------------
	ServerService
------------------*/

class ServerService : public Service
{
public:
	ServerService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService() {}

	virtual bool	Start() override;
	virtual void	CloseService() override;

private:
	ListenerRef _listener = nullptr;
};