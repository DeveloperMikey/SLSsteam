#pragma once

#include "CNetPacket.hpp"


class CCMInterface
{
public:
	void recvPkt(CNetPacket* pkt);
};

extern CCMInterface* g_pCMInterface;
