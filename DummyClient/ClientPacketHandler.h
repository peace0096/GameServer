#pragma once

class ClientPacketHandler
{
public:
	// ��Ŷ �ް�, ��Ŷ �����ؼ� �ؼ��ϴ� �Լ�
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

