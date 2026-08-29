#pragma once
#include "Allocator.h"
#include <new>

// 전방선언
class MemoryPool;

/*---------------------------
			Memory
---------------------------*/

// 메모리 풀을 관리하는 매니저
class Memory
{
	enum
	{
		/*
			1024바이트까지는 32바이트 간격으로 메모리 풀을 만든다.
			1024~2048까지는 128바이트 간격
			2048~4096까지는 256바이트 간격으로 메모리 풀을 만든다.
			4096보다 큰 건 그냥 쌩 할당기로 할당한다.

			아무래도 크기가 작은 게 많기 때문에.
		*/

		POOL_COUNT = (1024 / 32) + (2048 / 128) + (4096 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	// 메모리 풀 포인터를 벡터로 가진다.
	vector<MemoryPool*> _pools;

	// 메모리 크기 <-> 메모리 풀
	// 메모리 크기에 따라 그에 맞는 메모리 풀을 찾을 수 있도록
	// O(1)로 찾을 수 있게 테이블을 사용
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};


template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	// placement new -> 메모리 공간 확보하고 거기 객체 생성하는 문법
	// 생성자 호출
	new(memory)Type(forward<Args>(args)...); // placement new
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	// 소멸자 호출
	obj->~Type();
	PoolAllocator::Release(obj);
}


template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ xnew<Type>(forward<Args>(args)...), xdelete<Type> };
}