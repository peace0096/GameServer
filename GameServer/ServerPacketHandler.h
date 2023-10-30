#pragma once
#include "Protocol.pb.h"

enum
{
	S_TEST = 1
};

class ServerPacketHandler
{
public:
	// Ŭ�󿡼� ���� ��Ŷ ���� �� �ؼ��ϴ� �Լ�
	static void HandlePacket(BYTE* buffer, int32 len);

	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt);
};

template<typename T>
SendBufferRef _MakeSendBuffer(T& pkt, uint16 pktId)
{
	// ������ ������ ��ȯ
	const uint16 dataSize = pkt.ByteSizeLong();
	const uint16 packetSize = dataSize + sizeof(PacketHeader);

	// ��Ŷ �����ŭ ���� ����(��� + ������)
	SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
	header->size = packetSize;
	header->id = pktId;

	ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));

	sendBuffer->Close(packetSize);
	return sendBuffer;

}