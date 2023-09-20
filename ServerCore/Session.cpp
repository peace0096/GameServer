#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*
	Session
*/

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

/*
	구버전 Send. 일일히 복사해야되는 문제가 있음. 조만간 삭제할 예정
*/
//void Session::Send(BYTE* buffer, int32 len)
//{
//	// 생각해야될 점
//	// 1)버퍼 관리
//	// 2) sendEvent 관리? WSASend 중첩 가능성(귓속말이라던지, 몬스터 정보라던지 등 동시에 여러 개가 send될수도)
//
//	// TEMP
//	SendEvent* sendEvent = xnew<SendEvent>();
//	sendEvent->owner = shared_from_this();
//	sendEvent->buffer.resize(len);
//	::memcpy(sendEvent->buffer.data(), buffer, len);	// 유저가 수백명이라면, 수백번 복사해야된다는 점이 마음에 안 든다.
//
//	WRITE_LOCK;
//	RegisterSend(sendEvent);
//}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registeredSend = false;


	// 현재 RegisterSend가 걸리지 않은 상태라면, 걸어준다
	WRITE_LOCK;
	{
		_sendQueue.push(sendBuffer);

		// true로 바꾼다. 만약 바꾸기 전의 값이 false라면.
		if (_sendRegistered.exchange(true) == false)
		{
			registeredSend = true;
		}
	}
	if (registeredSend)
		RegisterSend();
	
}

// 서버끼리 연결해야 될 수도 있기 때문에 구현.
bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	wcout << "Disconnected : " << cause << endl;

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;

	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetSerivceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0) == false)
		return false;


	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent) == false)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr;	// Release_ref
			return false;
		}

		return true;
	}
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr;	// Release_ref
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();						// 초기화
	_recvEvent.owner = shared_from_this();	// Add_Ref

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr))
	{
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr;		// Release_Ref
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();	// ADD_Ref

	// 보낼 데이터를 sendEvent에 등록해야 함
	// sendEvent에 굳이 데이터(Queue)를 옮겨야 하는 이유는, WsaSend 도중 Queue에서 데이터를 빼낼 때 레퍼런스 카운트가 감소하여 사라질 수도 있기 때문에, sendEvent의 버퍼에 보관한다.
	{
		WRITE_LOCK;
		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			writeSize += sendBuffer->WriteSize();
			// TODO : 나중에 데이터가 일정 선을 넘으면 커트해줘야 함.

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	// Scatter-Gather : 흩어져 있는 데이터ㅏ들을 모아서 한 방에 보낸다.
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	// WSASend는 여러 버퍼를 모아두고 한번에 보내도 됨. 2~3번째 인자가 그 역할을 함
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr;	// Release_Ref
			_sendEvent.sendBuffers.clear();	// Release_Ref
			_sendRegistered.store(false);
		}
	}

}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;	// Release_ref
	_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠코드에서 오버로딩
	OnConnected();

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;	// Release_ref

	// 주의 : Ref Count를 감소시킨다음 disconnect해야 릴리즈시켜야 smart pointer ref 문제가 발생하지 않음
	OnDisconnected();	// 컨텐츠 코드에서 재정의
	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{

	// 이거 안 해주면 Session이 소멸이 안됨.
	_recvEvent.owner = nullptr;		// Release_Ref

	// Recv 실패했을 경우
	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	// 컨텐츠 코드에서 재정의
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	// 커서 정리
	_recvBuffer.Clean();

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr;	//Release_Ref
	_sendEvent.sendBuffers.clear();	// Release_Ref

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	OnSend(numOfBytes);

	WRITE_LOCK;
	// 모든 데이터를 보냈다면, 끝낸다.
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	// 아직 데이터가 남았다면 보낸다.
	else
		RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		cout << "Handle Error : " + errorCode << endl;
		break;
	}
}

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		int32 dataSize = len - processLen;
		// 최소한 헤더는 파싱할 수 있어야 한다.
		if (dataSize < sizeof(PacketHeader))
			break;

		// 받아온 버퍼를 헤더로 캐스팅한다
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// 헤더에 기록된 패킷 크기를 파싱할 수 있어야 한다
		if (dataSize < header.size)
			break;

		// 패킷 조립 성공
		// SendBuffer가 풀링 처리되어있어서, 패킷 시작점부터 받아야함
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;


	}
	return int32();
}
