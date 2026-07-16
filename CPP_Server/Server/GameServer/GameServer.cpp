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
using InventoryRef = TSharedPtr<class Inventory>;

class Knight : public RefCountable
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}
	
	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	void SetTarget(KnightRef target)
	{
		_target = target;
	}

	/*static void* operator new(size_t size)
	{
		cout << "new!" << endl;
		void* ptr = malloc(size);
		return ptr;
	}
	
	static void operator delete(void* ptr)
	{
		cout << "delete!" << endl;
		free(ptr);
	}*/

private:
	KnightRef _target = nullptr;
	InventoryRef _inventory = nullptr;

};

class Inventory : public RefCountable
{
public:
	Inventory(KnightRef& knight) : _knight(knight)
	{

	}

private:

	KnightRef& _knight;

};


int main()
{
	Knight* knight = xnew<Knight>();

	xdelete(knight);


}
	


