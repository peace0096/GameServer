#pragma once

class SendBufferChunk;

/*
	SendBuffer
	Ref_count 필요 : 하나의 SendBuffer를 여러 개의 Send이벤트에 등록해야 하기 때문이다.
*/

class SendBuffer
{
public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize);
	~SendBuffer();

	BYTE*				Buffer() { return _buffer; }
	uint32				AllocSize() { return _allocSize; }
	int32				WriteSize() { return _writeSize; }
	void				Close(uint32 writeSize);

private:
	BYTE*				_buffer;
	uint32				_allocSize = 0;
	uint32				_writeSize = 0;
	SendBufferChunkRef	_owner;
};

/*
	SendBufferChunk(매우 큰 덩어리. 여기서 잘라서 SendBuffer로 사용)
*/
class SendBufferChunk : public enable_shared_from_this<SendBufferChunk>
{
	enum {
		SEND_BUFFER_CHUNK_SIZE = 6000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void				 Reset();
	SendBufferRef		 Open(uint32 allocSize);
	void				 Close(uint32 writeSize);
		
	bool				 IsOpen() { return _open; }
	BYTE*				 Buffer() { return &_buffer[_usedSize]; }
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
	SendBufferRef			Open(uint32 size);					// SendBuffer 덩어리에서 일부를 사용하겠다.

private:
	SendBufferChunkRef		Pop();
	void					Push(SendBufferChunkRef buffer);

	static void				PushGlobal(SendBufferChunk* buffer);


private:
	USE_LOCK;
	Vector<SendBufferChunkRef> _sendBufferChunks;

};