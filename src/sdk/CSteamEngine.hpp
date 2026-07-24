#pragma once

#include "steam.hpp"

#include <cstdint>


class CUser;
class IClientUtils;

class CSteamEngine
{
public:
	CUser* getUser(const uint32_t index = 0);
	IClientUtils* getUtils();
	void setAppIdForCurrentPipe(const AppId_t appId);
};

extern CSteamEngine* g_pSteamEngine;
