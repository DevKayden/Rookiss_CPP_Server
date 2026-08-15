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

    // -------------------- 소켓 옵션 관련 이론 시작 --------------------

    // setsockopt함수의옵션 두번째 파라미터
    // 옵션을 해석하고 처리할 주체? 소켓인지, IP인지, 프로토콜인지에 따라 아래와 같음
    // 소켓 코드 -> SOL_SOCKET
    // IPv4 -> IPROTO_IP
    // TCP 프로토콜 -> IPPROTO_TCP

    // SO_KEEPALIVE = 주기적으로 연결 상태 확인 여부 (TCP ONLY) UDP는 연결이 없으니까
    // 상대방이 소리소문없이 연결을 끊는지, 아니면 데이터를 보내지 않는 건지 구분하기 위해서 사용
    // 주기적으로 TCP 프로토콜 연결 상태 확인 -> 끊어진 연결 감지

    // 참고 : 세번째 파라미터인 optionname마다 네번째와 다섯번째 파라미터인 optval과 optlen이 다르다.
    // 어떤 자료형인지도 다르기 때문에 MS에 관련 문서를 optionname에 해당하는 value에 맞는 변수를 선언해서 char*로
    // 캐스팅해서 파라미터를 넣어주면 된다.

    bool enable = true;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

    // -------------------- 구분선 --------------------
    
    // SO_LINGER = 지연하다

    // 소켓 리소스 반환
    // closesocket을 하면 상대쪽에서 더이상 데이터를 주고받을 수가 없게 된다. 소켓을 닫아주었기 때문이다.
    // 만약 send 후에 바로 closesocket을 하면, 예약이 된 패킷이 커널 버퍼에 있을텐데, 전송중일 텐데 나머지 패킷을
    // 다받고 끊을지, 아니면 그냥 안 받고 버릴 것인지 정할 수 있음.

    /*
     Linger 구조체의 실제 정의
     
    struct  linger {
        u_short l_onoff;                // option on/off
        u_short l_linger;               // linger time 
    };

    */
    
    // 송신 버퍼에 있느 ㄴ데이터를 보낼 것인가? 날릴 것인가? 를 링거를 통해 옵션을 줄 수 있다.
    // onoff = 0이면 closesocket()이 바로 리넡, 아니면 linger초 만큼 대기 (default 0)
    // linger : 대기 시간

    LINGER linger;
    linger.l_onoff = 1;
    linger.l_linger = 5;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&linger, sizeof(linger));

    // -------------------- 구분선 --------------------

    // 사실 closesocket으로 소켓을 닫아버리는 것보다 좀 더 선택적인 방법이 있다.
    // shutdown이라는 함수를 사용한다.
    // Half-Close
    // SD_SEND : send만 막는다.
    // SD_RECEIVE : recv만 막는다
    // SD_BOTH : 둘다 막는다
    //::shutdown(serverSocket, SD_SEND);

    /*
        정석적인 방법은 매너없이(?) 소켓을 닫아버리는 것보다 shutdown함수로 상대방한테 데이터를 전송받을
        또는 send, recv받을 의사가 없음을 먼저 알리는게 맞음

        이게 이론적으로 맞긴 하지만 안 지킨다고 문제가 생기는 건 아님. 그냥 이론적으로 이게 정석이라는 의미.
    */

    //::closesocket(serverSocket);

    // -------------------- 구분선 --------------------

    // SO_SNDBUF = 송신 버퍼 크기
    // SO_RCVBUF = 수신 버퍼 크기

    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(recvBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, &optionLen);
    cout << "수신 버퍼 크기 : " << recvBufferSize << endl;


    // -------------------- 구분선 --------------------

    // SO_REUSEADDR
    // IP주소 및 port 재사용
    // 이게 필요한 이유를 알려면 TCP에 대해 잘 알야함
    // 간단 설명: tcp에서 소켓을 만들고 IP와 포트번호를 바인딩해서 쓰는데, 그 주소를 다른 프로그램이나 등등에서
    // 사용하고 있는 경우나, 사용 못하는 경우가 있을 수 있음. 바인딩이 실패해서 시간을 기다리지 않으면 서버를 띄울 수 가 없는 경우가 있음
    // 그런 경우에 해당 IP와 포트를 재사용 하겠다고 미리 선언하는 것이다.
    // 이미 사용하고 있다고 해도 강제로 쓰게 하는 것

    // 서버 켰다가 바로 종료하면, 그 주소와 포트에서 바로 다시 열수가 없음. 그걸 가능하게 하는것.
    // 이건 사실 개발 단계에서 편하기 위해 사용하는 거라고 한다.

    {
        bool enable = true;
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    // -------------------- 구분선 --------------------

    // IPPROTO_TCP
    // TCP_NODELAY = Nagle 네이글 알고리즘 작동 여부
    // 데이터가 충분히 크면 보내고, 그렇지 않으면 데이터가 충분히 쌓일때까지 대기
    // 효율적으로 보내겠단 거지 네트워크 회선 측면에서인가?

    // 장점 : 작은 패킷이 불필요하게 많이 생성되는 일을 방지
    // 단점 : 반응 시간 손해

    // 일반적으로 게임에서는 끈다. 딜레이를 없게 만든다.

    {
        bool enable = true;
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }


    // -------------------- 소켓 옵션 관련 이론 끝 --------------------





    // WinSock 종료
    ::WSACleanup();
}