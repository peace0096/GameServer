#pragma once
/*
	SendBuffer
	Ref_count 필요 : 하나의 SendBuffer를 여러 개의 Send이벤트에 등록해야 하기 때문이다.
*/

class SendBuffer : enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int32 bufferSize);
	~SendBuffer();

	BYTE* Buffer() { return _buffer.data(); }
	int32 WriteSize() { return _writeSize; }
	int32 Capacity() { return static_cast<int32>(_buffer.size()); }

	void CopyData(void* data, int32 len);

private:
	Vector<BYTE> _buffer;
	int32 _writeSize = 0;
};

