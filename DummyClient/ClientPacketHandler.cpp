#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

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

#pragma pack(1)		// 구조체 바이트 할당할 때 자주 사용된다.
// 패킷 설계 Temp


// [ PKT_S_TEST ][BuffsListItem BuffsListItem BuffsListItem]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
	};

	uint16 packetSize;  // 공용 헤더
	uint16 packetId;	// 공용 헤더	-> 헤더역할을 하고 있으므로 헤더 클래스 필요없음
	uint64 id;	// 8
	uint32 hp;	// 4
	uint16 attack;	// 2

	// 가변데이터 데이터셋
	uint16 buffsOffset;
	uint16 buffsCount;
	//vector<BuffData> buffs;
	//wstring name;

	bool Validate()
	{
		uint32 size = 0;

		size += sizeof(PKT_S_TEST);
		if (packetSize < size)
			return false;

		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}

	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>;

	BuffsList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset;
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}
};
#pragma pack()



void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	// 받아온 버퍼를 바로 구조체에 대입!
	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

	//PKT_S_TEST pkt;
	//br >> pkt;

	// cout << "ID : " << id << " HP : " << hp << " ATT : " << attack << endl;

	if (pkt->Validate() == false)
		return;

	PKT_S_TEST::BuffsList buffs = pkt->GetBuffsList();

	cout << "BufCount : " << buffs.Count() << endl;
	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << " " << buffs[i].remainTime << endl;
	}

	for (auto it = buffs.begin(); it != buffs.end(); ++it)
	{
		cout << "BufInfo : " << it->buffId << " " << it->remainTime << endl;
	}

	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << " " << buffs[i].remainTime << endl;
	}

	for (auto& buff : buffs)
	{
		cout << "BufInfo : " << buff.buffId << " " << buff.remainTime << endl;
	}
}

