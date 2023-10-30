#include "pch.h"
#include <iostream>
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"

int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		100
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
	
	//char sendData[1000] = "가";		// CP949 = KS-X-1001(한글2바이트) + KS-X-1003(로마1바이트)
	//char sendData2[1000] = u8"가";	// UTF-8 = Unicode(한글3바이트 + 로마1바이트)
	//WCHAR sendData3[1000] = L"가";	// UTF-16 = Unicode(한글/로마 2바이트)
	//TCHAR sendData4[1000] = _T("가");	// 프로젝트 설정 -> 문자 집합 설정에 따라 설정된다.

	// 비용이 싼 WCHAR를 통신에 이용하는 것이 바람직할 것이다!

	while (true)
	{
		// 패킷 버퍼 채우기

		Protocol::S_TEST pkt;
		pkt.set_id(1000);
		pkt.set_hp(100);
		pkt.set_attack(10);


		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(100);
			data->set_remaintime(1.2f);
			data->add_victims(4000);
		}

		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(200);
			data->set_remaintime(2.5f);
			data->add_victims(1000);
			data->add_victims(2000);
		}

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);

		GSessionManager.Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}

