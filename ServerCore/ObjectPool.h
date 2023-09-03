#pragma once

#include "Types.h"
#include "MemoryPool.h"

// 클래스 객체를 넣을 수 있는 Pool
template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static Type* Pop(Args&&... args)
	{
// Types.h에 _STOMP 정의가 되어있다면
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
// 정의가 없다면
#else
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif

		
		new(memory)Type(forward<Args>(args)...);	// placement new
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type();
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.Push(MemoryHeader::DetachHeader(obj));
#endif
		
	}
	
	template<typename... Args>
	static shared_ptr<Type> MakeShared(Args&&... args)
	{
		// 두번째 인자로 삭제
		shared_ptr<Type> ptr = { Pop(forward<Args>(args)...), Push };
		return ptr;

	}

private:
	static int32		s_allocSize;
	static MemoryPool	s_pool;
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };