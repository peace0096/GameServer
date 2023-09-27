#pragma once
/*
	BufferWriter
*/

// 버퍼 내 시작주소 및 사이즈를 잡아줌
class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();

	BYTE* Buffer() { return _buffer; }
	uint32			Size() { return _size; }
	uint32			WriteSize() { return _pos; }
	uint32			FreeSize() { return _size - _pos; }


	template<typename T>
	bool			Write(T* src) { return Write(src, sizeof(T)); }
	bool			Write(void* src, uint32 len);

	template<typename T>
	T*				Reserve();

	template<typename T>
	BufferWriter&	operator<<(const T& src);

	template<typename T>
	BufferWriter&	operator<<(T&& src);

private:
	BYTE* _buffer = nullptr;
	uint32			_size = 0;
	uint32			_pos = 0;			// 버퍼의 커서 역할(현재 버퍼 내에서 가리키고 있는 위치)

};

template<typename T>
T* BufferWriter::Reserve()
{
	if (FreeSize() < sizeof(T))
		return false;

	T* ret = reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);

	return ret;
}

// 당기기
template<typename T>
inline BufferWriter& BufferWriter::operator<<(const T& src)
{
	*reinterpret_cast<T*>(&_buffer[_pos]) = src;
	_pos += sizeof(T);
	return *this;
}

// 오른값 참조
template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& src)
{
	*reinterpret_cast<T*>(&_buffer[_pos]) = src;
	_pos += sizeof(T);
	return *this;
}
