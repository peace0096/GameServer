#pragma once
#include "Types.h"

/*
* Mutex 대신 Lock을 직접 구현하는 이유
* 1) 재귀적으로 락을 걸 수 없다.
* 2) 상호베타적 특성이 없다.
* => Reader/Writer 형식으로 구현!
*/

/*----------------------
 	RW SpinLock
----------------------*/


/*----------------------
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
읽기/쓰기 락을 32비트에 저장
----------------------*/

// W -> W 가능
// W -> R 가능
// R -> W 불가능
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,		// 상위 16비트는 모두 1	=> WriteFlag 값을 추출하기 위한 마스크
		READ_COUNT_MASK = 0x0000'FFFF,			// ReadFlag를 읽기 위한 마스크
		EMPTY_FLAG = 0x0000'0000				// 모든 Flag가 비어있음.
	};

public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);


private:
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
};


/*----------------------
	LockGuards
----------------------*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
	~ReadLockGuard() { _lock.ReadUnlock(_name); }
private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }
private:
	Lock& _lock;
	const char* _name;
};