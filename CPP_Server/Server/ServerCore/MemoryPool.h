#pragma once


/*
	특정 범위(크기)인 데이터들을 모아서 각 메모리 풀에 넣어줄거임
	근데 그 객체들마다 사이즈가 다를 수 있으니까 어떤 메모리를 할당할때 디버깅을 도와주기 위한
	메모리 헤더를 붙여줄거임. 메모리를 할당할때는 메모리 헤더와 실제 데이터 구조로 만들어줄거임

	[MemoryHeader][Data]

	실제 표준 Allocator도 메모리 헤더를 앞에 따로 두고, 그 안에 크기가 얼마인지, 그 다음 주소는 뭔지 등등의
	메타데이터를 저장하는 공간이 있음.
*/

/*---------------------------
		MemoryHeader
---------------------------*/

struct MemoryHeader
{
	// [MemoryHeader][Data]

	MemoryHeader(int32 size) : allocSize(size) { }

	// 메모리 할당한 다음에 첫번째 주소를 인자로 넘겨받음
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		// replacement new를 이용해서 MemoryHeader 영역에 크기 넣기
		new(header)MemoryHeader(size);
		/*
			메모리 헤더는 우리가 내부적으로 관리하는 것이기에 그 다음 데이터를 넣는 주소 공간을
			리턴해줄 것이다.

			C++ 포인터 연산 특성상 ++을 해주면 그 자료형 크기 만큼 넘겨주기까 ++연산만해서 넘기면 된다.
		*/
		return reinterpret_cast<void*>(++header);
	}

	// 반대로 DetechHeadar는 MemoryHaeder의 주소 값을 리턴한다.
	static MemoryHeader* DetechHeader(void* ptr)
	{
		/*
			ptr이 Date영역의 시작 주소일테니 MemoryHeader로 캐스팅해서 1을 빼면
			MemoryHeader의 시작주소가 된다.
		*/

		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;

	}

	int32 allocSize;
	// 필요한 정보가 있으면 여기에 이후에 추가
};

/*---------------------------
		MemoryPool
---------------------------*/

/*
	메모리 풀은 크기별로 여러개가 생길 것이므로 멤버변수로 size를 가져야함

	Push -> 다 사용했는데 당장 삭제하지 않고 재사용하고 싶으면 Push
	Pop  -> 메모리가 필요해서 사용하고 싶으면 Pop
*/

class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void Push(MemoryHeader* ptr);
	MemoryHeader* Pop();


private:
	int32 _allocSize = 0;
	atomic<int32> _allocCount = 0; // 해당 메모리풀에 몇개가 할당되어 있는지 카운트

	USE_LOCK;
	queue<MemoryHeader*> _queue;
	// 해당 큐에는 재사용 가능한 여분이 있으면 넣어줄거임. 메모리가 필요하면 큐가 비어있지않으면 꺼내서 그 공간 쓰고,
	// 큐가 비어있으면 못 쓰는거?


};

