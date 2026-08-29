#pragma once

/*----------------
	NetAddress
----------------*/

/*
	NetAddress는 SOCKADDR_IN을 이용해서 주소를 받아준다. 근데 나중에 클라이언트가 접속해서
	서버를 운영하다보면 클라이언트의 주소가 필요한 경우가 있음. 그럴때마다 함수를 호출해서 추출하기 보다는
	모든 주소를	NetAddress에 저장해두고 필요할때마다 꺼내서 쓰는게 좋을것 같음.
*/

class NetAddress
{
public:
	// 생성자는 여러가지 방법으로 생성할 수 있도록 오버로딩을 해주었다.
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(wstring ip, uint16 port);

	SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
	wstring			GetIpAddress();
	uint16			GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
	// IP주소를 문자열로 받아서 IN_ADDR 구조체로 변환해주는 함수
	static IN_ADDR	Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN		_sockAddr = {};
};

