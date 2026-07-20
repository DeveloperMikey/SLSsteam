#pragma once

#include "../sdk/steam.hpp"

#include <cstddef>
#include <cstdint>


struct AppOwnershipInfo_t;

namespace DLC
{
	bool shouldUnlockDlc(AppId_t appId);

	bool checkAppOwnership(AppId_t appId, AppOwnershipInfo_t* info);
	bool isDlcEnabled(AppId_t appId);
	bool isAppDlcInstalled(AppId_t appId);
	bool userSubscribedInTicket(AppId_t appId);

	uint32_t getDlcCount(AppId_t appId);
	bool getDlcDataByIndex(AppId_t appId, int index, AppId_t* dlcId, bool* available, char* dlcName, size_t& dlcNameLen);
}
