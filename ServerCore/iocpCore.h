#pragma once

/*
	iocpObject

	iocpEvent는 Overlapped 구조체를 받아와서 recv/write 등의 상태를 판단
*/
class IocpObject : public enable_shared_from_this<IocpObject>	// 자기자신에 대한 weak_ptr을 갖게 됨.
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

	bool		Register(IocpObjectRef iocpObject);
	bool		Dispatch(uint32 timeoutMs = INFINITE);
	
private:
	HANDLE		_iocpHandle;
};