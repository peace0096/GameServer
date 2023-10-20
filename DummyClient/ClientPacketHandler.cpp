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
	// ��� ���̵� ���� �б� ó��
	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;
	}
}

#pragma pack(1)		// ����ü ����Ʈ �Ҵ��� �� ���� ���ȴ�.
// ��Ŷ ���� Temp


// [ PKT_S_TEST ][BuffsListItem BuffsListItem BuffsListItem]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
	};

	uint16 packetSize;  // ���� ���
	uint16 packetId;	// ���� ���	-> ��������� �ϰ� �����Ƿ� ��� Ŭ���� �ʿ����
	uint64 id;	// 8
	uint32 hp;	// 4
	uint16 attack;	// 2

	// ���������� �����ͼ�
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

	// �޾ƿ� ���۸� �ٷ� ����ü�� ����!
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

