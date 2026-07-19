#include "pch.h"
#include "MemoryPool.h"

/*---------------------------
		MemoryPool
---------------------------*/

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
}

MemoryPool::~MemoryPool()
{
	while (_queue.empty() == false)
	{
		MemoryHeader* header = _queue.front();
		_queue.pop();
		::free(header);
	}
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	WRITE_LOCK;
	ptr->allocSize = 0;

	//Pool에 메모리 반납
	_queue.push(ptr);

	_allocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{

	MemoryHeader* header = nullptr;
	
	{// LOCK 생명주기를 위한 괄호

		WRITE_LOCK;
		//Pool에 여분이 있는지?
		if (_queue.empty() == false)
		{
			// 여분이 있는거니 하나를 꺼내온다.
			header = _queue.front();
			_queue.pop();

		}

	}

	// 없으면 새로 만든다.
	if (header == nullptr)
	{
		header = reinterpret_cast<MemoryHeader*>(::malloc(_allocSize));
	}
	else // 디버깅용. else에 들어가면 큐에서 꺼내와서 header에 넣은 거니까
	{
		// MemoryHeader의 allocSize가 0이면 크래시
		ASSERT_CRASH(header->allocSize == 0);
	}

	// 여기까지 왔으면 어쨋든 이제 주소공간의 첫 시작 주소는 header에 들어가 있는거지

	_allocCount.fetch_add(1);

	return header;
}
