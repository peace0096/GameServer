#pragma once

enum
{
	S_TEST = 1
};

struct BuffData
{
	uint64 buffId;
	float remainTime;
};

class ServerPacketHandler
{
public:
	// Ŭ�󿡼� ���� ��Ŷ ���� �� �ؼ��ϴ� �Լ�
	static void HandlePacket(BYTE* buffer, int32 len);

	// ������ ��Ŷ�� �����ϴ� �Լ�
	static SendBufferRef Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffs);
};

