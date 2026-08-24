#include "pch.h"
#include "NetAddress.h"


/*--------------
	NetAddress
---------------*/

NetAddress::NetAddress(SOCKADDR_IN sockAddr) : _sockAddr(sockAddr)
{
}

NetAddress::NetAddress(wstring ip, uint16 port)
{
	// 0으로 밀어주기
	::memset(&_sockAddr, 0, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET; // IPv4
	_sockAddr.sin_addr = Ip2Address(ip.c_str()); // 문자열로 받은 IP주소를 IN_ADDR 구조체로 변환
	_sockAddr.sin_port = ::htons(port); // 포트번호를 네트워크 바이트 순서로 변환
}

wstring NetAddress::GetIpAddress()
{
	WCHAR buffer[100];
	::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, len32(buffer));

	/*
		::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, sizeof(buffer));
		마지막 인자를 저렇게 주면 100이 아니라 WCHAR의 크기인 2바이트 단위로 계산되기 때문에 200이 되어버림.

		::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, sizeof(buffer) / sizeof(WCHAR));
		이렇게 주면 100이 되긴 하지만, sizeof(buffer) / sizeof(WCHAR)로 계산하는 것보다
		매크로를 만들어서 len32(buffer)로 계산하는게 더 직관적이고 가독성이 좋음.
		해당 매크로 정의는 Types.h에 있음.
	*/
	return wstring(buffer);
}

IN_ADDR NetAddress::Ip2Address(const WCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
