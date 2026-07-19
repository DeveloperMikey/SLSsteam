#pragma once

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
	extern std::map<uint32_t, SavedTicket> ticketMap;
	extern std::map<uint32_t, SavedTicket> encryptedTicketMap;

	std::string getTicketDir();

	//TODO: Fill with error checks
	std::string getTicketPath(uint32_t appId);
	SavedTicket getCachedTicket(uint32_t appId);
	bool saveTicketToCache(const CMsgClientGetAppOwnershipTicketResponse* resp);

	void launchApp(uint32_t appId);
	void getTicketOwnershipExtendedData(uint32_t appId);

	std::string getEncryptedTicketPath(uint32_t appId);
	SavedTicket getCachedEncryptedTicket(uint32_t appId);
	bool saveEncryptedTicketToCache(CMsgClientRequestEncryptedAppTicketResponse* resp);

	void recvEncryptedAppTicket(CNetPacket* pkt);
	void recvAppTicket(const CNetPacket* pkt);
	void recvMsg(CNetPacket* pkt);
}
