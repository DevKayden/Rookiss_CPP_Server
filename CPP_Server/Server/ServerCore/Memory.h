#pragma once
#include "Allocator.h"
#include <new>


template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(Xalloc(sizeof(Type)));
	// placement new -> 메모리 공간 확보하고 거기 객체 생성하는 문법
	// 생성자 호출
	new(memory)Type(std::forward<Args>(args)...);
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	// 소멸자 호출
	obj->~Type();
	Xrelease(obj);
}
