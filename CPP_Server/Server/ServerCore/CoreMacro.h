#pragma once

#define OUT

/*
온갖 매크로를 넣을 파일
*/

/*-------------------------
		Lock
-------------------------*/

#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define	READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define	WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)

/*-------------------------
		Memory
-------------------------*/
#ifdef _DEBUG
#define Xalloc(size) BaseAllocator::Alloc(size)
#define Xrelease(ptr) BaseAllocator::Release(ptr)
#else
#define Xalloc(size) BaseAllocator::Alloc(size)
#define Xrelease(ptr) BaseAllocator::Release(ptr)
#endif

/*-------------------------
		CRASH
-------------------------*/


/*
CRASH 매크로:

일부러 크래시를 내고 싶을때 쓰는 메크로이다.
nullptr을 crash에 넣고,참조해서 크래시를 내려는 코드이다.
하지만 컴파일러가 잡아줄 가능성 때문에 

_anlaysis_assume_(crash != nullptr);

이 코드로 crash가 nullptr이 아님을 속이는 방식이다.

뒤의 원화 표시는 define문을 여러줄 쓰려면 붙여줘야하는 기호.

*/

#define CRASH(cause)							\
{												\
	uint32* crash = nullptr;					\
	__analysis_assume(crash != nullptr);		\
	*crash = 0xDEADBEEF;						\
}


/*
	조건부 크래시:
	인자로 넣어준게 거짓이면 크래시한다.
*/
#define ASSERT_CRASH(expr)						\
{												\
	if(!(expr))									\
	{											\
		CRASH("ASSERT_CRASH");					\
		__analysis_assume(expr);				\
	}											\
}												