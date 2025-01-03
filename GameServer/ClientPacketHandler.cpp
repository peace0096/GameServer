#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "Player.h"
#include "Room.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 컨텐츠 작업 필요

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : 로그이 검증 필요
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	static Atomic<uint64> idGenerator = 1;

	{
		auto player = loginPkt.add_players();
		player->set_name(u8"name1");
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		PlayerRef playerRef = MakeShared<Player>();
		playerRef->playerId = idGenerator.fetch_add(1);
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->ownerSession = gameSession;

		gameSession->_players.push_back(playerRef);
	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);


	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	uint64 index = pkt.playerindex();

	PlayerRef player = gameSession->_players[index];
	GRoom.Enter(player);

	Protocol::S_ENTER_GAME enterGamePkt;
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterGamePkt);
	player->ownerSession->Send(sendBuffer);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	std::cout << pkt.msg() << '\n';
	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	GRoom.Broadcast(sendBuffer);
	return true;
}