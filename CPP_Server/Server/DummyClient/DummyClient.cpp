#include "pch.h"
#include <iostream>

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void HandleError(const char* cause)
{
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;

}

int main()
{
    //WinSock 라이브러리를 사용하려면, 라이브러리 초기화를 해줘야한다.
    /*
        시작에는 WSAStartup()을 호출해서 초기화
        두번째 인자인 WSAData에 관련 정보가 들어가긴 하는데, 쓸 일은 없다고 함.
        성공하면 0을 반환, 실패하면 0이 아닌 값을 반환.
        
        WSACleanup()은 마지막에 호출해줘야한다. 근데 호출안한다고 크래시가 나거나 큰 문제는 생기지 않음.
        그냥 표준에서 호출하라니까 호출하는거.
    */
    // WinSock 초기화 (ws2_32 라이브러리 초기화)
    // 관련 정보가 wsaData에 채워짐
    WSAData wasData;
    if (::WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        return 0;

    
    // ad : Address Family (AF_INET = IPv4, AF_INET6 = IPv6)
    // type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
    // protocol : 0
    // return : descriptor
    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    // SOCKET 타입 자체가 Int형이다. 해당 번호에 해당하는 소켓을 이용하라고 OS에 요청하는거.

    if (clientSocket == INVALID_SOCKET) // Socket생성 실패시
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode : " << errCode << endl;
        return 0;
    }


    
    
    // 소켓 리소스 반환
    ::closesocket(clientSocket);

    // WinSock 종료
    ::WSACleanup();
}

