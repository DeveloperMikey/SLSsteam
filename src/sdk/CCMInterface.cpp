#include "CCMInterface.hpp"

#include "../hooks.hpp"


void CCMInterface::recvPkt(CNetPacket* pkt)
{
	Hooks::CCMInterface_RecvPkt.hookFn.fn(this, pkt);
}

CCMInterface* g_pCMInterface = nullptr;
