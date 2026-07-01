#pragma once

#include <mutex>
#include <stack>
#include <condition_variable>
#include <atomic>

template<typename T>
class LockStack
{
public:
	LockStack() {} //생성자는 동작 없음.

	// 복사 연산 같은 것들을 막는다.
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;
	
	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_conVar.notify_one(); // 스택에 값 넣으면 notify
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty() == false)
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;

	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex); // 락 잡기
		_conVar.wait(lock, [this] { return _stack.empty() == false; });
		// 스택이 비지 않았을 때만 가능하게 condition variable 사용
		value = std::move(_stack.top());
		_stack.pop();

	}

	


private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _conVar;
};


template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node
	{
		// 생성자: data는 value를 넣고, next는 nullptr로 초기화
		Node(const T& value) : data(make_shared<T>(value))
		{

		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:

	void Push(const T& value)
	{
		
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = _head;

		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{

		}

		

	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			// 참조권 획득
			IncreaseHeadCount(oldHead);
		}
	}


	

	
private:

	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			//Counter;
		}
	}


	atomic<CountedNodePtr> _head;

	
};
