#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

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

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	br >> id >> hp >> attack;

	cout << "ID : " << id << " HP : " << hp << " ATT : " << attack << endl;

	vector<BuffData> buffs;
	uint16 buffCount;

	br >> buffCount;

	buffs.resize(buffCount);
	for (int32 i = 0; i < buffCount; i++)
	{
		br >> buffs[i].buffId >> buffs[i].remainTime;
	}

	cout << "BufCount : " << buffCount << endl;
	for (int32 i = 0; i < buffCount; i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << " " << buffs[i].remainTime << endl;
	}

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);

	br.Read((void*)name.data(), nameLen * sizeof(WCHAR));

	// �ѱ��� ����
	wcout.imbue(std::locale("kor"));
	wcout << name << endl;
}
