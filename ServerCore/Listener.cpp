#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"


/*
	Listener
*/

Listener::~Listener()
{
	SocketUtils::Close(_socket);
	for(AcceptEvent* acceptEvent : _acceptEvents)
	{
		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(NetAddress netAddress)
{
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (GIocpCore.Register(this) == false)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if(SocketUtils::Bind(_socket, netAddress) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32 acceptCount = 1;
	for (int32 i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return false;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	// accept가 아닌 다른 overlapped가 왔다면 에러
	// accpet가 맞다면, 정상적으로 처리
	ASSERT_CRASH(iocpEvent->GetType() == EventType::Accept);
	AcceptEvent* accpetEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(accpetEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	Session* session = xnew<Session>();
	acceptEvent->Init();
	acceptEvent->SetSession(session);

	DWORD bytesReceived = 0;
	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		// 실패했을 때
		int32 errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			// Pending 상태 외의 에러코드라면, 문제가 있는 상황임
			// 일단 다시 Accept 해줌
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	Session* session = acceptEvent->GetSession();

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}
	session->SetNetAddress(NetAddress(sockAddress));

	cout << "Client Conneted !" << endl;

	RegisterAccept(acceptEvent);
	

}
