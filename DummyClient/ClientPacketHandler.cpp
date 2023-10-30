#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "Protocol.pb.h"

enum
{
	S_TEST = 1
};

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;
	// 헤더 아이디에 따른 분기 처리
	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;
	}
}


void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	Protocol::S_TEST pkt;
	
	ASSERT_CRASH(pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)));

	cout << pkt.id() << " " << pkt.hp() << " " << pkt.attack() << endl;
	cout << "BuffSize : " << pkt.buffs_size() << endl;
	
	for (auto& buf : pkt.buffs())
	{
		cout << "BUFINFO : " << buf.buffid() << " " << buf.remaintime() << endl;
		cout << "VICTIMS : " << buf.victims_size() << endl;
		for (auto& vic : buf.victims())
		{
			cout << vic << " ";
		}

		cout << endl;
	}
}

