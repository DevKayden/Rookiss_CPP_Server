#include "pch.h"
#include <iostream>

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

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
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    // SOCKET 타입 자체가 Int형이다. 해당 번호에 해당하는 소켓을 이용하라고 OS에 요청하는거.

    if (clientSocket == INVALID_SOCKET) // Socket생성 실패시
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket ErrorCode : " << errCode << endl;
        return 0;
    }


    // 연결할 목적지 설정 (IP주소 + 포트)
    SOCKADDR_IN serverAddr; //IPv4
    ::memset(&serverAddr, 0, sizeof(serverAddr)); // SOCKADDR_IN 구조체변수 값을 0으로 초기화
    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); << deprecated 오래된거라 사용 권장 x 대신 아래코드로 대체
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // 127.0.0.1 이 루프백 주소라서 우린 한 컴퓨터에서 클라-서버로 테스트 하니까
    serverAddr.sin_port = ::htons(7777); // port설정. 서버쪽에서 열어놓은 포트 번호로 지정되어야함. 
    
    // htons : host to network short -> host에서 network방식의 Endian으로 맞춰주는 함수이다.
    // Big-Endian vs Little-Endian 방식이 다를 수 있어서 이 함수를 통해서 포트번호를 넣어주는 것이다.
    // network에서는 Big-Endian을 사용하기 때문에 거기에 맞춰주는 듯.


    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Connect ErrorCode : " << errCode << endl;
        return 0;
    }

    /*----------------------------------------------------------------------
        여기까지 왔으면 연결이 성공함. 이제부터 데이터 송수신이 가능하다. 
    ----------------------------------------------------------------------*/

    cout << "Connected To Server!" << endl;

    while (true)
    {
        //TODO

        this_thread::sleep_for(1s);
    }
    
    // 소켓 리소스 반환
    ::closesocket(clientSocket);

    // WinSock 종료
    ::WSACleanup();
}

