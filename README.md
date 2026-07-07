# Modern C++ Multi-Thread Game Server

Rookiss님의 [C++ 서버 강의]를 학습하며 직접 구현하고 확장한 멀티스레드 게임 서버 프레임워크 저장소입니다.  
학습한 핵심 개념과 트러블슈팅 과정은 개인 블로그에 기록하고 있습니다.

[학습 정리 블로그 방문하기 (dev-kayden)](https://dev-kayden.pages.dev/)

---

## 핵심 구현 및 학습 내용

### 1. Multi-Thread & Concurrency
* Thread Management: 스레드 풀(Thread Pool) 구현 및 효율적인 태스크 분배
* Lock & Synchronization: 데드락(Deadlock) 방지를 위한 락 순서(Lock Order) 감지 및 커스텀 Read-Write Lock 구현
* TLS (Thread Local Storage): 스레드 안전성을 확보하고 컨텐션을 줄이기 위한 TLS 활용


---

## 블로그 정리 포스트
강의를 따라가며 깊이 있게 고민한 흔적들과 핵심 이론 정리는 아래 블로그에서 확인하실 수 있습니다.

* 기술 블로그: https://dev-kayden.pages.dev/
* 주요 포스팅 키워드: IOCP, Multi-Threading, C++ Server, Concurrency

---

## 개발 환경
* Language: C++
* OS: Windows
* IDE: Visual Studio 2022