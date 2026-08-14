#pragma once

#include "types.hpp"

#include <cstdint>
#include <vector>


class IClientCompat
{
public:
	//Set name to "" to unset
	void specifyCompatTool(const AppId_t appId, const char* name, const char* config, int32_t priority);
};

extern IClientCompat* g_pClientCompat;
