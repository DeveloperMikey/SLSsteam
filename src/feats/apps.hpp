#pragma once

#include <cstdint>
#include <unordered_set>

class CNetPacket;
class CMsgClientGamesPlayed;
class CMsgClientPICSProductInfoRequest;
class CMsgClientPICSProductInfoResponse;
class CPlayer_GetLastPlayedTimes_Response;

template<typename T> class CUtlVector;

struct AppOwnershipInfo_t;
struct DepotInfo_t;

namespace Apps
{
	extern bool applistRequested;
	extern uint32_t lastAppLaunched;

	bool unlockApp(uint32_t appId, AppOwnershipInfo_t* info, uint32_t ownerId);
	bool unlockApp(uint32_t appId, AppOwnershipInfo_t* info);

	void buildDepotDependency(uint32_t appId, CUtlVector<DepotInfo_t>* depots, CUtlVector<DepotInfo_t>* sharedDepots);
	bool checkAppOwnership(uint32_t appId, AppOwnershipInfo_t* info);
	void getSubscribedApps(uint32_t* appList, uint32_t size, uint32_t& count);
	void parseProductInfoFromResponse(CMsgClientPICSProductInfoResponse* msg);
	void runIPCFrame();

	void postAppLicensesChanged(const std::unordered_set<uint32_t>& apps);

	bool shouldDisableCloud(uint32_t appId);
	bool shouldDisableCDKey(uint32_t appId);
	bool shouldDisableUpdates(uint32_t appId);

	void recvPersonaState(CNetPacket* pkt);
	void recvMsg(CNetPacket* pkt);

	void sendAndRecvLastPlayedTimes(const char* name, CPlayer_GetLastPlayedTimes_Response* recv);
	void sendGamesPlayed(CNetPacket* pkt);
	void sendPICSInfoRequest(CNetPacket* pkt);
	void sendRichPresenceUpload(CNetPacket* pkt);
	void sendMsg(CNetPacket* pkt);
};
