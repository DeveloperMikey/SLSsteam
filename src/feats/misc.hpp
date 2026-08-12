#pragma once

#include "../sdk/sdk.hpp"


namespace Misc
{
	bool shouldFakeOffline();
	void recvMsg(CNetPacket* pkt);
}
