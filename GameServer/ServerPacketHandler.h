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
	// 클라에서 받은 패킷 분해 후 해석하는 함수
	static void HandlePacket(BYTE* buffer, int32 len);

	// 전달할 패킷을 조립하는 함수
	static SendBufferRef Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffs);
};

