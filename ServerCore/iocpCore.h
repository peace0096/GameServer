#pragma once

/*
	iocpObject

	iocpEvent는 Overlapped 구조체를 받아와서 recv/write 등의 상태를 판단
*/
class IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};


/*
	iocpCore
*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE		GetHandle() { return _iocpHandle; }

	bool		Register(IocpObject* iocpObject);
	bool		Dispatch(uint32 timeoutMs = INFINITE);
	
private:
	HANDLE		_iocpHandle;
};


// TEMP 임시용
extern IocpCore GIocpCore;