#pragma once

class ClientPacketHandler
{
public:
	// 패킷 받고, 패킷 분해해서 해석하는 함수
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

