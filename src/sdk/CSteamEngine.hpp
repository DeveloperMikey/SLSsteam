#pragma once

#include "steam.hpp"

#include <cstdint>


class CUser;
class IClientUtils;

class CServerPipe
{
public:
	uint8_t __pad0x0[0x14];	//0x0
	uint32_t pid;			//0x14
};

class CSteamEngine
{
public:
	CServerPipe* getServerPipe(const HSteamPipe pipe);
	CUser* getUser(const uint32_t index = 0);
	IClientUtils* getUtils();

	void setAppIdForCurrentPipe(const AppId_t appId);
};

extern CSteamEngine* g_pSteamEngine;
