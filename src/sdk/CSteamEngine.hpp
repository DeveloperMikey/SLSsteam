#pragma once

#include "steam.hpp"

#include <cstdint>


class CUser;

class CSteamEngine
{
public:
	CUser* getUser(uint32_t index);
	void setAppIdForCurrentPipe(AppId_t appId);
};

extern CSteamEngine* g_pSteamEngine;
