#include "pch.h"
#include "DeadLockProfiler.h"


/*--------------------------------------
			DeadLockProfiler
--------------------------------------*/



void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// 아이디를 찾거나 발급한다.
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{ 
		lockId = findIt->second;
	}


	// 잡고 있는 락이 있었다면 즉, lockStack이 비어있지 않은 상태라면
	if (_lockStack.empty() == false)
	{
		// 기존에 발견되지 않은 케이스라면 데드락 여부 다시 확인한다.

		/*
			기존에 잡고 있었던 락 번호가 무엇인지 찾은 다음에,
			만약에 그게 지금 잡으려는 아이디와 다르다면
		*/

		// lockStack에서 이전 lockId를 꺼낸다.
		const int32 prevId = _lockStack.top();
		if (lockId != prevId)
		{
			set<int32>& history = _lockHistory[prevId];
			
			// 발견한 lockId가 처음 발견한 경우면 
			if (history.find(lockId) == history.end())
			{
				// 히스토리에 락 아이디를 추가해주고, 사이클을 체크한다.
				history.insert(lockId);
				CheckCycle();
			}
		}

	}

	_lockStack.push(lockId);

}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	// 아래 두 조건문 처리는 제대로 사용한다면 없어도 되지만,
	// 혹시 모를 버그를 위해서 넣어준 것임.
	if (_lockStack.empty())
		CRASH("MULTIPLE UNLOCK");

	int32 lockId = _nameToId[name];
	if (lockId != _lockStack.top())
		CRASH("INVALID_UNLOCK");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	/*
		Vector 생성자:

		vector<[type]> v(n, m) -> m으로 초기화 된 n개의 원소를 가지는 [type]형의 vector를 생성
	*/

	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++)
	{
		Dfs(lockId);
	}

	// 연산이 끝난 후 정리하는 코드
	// clear(): 모든 원소 제거
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;

	// 모든 인접한 정점을 순회한다.
	// 현재 정점에 해당하는 락 히스토리의 매핑된 set값을 받아온다.
	auto findIt = _lockHistory.find(here);

	// 히스토리에 없었다면
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// 아직 방문한 적이 없다면 방문한다.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// 이미 방문한 적이 있다면 순방향인지 역뱡향인지 체크해야한다.

		
		// here가 there보다 먼저 발견되었다면, there는 here의 후손이다. (순방향 간선)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		// 순방향이 아니고, Dfs(there)가 아직 종료하지 않았다면, there는 here의 선조이다. (역방향 간선)
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);
			
			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}
			
			CRASH("DEAD_DETECTED");
		}


	}

	_finished[here] = true;
}
