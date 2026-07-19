#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

/*---------------------------
			Memory
---------------------------*/

// 여기서 벡터안에 메모리풀 포인터와 풀테이블을 다 만들어준다
Memory::Memory()
{
	int32 size = 0;
	int32 tableIndex = 0;

	// 1024바이트까지는 32바이트 간격으로 메모리 풀을 만든다.
	for (size = 32; size <= 1024; size += 32)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	// 1024~2048까지는 128바이트 간격
	for (size = 1024; size <= 2048; size += 128)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	// 2048~4096까지는 256바이트 간격으로 메모리 풀을 만든다.
	for (size = 2048; size <= 4096; size += 256)
	{
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

}

Memory::~Memory()
{
	// 사실 이게 소멸한다는 건 프로그램이 종료되는 거라서 상관없긴 한데 그래도 메모리 해제는 하자.
	for (MemoryPool* pool : _pools)
	{
		delete pool;
	}

	_pools.clear(); // 벡터 비우기
}

void* Memory::Allocate(int32 size)
{
	MemoryHeader* header = nullptr;
	const int32 allocSize = size + sizeof(MemoryHeader);

	if (allocSize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 할당
		header = reinterpret_cast<MemoryHeader*>(::malloc(allocSize));
	}
	else
	{
		// 메모리 풀에서 꺼내온다
		header = _poolTable[allocSize]->Pop();
	}

	// 해당 메모리 풀에 memoryheader만들고, Data영역 포인터 리턴
	return MemoryHeader::AttachHeader(header, allocSize);
}

void Memory::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetechHeader(ptr);

	const int32 allocSize = header->allocSize;
	ASSERT_CRASH(allocSize > 0);

	if (allocSize > MAX_ALLOC_SIZE)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 해제
		::free(header);
	}
	else
	{
		// 메모리 풀에 반납한다
		_poolTable[allocSize]->Push(header);
	}
}