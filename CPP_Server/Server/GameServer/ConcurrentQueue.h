#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>


template<typename T>
class LockQueue
{
public:
	LockQueue() { }

	// 멀티 쓰레드에서는 복사에 취약하니 복사를 막는다.
	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_conVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_queue.empty())
			return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;

	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_conVar.wait(lock, [this] { return _queue.empty() == false; });
		value = std::move(_queue.front());
		_queue.pop();
	}

private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _conVar;


};