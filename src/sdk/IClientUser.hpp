#pragma once

#include "steam.hpp"

#include <cstdint>


class IClientUser
{
public:

	uint32_t getAppOwnershipTicketExtendeData
	(
		AppId_t appId,
		void* pTicket,
		uint32_t ticketSize,
		uint32_t* pOffAppId,
		uint32_t* pOffSteamId,
		uint32_t* pOffSig,
		uint32_t* pSigSize
	);
};

extern IClientUser* g_pClientUser;
