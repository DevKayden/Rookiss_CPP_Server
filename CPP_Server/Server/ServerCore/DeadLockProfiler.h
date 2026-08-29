#pragma once
#include <stack>
#include <map>
#include <vector>

/*--------------------------------------
			DeadLockProfiler
--------------------------------------*/


/*
	디버그 모드에서만 락을 잡을때와 락을 해제할때 DeadLockProfiler에게
	정보를 전달한다. 그러면 그래프 알고리즘으로 사이클이 있는지 판별을 한다.
*/

class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	// 이름과 Id를 매핑하는 맵
	unordered_map<const char*, int32> _nameToId;
	// 반대로 Id와 이름을 매핑하는 맵
	unordered_map<int32, const char*> _idToName;
	// 락이 실행되는 걸 추적할 스택
	stack<int32>					  _lockStack;
	// 몇번 락이 몇번 락을 잡았는지 히스토리를 남긴다
	// 아마 간선 연결을 저장하는 맵인 것 같음
	map<int32, set<int32>>			  _lockHistory;

	Mutex _lock;

private:
	/*
		사이클을 판별하는 알고리즘을 돌리기 위한 임시적인 값을 저장할 벡터
		사이클 체크할때마다 초기화해서 사용
	*/
	vector<int32> _discoveredOrder; // 노드가 발견된 순서를 기록하는 배열
	int32 _discoveredCount = 0; // 노드가 발견된 순서 추적을 위한 카운팅
	vector<bool> _finished; //Dfs(i)가 종료되었는지 여부
	vector<int32> _parent; //parent가 누군지 저장할 벡터
};

