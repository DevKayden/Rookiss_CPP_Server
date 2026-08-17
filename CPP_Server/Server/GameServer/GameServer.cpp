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
    
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) // Socket생성 실패시
    {
        HandleError("Socket");
        return 0;
    }

    // 블로킹(blocking) 소켓
    // accept->접속한 클라가 있을 때
    // connet->서버 접속 성공했을 때
    // send, sendto->요청한 데이터를 송신 버퍼에 복사했을 때
    // recv, recvfrom->수신 버퍼에 도착한 데이터가 있고, 이를 유저레벨 버퍼에 복사했을 때

    // ioctlsocket()함수
    // 논블로킹 방식으로 바꾸기 위해서 사용하는 함수이다.

    u_long on = 1;
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;


    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);


    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    // 여기까지는 블로킹과 논블로킹이 다른 것 없지만, 아래부터 달라지게 된다.
    
    SOCKADDR_IN clientAddr;
    int32 addrLen = sizeof(clientAddr);

    // 여기서 달라지는 점
        /*
            블로킹 방식에서는 accept이 블로킹 방식이라서 접속한 클라가 있을때 리턴을 되기에,
            리턴 값이 INVALID_SOCKET이면 문제 상황이 맞다.

            하지만 논블로킹 방식을 한다면, accept이 블로킹 방식이 아니라서 그냥 빠져나와서
            INVALID_SOCKET이 되어도 문제가 없는 상황일 수 있는 거다.

            그래서 이중 조건문을 사용해서 처리한다.
            일단 accept이 논블로킹 방식이므로 accept이 성공할때까지 계속해서 accept을 호출해줄 것이고,
            두번째로는 LastError가 "WSAEWOULDBLOCK"이라면 문제가 아니기에 continue를 해서 다시 반복문을 반복하게 한다.
        */

    while (true)
    {
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			// 원래 블록했어야 했는데... 너가 논블로킹으로 하라며?
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
				continue;

			// Error
			break;
		}

        cout << "Client Connected!" << endl;


        // Recv
        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

            // 여기서도 같은 문제가 발생
            if (recvLen == SOCKET_ERROR)
            {
                // WSAEWOULDBLOCK이라는 에러를 뱉은 것이면 아직 상대쪽에서 보내지 않은 것이므로 문제 상황은 아님
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    continue;
                }

                // Error
                break;
            }
            else if (recvLen == 0)
            {
                // 연결이 끊긴 거니까
                break;
            }

            cout << "Recv Data! Len = " << recvLen << endl;

            //Send
            while (true)
            {
                if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
                {
                    if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    {
                        continue;
                    }
                    //Error
                    break;
                }

                cout << "Send Data! Len = " << recvLen << endl;
                break;
            }

        }
    }

    



    



    // WinSock 종료
    ::WSACleanup();
}