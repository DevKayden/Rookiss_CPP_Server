#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

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
    // WinSock 초기화 (ws2_32 라이브러리 초기화)
    // 관련 정보가 wsaData에 채워짐
    WSAData wasData;
    if (::WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        return 0;
    
    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    // SOCKET 타입 자체가 Int형이다. 해당 번호에 해당하는 소켓을 이용하라고 OS에 요청하는거.

    // TCP와 크게 다른 점은 서버소켓이 하나라는 점이다.
    if (serverSocket == INVALID_SOCKET) // Socket생성 실패시
    {
        HandleError("Socket");
        return 0;
    }


    SOCKADDR_IN serverAddr; //IPv4
    ::memset(&serverAddr, 0, sizeof(serverAddr)); // SOCKADDR_IN 구조체변수 값을 0으로 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);


    if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        HandleError("Bind");
        return 0;
    }

    /*
        TCP로 만들면 bind -> listen -> accept 흐름으로 진행되지만
        UDP는 bind까지 하면 준비가 끝난다.
    */

    while (true)
    {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);


        char recvBuffer[1000];

        // 실질적으로 받은 패킷의 바이트 수를 반환. 0이하로 떨어지면 문제가 있는 것
        // clientAddr에는 나한테 데이터를 보낸 클라의 주소가 들어오게 된다.
        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0,
            (SOCKADDR*)&clientAddr, &addrLen);

        if (recvLen <= 0)
        {
            HandleError("RecvFrom");
            return 0; // 실제로는 서버를 닫는게 아니라 해당하는 클라이언트를 종료시켜줘야겠지.
        }

        cout << "Recv Data! Data = " << recvBuffer << endl;
        cout << "Recv Data! Len = " << recvLen << endl;

        int32 errorCode = ::sendto(serverSocket, recvBuffer, recvLen, 0,
            (SOCKADDR*)&clientAddr, sizeof(clientAddr));

        if (errorCode == SOCKET_ERROR)
        {
            HandleError("SendTo");
            return 0; // 원래는 프로그램 종료 대신 해당하는 클라를 종료해줘야한다.
        }

        cout << "Send Data! Len = " << recvLen << endl;


    }



    // WinSock 종료
    ::WSACleanup();
}