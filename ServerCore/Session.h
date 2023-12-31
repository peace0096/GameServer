#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

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

	enum
	{
		BUFFER_SIZE = 0x10000, // 64KB
	};

public:
	Session();
	virtual ~Session();

public:
						/* 외부에서 사용 */
	void				Send(SendBufferRef sendBuffer);
	bool				Connect();
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
	bool					RegisterConnect();
	bool					RegisterDisconnect();
	void					RegisterRecv();
	void					RegisterSend();
	
	void					ProcessConnect();
	void					ProcessDisconnect();
	void					ProcessRecv(int32 numOfBytes);
	void					ProcessSend(int32 numOfBytes);
	
	void					HandleError(int32 errorCode);

protected:
						/* 컨텐츠 코드에서 오버로딩 */
	virtual void			OnConnected() { }
	virtual int32			OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void			OnSend(int32 len) { }
	virtual void			OnDisconnected() { }

private:
	weak_ptr<Service>		_service;
	SOCKET					_socket = INVALID_SOCKET;
	NetAddress				_netAddress = {};
	Atomic<bool>			_connected = false;

private:
	USE_LOCK;
	
						/* 수신 관련 */
	RecvBuffer				_recvBuffer;


	/* 송신 관련 */
	Queue<SendBufferRef>	_sendQueue;
	Atomic<bool>			_sendRegistered = false;	// 이벤트 등록도중, WSASend가 실행되면 큐에만 넣고 빠져나가게 할 것임.

private:
						/* IocpEvent 재사용 */
	ConnectEvent			_connectEvent;
	DisconnectEvent			_disconnectEvent;
	RecvEvent				_recvEvent;
	SendEvent				_sendEvent;
};


// 나중에 송수신할 데이터들이 많아지면 tcp 특성상 여러 개로 쪼개서 전달될 것임
// 그래서 전체패킷이 도착했는지 확인할 수단이 필요함
// 우리만의 프로토콜을 정의해야 한다!

/*
	PacketSession
*/

// 패킷 종류를 알려주는 녀석
struct PacketHeader
{
	uint16 size;
	uint16 id; // 프로토콜 ID (ex 1 = 로그인, 2=이동요청)

};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSession() { return static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32 OnRecv(BYTE* buffer, int32 len) sealed;	// 앞으로, PacketSession을 상속받는 친구는 onRecv를 재정의를 할 수 없다.
	virtual void OnRecvPacket(BYTE* buffer, int32 len) abstract;	// 추상화만 되어있음. 무조건 구현해줘야 한다!
};