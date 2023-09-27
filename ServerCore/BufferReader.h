#pragma once
/*
	BufferReader
*/

// 버퍼 내 시작주소 및 사이즈를 잡아줌
class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();

	BYTE*			Buffer() { return _buffer; }
	uint32			Size() { return _size; }
	uint32			ReadSize() { return _pos; }
	uint32			FreeSize() { return _size - _pos; }

	template<typename T>
	bool			Peek(T* dest) { return Peek(dest, sizeof(T)); }
	bool			Peek(void* dest, uint32 len);

	template<typename T>
	bool			Read(T* dest) { return Read(dest, sizeof(T)); }
	bool			Read(void* dest, uint32 len);


	template<typename T>
	BufferReader&	operator>>(OUT T& dest);	// 데이터를 꺼내 쓸 때 유용하게 사용된다.

private:
	BYTE*			_buffer = nullptr;
	uint32			_size = 0;
	uint32			_pos = 0;			// 버퍼의 커서 역할(현재 버퍼 내에서 가리키고 있는 위치)

};

// 데이터를 읽고 커서를 밀어낸다.
// 1. 예를 들어, 현재 버퍼가 [////|               ]
// 2. size 4만큼 읽었다면,   [////////|           ]
// 3. 읽은 버퍼의 내용을 dest로 반환
template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest)
{
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}
