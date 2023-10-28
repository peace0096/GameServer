#pragma once

enum
{
	S_TEST = 1
};

template<typename T, typename C>
class PacketIterator
{
public:
	PacketIterator(C& container, uint16 index) : _container(container), _index(index) {}

	bool				operator!=(const PacketIterator& other) const { return _index != other._index; }
	const T& operator*() const { return _container[_index]; }
	T& operator*() { return _container[_index]; }
	T* operator->() { return &_container[_index]; }
	PacketIterator& operator++() { _index++; return *this; }
	PacketIterator		operator++(int32) { PacketIterator ret = *this; ++_index; return ret; }

private:
	C& _container;
	uint16	_index;

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

	// ranged-base for 지원
	PacketIterator<T, PacketList<T>> begin() { return PacketIterator<T, PacketList<T>>(*this, 0); }
	PacketIterator<T, PacketList<T>> end() { return PacketIterator<T, PacketList<T>>(*this, _count); }

private:
	T* _data;
	uint16		_count;
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

};

#pragma pack(1)		// 구조체 바이트 할당할 때 자주 사용된다.
// 패킷 설계 Temp


// [ PKT_S_TEST ][BuffsListItem BuffsListItem BuffsListItem][victim victim][victim victim]
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;			// 8
		float remainTime;		// 4

		// Victim List
		uint16 victimsOffset;	// 2
		uint16 victimsCount;	// 2
	};

	uint16 packetSize;  // 공용 헤더
	uint16 packetId;	// 공용 헤더	-> 헤더역할을 하고 있으므로 헤더 클래스 필요없음
	uint64 id;	// 8
	uint32 hp;	// 4
	uint16 attack;	// 2

	// 가변데이터 데이터셋
	uint16 buffsOffset;
	uint16 buffsCount;
	//vector<BuffData> buffs;
	//wstring name;
};

class PKT_S_TEST_WRITE
{
private:
	PKT_S_TEST* _pkt = nullptr;
	SendBufferRef _sendBuffer;
	BufferWriter _bw;

public:
	using BuffsListItem = PKT_S_TEST::BuffsListItem;
	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>;
	using BuffsVictimsList = PacketList<uint64>;

	PKT_S_TEST_WRITE(uint64 id, uint32 hp, uint16 attack)
	{
		// 버퍼 선언
		_sendBuffer = GSendBufferManager->Open(4096);
		_bw = BufferWriter(_sendBuffer->Buffer(), _sendBuffer->AllocSize());

		_pkt = _bw.Reserve <PKT_S_TEST>();
		_pkt->packetSize = 0;		// 값 채워야 함
		_pkt->packetId = S_TEST;
		_pkt->id = id;
		_pkt->hp = hp;
		_pkt->attack = attack;
		_pkt->buffsOffset = 0;		// 값 채워야 함
		_pkt->buffsCount = 0;		// 값 채워야 함

	}

	BuffsList ReserveBuffsList(uint16 buffCount)
	{
		// 패킷 시작점
		BuffsListItem* firstBuffsListItem = _bw.Reserve<BuffsListItem>(buffCount);
		_pkt->buffsCount = buffCount;
		_pkt->buffsOffset = (uint64)firstBuffsListItem - (uint64)_pkt;
		return BuffsList(firstBuffsListItem, buffCount);
	}

	BuffsVictimsList ReserveBuffsVictimsList(BuffsListItem* buffsItem, uint16 victimsCount)
	{
		uint64* firstVictimsListItem = _bw.Reserve<uint64>(victimsCount);
		buffsItem->victimsOffset = (uint64)firstVictimsListItem - (uint64)_pkt;
		buffsItem->victimsCount = victimsCount;
		return BuffsVictimsList(firstVictimsListItem, victimsCount);
	}

	SendBufferRef CloseAndReturn()
	{
		// 패킷사이즈 계산
		_pkt->packetSize = _bw.WriteSize();
		_sendBuffer->Close(_bw.WriteSize());
		return _sendBuffer;
	}


};

#pragma pack()
