#pragma once

// -------------------
//		1차 시도(싱글스레드 기준)
// -------------------


/*

struct SListEntry
{
	SListEntry* next;
};


// [data][ ][ ][ ]
// Header[ next ]

struct SListHeader
{
	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);

*/

// -------------------
//		2차 시도(멀티스레드 기준)
// -------------------
/*

struct SListEntry
{
	SListEntry* next;
};


// [data][ ][ ][ ]
// Header[ next ]

struct SListHeader
{
	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);
*/

// -------------------
//		3차 시도(ABA Problem 해결)
// -------------------

// 컴파일러에게 16바이트 정렬을 해달라고 요청.
DECLSPEC_ALIGN(16)
struct SListEntry
{
	SListEntry* next;
};

DECLSPEC_ALIGN(16)
struct SListHeader
{
	SListHeader()
	{
		alignment = 0;
		region = 0;
	}

	union
	{
		// 128 바이트 사용
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;	// 아무 의미 없는 define
		// 총 128 바이트 사용
		struct
		{
			uint64 depth : 16;
			uint64 sequence : 48;
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);
