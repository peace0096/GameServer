#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

/*
	Session
*/

// Ŭ���̾�Ʈ�� ������ ��� �ִ� Ŭ����
class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	/* ���� ���� */
	void		SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress	GetAddress() { return _netAddress; }
	SOCKET		GetSocket() { return _socket; }

public:
	/* �������̽� ���� */
	// IocpObject��(��) ���� ��ӵ�
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) override;


	
public:
	// Temp �ӽÿ�
	char _recvBuffer[1000];

private:
	SOCKET			_socket = INVALID_SOCKET;
	NetAddress		_netAddress = {};
	Atomic<bool>	_connected = false;

	
};

