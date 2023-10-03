#pragma once

class ClientPacketHandler
{
public:
	// 패킷 받고, 패킷 분해해서 해석하는 함수
	static void HandlePacket(BYTE* buffer, int32 len);

	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

template<typename T, typename C>
class PacketIterator
{
public:


private:

};

// 가변 데이터 리스트를 받아오는 헬퍼 클래스
template<typename T>
class PacketList
{
public:
	PacketList() : _data(nullptr), _count(0) { }
	PacketList(T* data, uint16 count) : _data(data), _count(count) { }

	T& operator[](uint16 index)
	{
		ASSERT_CRASH(index < _count);
		return _data[index];
	}

	uint16 Count() { return _count; }

private:
	T*			_data;
	uint16		_count;
};