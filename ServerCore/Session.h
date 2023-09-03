#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

class Service;

/*
	Session
*/

// 클라이언트의 정보를 담고 있는 클래스
class Session : public IocpObject
{
	// 이 친구들만 사용할 수 있도록 열어줌.
	friend class Listener;
	friend class IocpCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:
	void				Disconnect(const WCHAR* cause);	// 강제 연결종료

	shared_ptr<Service> GetService() { return _service.lock(); }
	void				SetService(shared_ptr<Service> service) { _service = service; }

public:
						/* 정보 관련 */
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
						/* 인터페이스 구현 */
						// IocpObject을(를) 통해 상속됨
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) override;

private:
						/* 전송 관련 */
	void				RegisterConnect();
	void				RegisterRecv();
	void				RegisterSend();
	
	void				ProcessConnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);
	
	void				HandleError(int32 errorCode);

protected:
						/* 컨텐츠 코드에서 오버로딩 */
	virtual void		OnConnected() { }
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }
public:
	// Temp 임시용
	char _recvBuffer[1000];

private:
	weak_ptr<Service>	_service;
	SOCKET			_socket = INVALID_SOCKET;
	NetAddress		_netAddress = {};
	Atomic<bool>	_connected = false;

private:
	USE_LOCK;
	
	/* 수신 관련 */

	/* 송신 관련 */

private:
						/* IocpEvent 재사용 */
	RecvEvent			_recvEvent;
};

