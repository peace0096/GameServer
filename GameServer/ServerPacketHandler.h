#pragma once
#include "Protocol.pb.h"

enum
{
	S_TEST = 1
};

class ServerPacketHandler
{
public:
	// 클라에서 받은 패킷 분해 후 해석하는 함수
	static void HandlePacket(BYTE* buffer, int32 len);

	static SendBufferRef MakeSendBuffer(Protocol::S_TEST& pkt);
};

template<typename T>
SendBufferRef _MakeSendBuffer(T& pkt, uint16 pktId)
{
	// 데이터 사이즈 반환
	const uint16 dataSize = pkt.ByteSizeLong();
	const uint16 packetSize = dataSize + sizeof(PacketHeader);

	// 패킷 사이즈만큼 버퍼 생성(헤더 + 데이터)
	SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);

	PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
	header->size = packetSize;
	header->id = pktId;

	ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));

	sendBuffer->Close(packetSize);
	return sendBuffer;

}