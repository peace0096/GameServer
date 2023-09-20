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
	������ Send. ������ �����ؾߵǴ� ������ ����. ������ ������ ����
*/
//void Session::Send(BYTE* buffer, int32 len)
//{
//	// �����ؾߵ� ��
//	// 1)���� ����
//	// 2) sendEvent ����? WSASend ��ø ���ɼ�(�ӼӸ��̶����, ���� ��������� �� ���ÿ� ���� ���� send�ɼ���)
//
//	// TEMP
//	SendEvent* sendEvent = xnew<SendEvent>();
//	sendEvent->owner = shared_from_this();
//	sendEvent->buffer.resize(len);
//	::memcpy(sendEvent->buffer.data(), buffer, len);	// ������ ������̶��, ����� �����ؾߵȴٴ� ���� ������ �� ���.
//
//	WRITE_LOCK;
//	RegisterSend(sendEvent);
//}

void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registeredSend = false;


	// ���� RegisterSend�� �ɸ��� ���� ���¶��, �ɾ��ش�
	WRITE_LOCK;
	{
		_sendQueue.push(sendBuffer);

		// true�� �ٲ۴�. ���� �ٲٱ� ���� ���� false���.
		if (_sendRegistered.exchange(true) == false)
		{
			registeredSend = true;
		}
	}
	if (registeredSend)
		RegisterSend();
	
}

// �������� �����ؾ� �� ���� �ֱ� ������ ����.
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

	_recvEvent.Init();						// �ʱ�ȭ
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

	// ���� �����͸� sendEvent�� ����ؾ� ��
	// sendEvent�� ���� ������(Queue)�� �Űܾ� �ϴ� ������, WsaSend ���� Queue���� �����͸� ���� �� ���۷��� ī��Ʈ�� �����Ͽ� ����� ���� �ֱ� ������, sendEvent�� ���ۿ� �����Ѵ�.
	{
		WRITE_LOCK;
		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			writeSize += sendBuffer->WriteSize();
			// TODO : ���߿� �����Ͱ� ���� ���� ������ ĿƮ����� ��.

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	// Scatter-Gather : ����� �ִ� �����ͤ����� ��Ƽ� �� �濡 ������.
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
	// WSASend�� ���� ���۸� ��Ƶΰ� �ѹ��� ������ ��. 2~3��° ���ڰ� �� ������ ��
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

	// ���� ���
	GetService()->AddSession(GetSessionRef());

	// �������ڵ忡�� �����ε�
	OnConnected();

	// ���� ���
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;	// Release_ref

	// ���� : Ref Count�� ���ҽ�Ų���� disconnect�ؾ� ��������Ѿ� smart pointer ref ������ �߻����� ����
	OnDisconnected();	// ������ �ڵ忡�� ������
	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{

	// �̰� �� ���ָ� Session�� �Ҹ��� �ȵ�.
	_recvEvent.owner = nullptr;		// Release_Ref

	// Recv �������� ���
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

	// ������ �ڵ忡�� ������
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	// Ŀ�� ����
	_recvBuffer.Clean();

	// ���� ���
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
	// ��� �����͸� ���´ٸ�, ������.
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	// ���� �����Ͱ� ���Ҵٸ� ������.
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
		// �ּ��� ����� �Ľ��� �� �־�� �Ѵ�.
		if (dataSize < sizeof(PacketHeader))
			break;

		// �޾ƿ� ���۸� ����� ĳ�����Ѵ�
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));

		// ����� ��ϵ� ��Ŷ ũ�⸦ �Ľ��� �� �־�� �Ѵ�
		if (dataSize < header.size)
			break;

		// ��Ŷ ���� ����
		// SendBuffer�� Ǯ�� ó���Ǿ��־, ��Ŷ ���������� �޾ƾ���
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;


	}
	return int32();
}
