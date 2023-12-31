#pragma once

class Session;

/*
	EventType
*/

enum class EventType : uint8 {
	Connect,
	Disconnect,
	Accept,
	//PreRecv,
	Recv,
	Send
};

/*
	IocpEvent
	절대 가상함수를 사용하지 마라. offset 첫번째로 설정되어 메모리를 덮어씌울 수도 있기 때문이다.
*/

// 예전에 만들었던 overlappedEx 구조체랑 같은 역할임!
class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void			Init();

public:
	EventType		eventType;
	IocpObjectRef	owner;
	
};

/*
	ConnectEvent
*/
class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) { };
};

/*
	DisconnectEvent
*/
class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) { };
};

/*
	AcceptEvent
*/
class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { };

public:
	SessionRef	session = nullptr;
};


/*
	RecvEvent
*/
class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) { };
};

/*
	SendEvent
*/
class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) { };

	// TEMP -> WSASend가 겹치면 안되기 때문임.
	vector<SendBufferRef> sendBuffers;
};