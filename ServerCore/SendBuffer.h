#pragma once


/*
	SendBuffer
	Ref_count �ʿ� : �ϳ��� SendBuffer�� ���� ���� Send�̺�Ʈ�� ����ؾ� �ϱ� �����̴�.
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

/*
	SendBufferChunk(�ſ� ū ���. ���⼭ �߶� SendBuffer�� ���)
*/
class SendBufferChunk : public enable_shared_from_this<SendBufferChunk>
{
	enum {
		SEND_BUFFER_CHUNK_SIZE = 0x1000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void				 Reset();
	SendBufferRef		 Open(uint32 allocSize);
	void				 Close(uint32 writeSize);
		
	bool				 IsOpen() { return _open; }
	BYTE*				 Buffer() { }
	uint32				 FreeSize() { return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE>		_buffer = {};
	bool									_open = false;
	uint32									_usedSize = 0;
	

};

/*
	SendBufferManager
*/
class SendBufferManager
{
public:
	SendBufferRef			Open(uint32 size);					// SendBuffer ������� �Ϻθ� ����ϰڴ�.

private:
	SendBufferChunkRef		Pop();
	void					Push(SendBufferChunkRef buffer);

	static void				PushGlobal(SendBufferChunk* buffer);


private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;

};