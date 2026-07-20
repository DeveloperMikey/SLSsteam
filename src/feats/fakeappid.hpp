#pragma once

#include "../sdk/steam.hpp"

#include <cstdint>
#include <unordered_map>


class CNetPacket;

struct gameserverdetails_t;
struct servernetadr_t;

namespace FakeAppIds
{
	extern AppId_t lastAppLaunched;

	extern std::unordered_map<uint32_t, AppId_t> fakeAppIdMap;
	extern std::unordered_map<uint32_t, AppId_t> fakeAppIdMapServer;
	extern std::unordered_map<uint64_t, AppId_t> fakeAppIdMapPings;

	AppId_t getFakeAppId(AppId_t appId);
	AppId_t getRealAppIdForCurrentPipe(bool fallback = true);

	//General functionality
	void launchApp(AppId_t appId);
	void setAppIdForCurrentPipe(AppId_t& appId);
	void runIPCFrame(bool post);

	//Serverbrowser
	void getServerDetails(uint32_t handle, gameserverdetails_t& details);
	uint32_t requestInternetServerList(AppId_t appId);
	void pingResponse(gameserverdetails_t* details);

	void sendGamesPlayed(CNetPacket* pkt);
	void sendRichPresenceUpload(CNetPacket* pkt);
	void sendMsg(CNetPacket* pkt);
}
