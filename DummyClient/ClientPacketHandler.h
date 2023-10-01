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

struct S_TEST
{
	uint64 id;
	uint32 hp;
	uint16 attack;
	// 가변 데이터
	// 1) 문자열
	// 2) 그냥 바이트 배열? (길드 이미지 같은 정보)
	// 3) 리스트
	vector<BuffData> buffs;

	wstring name;
};

class ClientPacketHandler
{
public:
	// 패킷 받고, 패킷 분해해서 해석하는 함수
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

