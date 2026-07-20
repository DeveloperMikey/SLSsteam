#pragma once

#include "../sdk/steam.hpp"

#include <cstdint>
#include <map>
#include <string>


class CMsgClientGetAppOwnershipTicketResponse;
class CMsgClientRequestEncryptedAppTicketResponse;
class CNetPacket;

namespace Ticket
{
	class SavedTicket
	{
public:
		uint32_t steamId;
		std::string ticket;
	};

	extern uint32_t oneTimeSteamIdSpoof;
	extern std::map<AppId_t, SavedTicket> ticketMap;
	extern std::map<AppId_t, SavedTicket> encryptedTicketMap;

	std::string getTicketDir();

	//TODO: Fill with error checks
	std::string getTicketPath(AppId_t appId);
	SavedTicket getCachedTicket(AppId_t appId);
	bool saveTicketToCache(const CMsgClientGetAppOwnershipTicketResponse* resp);

	void launchApp(AppId_t appId);
	void getTicketOwnershipExtendedData(AppId_t appId);

	std::string getEncryptedTicketPath(AppId_t appId);
	SavedTicket getCachedEncryptedTicket(AppId_t appId);
	bool saveEncryptedTicketToCache(CMsgClientRequestEncryptedAppTicketResponse* resp);

	void recvEncryptedAppTicket(CNetPacket* pkt);
	void recvAppTicket(const CNetPacket* pkt);
	void recvMsg(CNetPacket* pkt);
}
