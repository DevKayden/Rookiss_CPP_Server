#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <mutex>
#include <windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

CoreGlobal Core;

void ThreadMain()
{
    while (true)
    {
        cout << "Hello! I am Thread ... " << LThreadId << endl;
        this_thread::sleep_for(100ms);
    }
}

int main()
{
    for (int32 i = 0; i < 5; i++)
    {
        GThreadManager->Launch(ThreadMain);
    }
    
    /*
        GThreadManager는 CoreGlobal에서 전역으로 선언되어 있는
        ThreadManager의 포인터 변수이다.
    */
    GThreadManager->Join();
}
