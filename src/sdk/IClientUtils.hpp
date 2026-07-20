#pragma once

#include "steam.hpp"

#include <cstdint>


class IClientUtils
{
public:
	uint32_t* getPipeIndex();
	AppId_t getAppId();
};

extern IClientUtils* g_pClientUtils;
