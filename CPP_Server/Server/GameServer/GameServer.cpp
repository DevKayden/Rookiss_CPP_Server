#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <mutex>
#include <windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

#include "RefCounting.h"
#include "Memory.h"

using KnightRef = TSharedPtr<class Knight>;


class Player
{
public:

	Player() {}
	virtual ~Player() {}
};

class Knight : public Player
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	Knight(int32 hp) : _hp(hp)
	{
		cout << "Knight()" << endl;
	}
	
	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	int32 _hp;
	int32 _mp;
};


int main()
{

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Vector<Knight> v(10);

					Map<int32, Knight> m;
					m[100] = Knight();

					this_thread::sleep_for(10ms);
				}
			});
	}
	

	GThreadManager->Join();

}

