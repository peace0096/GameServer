#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

/*
	Session
*/

// 클라이언트의 정보를 담고 있는 클래스
class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	/* 정보 관련 */
	void		SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress	GetAddress() { return _netAddress; }
	SOCKET		GetSocket() { return _socket; }

public:
	/* 인터페이스 구현 */
	// IocpObject을(를) 통해 상속됨
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) override;


	
public:
	// Temp 임시용
	char _recvBuffer[1000];

private:
	SOCKET			_socket = INVALID_SOCKET;
	NetAddress		_netAddress = {};
	Atomic<bool>	_connected = false;

	
};

