#pragma once

/*
	iocpObject

	iocpEvent�� Overlapped ����ü�� �޾ƿͼ� recv/write ���� ���¸� �Ǵ�
*/
class IocpObject : public enable_shared_from_this<IocpObject>	// �ڱ��ڽſ� ���� weak_ptr�� ���� ��.
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