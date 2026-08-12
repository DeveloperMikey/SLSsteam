#pragma once

#include "../sdk/sdk.hpp"

class CNetPacket;

namespace Misc
{
	bool shouldFakeOffline();
	void recvMsg(CNetPacket* pkt);
}
