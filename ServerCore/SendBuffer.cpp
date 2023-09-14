#include "pch.h"
#include "SendBuffer.h"

/*
	SendBuffer
*/

SendBuffer::SendBuffer(int32 bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::CopyData(void* data, int32 len)
{
	// 오버플로우 체크.
	ASSERT_CRASH(Capacity() >= len);
	::memcpy(_buffer.data(), data, len);
	_writeSize = len;
}

/*
	SendBufferChunk
*/
SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);

	return SendBufferRef();
}

void SendBufferChunk::Close(uint32 writeSize)
{
}



/*
	SendBufferManager
*/

SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop();	// WRITE_LOCK
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	// 다 사용하면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}
	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal));
}

