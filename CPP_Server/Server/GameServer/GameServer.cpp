#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <mutex>
#include <windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

#include "PlayerManager.h"
#include "AccountManager.h"

int main()
{
    for (int32 i = 0; i < 2; i++)
    {
        GThreadManager->Launch([=]
        {
            while (true)
            {
                cout << "PlayerThenAccount" << endl;
                GPlayerManager.PlayerThenAccount();
                this_thread::sleep_for(100ms);
            }
        });
    }
	
	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([=]
            {
                while (true)
                {
                    cout << "AccountThenPlayer" << endl;
                    GAccountManager.AccountThenPlayer();
                    this_thread::sleep_for(100ms);
                }
            });
	}


    /*
        GThreadManager는 CoreGlobal에서 전역으로 선언되어 있는
        ThreadManager의 포인터 변수이다.
    */
    GThreadManager->Join();
}
