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
	// ���� ������
	// 1) ���ڿ�
	// 2) �׳� ����Ʈ �迭? (��� �̹��� ���� ����)
	// 3) ����Ʈ
	vector<BuffData> buffs;

	wstring name;
};

class ClientPacketHandler
{
public:
	// ��Ŷ �ް�, ��Ŷ �����ؼ� �ؼ��ϴ� �Լ�
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

