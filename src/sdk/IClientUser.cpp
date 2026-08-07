#include "IClientUser.hpp"

#include "../hooks.hpp"


bool IClientUser::loggedOn()
{
	return Hooks::IClientUser_BLoggedOn.originalFn.fn(this);
}

uint32_t IClientUser::getAppOwnershipTicketExtendeData
(
	const AppId_t appId,
	void* pTicket,
	const uint32_t ticketSize,
	uint32_t* pOffAppId,
	uint32_t* pOffSteamId,
	uint32_t* pOffSig,
	uint32_t* pSigSize
)
{
	return Hooks::IClientUser_GetAppOwnershipTicketExtendedData.originalFn.fn(this, appId, pTicket, ticketSize, pOffAppId, pOffSteamId, pOffSig, pSigSize);
}
