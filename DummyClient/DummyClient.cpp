#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"

char sendData[] = "Hello World";

class ServerSession : public PacketSession
{
public:
	virtual void OnConnected() override
	{
		//cout << "Connected To Server" << endl;

	}

	virtual int32 OnRecvPacket(BYTE* buffer, int32 len) override
	{
		BufferReader br(buffer, len);

		PacketHeader header;
		br >> header;

		uint64 id;
		uint32 hp;
		uint16 attack;
		br >> id >> hp >> attack;

		cout << "ID : " << id << " HP : " << hp << " ATT : " << attack << endl;

		char recvBuffer[4096];
		// 가변길이 때문에 8 4 2를 각각 빼줘야 함. 나중엔 이 가변길이도 버퍼에 포함해서 던져주면 좋겠죠?
		br.Read(recvBuffer, header.size - sizeof(PacketHeader) - 8 - 4 - 2);
		cout << recvBuffer << endl;

		return len;
	}

	virtual void OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;

	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,	// TODO : SessionManager 등
		1000
	);
	
	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}


	GThreadManager->Join();
}