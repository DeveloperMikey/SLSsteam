#pragma once

#include "../sdk/sdk.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>


namespace Ticket
{
	enum class ETicketType
	{
		AppOwnershipTicket,
		EncryptedAppOwnershipTicket
	};

	constexpr const char* getTicketField(const ETicketType type)
	{
		switch (type)
		{
			case ETicketType::AppOwnershipTicket:
				return "ticket";
			case ETicketType::EncryptedAppOwnershipTicket:
				return "encryptedTicket";
		}

		return "unknownTicket";
	}

	class SavedTicket
	{
public:
		CSteamId steamId;
		std::string ticket;

		constexpr bool isValid() const
		{
			return steamId.isSet() && ticket.size() > 0;
		}
	};

	extern std::unordered_map<AppId_t, CSteamId> oneTimeSteamIdSpoof;
	extern std::unordered_map<AppId_t, SavedTicket> ticketMap;
	extern std::unordered_map<AppId_t, SavedTicket> encryptedTicketMap;

	constexpr std::unordered_map<AppId_t, Ticket::SavedTicket>& getTicketMap(const ETicketType type)
	{
		switch (type)
		{
			case ETicketType::AppOwnershipTicket:
				return ticketMap;

			case ETicketType::EncryptedAppOwnershipTicket:
				return encryptedTicketMap;
		}
	}

	std::filesystem::path getTicketDir();

	//TODO: Fill with error checks
	std::filesystem::path getTicketPath(const AppId_t appId, const ETicketType type);
	SavedTicket* getCachedTicket(const AppId_t appId, const ETicketType type);
	bool saveTicketToCache(const AppId_t appId, const std::string& bytes, const CSteamId& owner, const ETicketType type);

	//Hooks
	void closePipe(const HSteamPipe pipe);
	void connectPipe(const HSteamPipe pipe);
	void launchApp(const AppId_t appId);
	void getEncryptedAppTicket(const AppId_t appId);
	void getTicketOwnershipExtendedData(const AppId_t appId);

	//Netpacket stuff
	void recvAppTicket(const CNetPacket* pkt);
	void recvEncryptedAppTicket(CNetPacket* pkt);

	void recvMsg(CNetPacket* pkt);
}
