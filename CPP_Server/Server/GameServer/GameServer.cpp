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


int main()
{
    // WinSock 초기화 (ws2_32 라이브러리 초기화)
    // 관련 정보가 wsaData에 채워짐
    WSAData wasData;
    if (::WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
        return 0;


    // ad : Address Family (AF_INET = IPv4, AF_INET6 = IPv6)
    // type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
    // protocol : 0
    // return : descriptor
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    // SOCKET 타입 자체가 Int형이다. 해당 번호에 해당하는 소켓을 이용하라고 OS에 요청하는거.

    if (listenSocket == INVALID_SOCKET) // Socket생성 실패시
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode : " << errCode << endl;
        return 0;
    }


    // 서버 주소 설정 (IP주소 + 포트)
    SOCKADDR_IN serverAddr; //IPv4
    ::memset(&serverAddr, 0, sizeof(serverAddr)); // SOCKADDR_IN 구조체변수 값을 0으로 초기화
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // 니가 알아서 골라달라는 의미.
    // 주소를 하나로 지정하면 그 주소로만 되지만, INADDR_ANY로 하면 LoopBack주소도 되고 ipconfig에 나오는 주소로도 연결가능.
    serverAddr.sin_port = ::htons(7777); // port설정. 서버쪽에서 열어놓은 포트 번호로 지정되어야함. 

    

    // 안내원 폰 개통! 식당의 대표 번호
    // 소켓에다가 주소를 묶어주는 것, 첫번째 파라미터인 소켓에 두번재 파라미터인 서버 주소를 연동시키는 것임.
    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Bind ErrorCode : " << errCode << endl;
        return 0;
    }

    // 영업 시작 -> 입장가능
    // 두번째 파라미터인 backlog는 대기열 같은 거임. 꽉찬 경우에 대기열의 최대한도라고 볼 수 있음. 
    // 이걸 초과하는 애들은 바로 끊어버리는 거지.
    if (::listen(listenSocket, 10) == SOCKET_ERROR)
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Listen ErrorCode : " << errCode << endl;
        return 0;
    }



    /*----------------------------------------------------------------------
        여기까지 왔으면 클라에서 connect를 해서 붙는다면, 처리할 준비가 된 것이다.
    ----------------------------------------------------------------------*/

    while (true)
    {
        SOCKADDR_IN clientAddr; //IPv4
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrlen = sizeof(clientAddr);

        // 두번째 파라미터의 SOCKADDR은 연결되는 클라리언트의 SOCKET 주소이다.
        /*
            accept함수는 SOCKET을 리턴하는데, 앞으로 클라이언트와 데이터 통신을 할때는 이 소켓을 이용하면 된다.
            위에서 사용한 리슨소켓은 클라이언트와 연결해서 새로운 소켓을 받을 수 있도록 문지기 역할을 하는 거지.
        */
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrlen);
        if (clientSocket == INVALID_SOCKET)
        {
            int32 errCode = ::WSAGetLastError();
            cout << "Accept ErrorCode : " << errCode << endl;
            return 0;
        }

        // 손님 입장 -> 클라 연결됨
        char ipAddress[16];
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        cout << "Client Connected! IP = " << ipAddress << endl;


        //TODO
        while (true)
        {
            // sendBuffer와는 다르게 recvBuffer는 데이터가 얼마나 들어올지 모르기에 크게 잡아야한다.
            // recv()가 반환하는 것은 실제로 받은 바이트의 크기를 int로 받아준다. -1을 리턴하면 SOCKET_ERROR이다.
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= 0)
            {
                int32 errCode = ::WSAGetLastError();
                cout << "Recv ErrorCode : " << errCode << endl;
                return 0;
            }

            cout << "Recv Data! Data = " << recvBuffer << endl;
            cout << "Recv Data! Len = " << recvLen << endl;
            
            // 위에서 recvBuffer에 데이터를 받고, 아래에서 다시 해당 데이터를 그대로 클라로 보내기

            int32 resultCode = ::send(clientSocket, recvBuffer, recvLen, 0);
            if (resultCode == SOCKET_ERROR)
            {
                int32 errCode = ::WSAGetLastError();
                cout << "Send ErrorCode : " << errCode << endl;
                return 0;
            }

        }


    }



    // WinSock 종료
    ::WSACleanup();
}