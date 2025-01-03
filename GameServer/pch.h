#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobufd.lib")
#endif


#include "CorePch.h"
#include "Enum.pb.h"

using GameSessionRef = std::shared_ptr<class GameSession>;
using PlayerRef = std::shared_ptr<class Player>;