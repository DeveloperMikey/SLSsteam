#include "fakeappid.hpp"

#include "../config.hpp"

#include "../sdk/CNetPacket.hpp"
#include "../sdk/CSteamEngine.hpp"
#include "../sdk/CSteamMatchmakingServers.hpp"
#include "../sdk/CUser.hpp"
#include "../sdk/IClientUtils.hpp"

AppId_t FakeAppIds::lastAppLaunched;

std::unordered_map<uint32_t, AppId_t> FakeAppIds::fakeAppIdMap = std::unordered_map<AppId_t, AppId_t>();
std::unordered_map<uint32_t, AppId_t> FakeAppIds::fakeAppIdMapServer = std::unordered_map<uint32_t, AppId_t>();
std::unordered_map<uint64_t, AppId_t> FakeAppIds::fakeAppIdMapPings = std::unordered_map<uint64_t, AppId_t>();

AppId_t FakeAppIds::getFakeAppId(const AppId_t appId)
{
	const auto fakeAppIds = g_config.fakeAppIds.get();

	if (fakeAppIds.contains(appId))
	{
		return fakeAppIds.at(appId);
	}
	else if (fakeAppIds.contains(0) && !g_pSteamEngine->getUser(0)->isSubscribed(appId))
	{
		return fakeAppIds.at(0);
	}

	return 0;
}

AppId_t FakeAppIds::getRealAppIdForCurrentPipe(const bool fallback)
{
	if (!g_pClientUtils)
	{
		return 0;
	}

	const HSteamPipe hPipe = g_pClientUtils->getCurrentSteamPipe();
	if (fakeAppIdMap.contains(hPipe))
	{
		return fakeAppIdMap[hPipe];
	}

	if (fallback)
	{
		return g_pClientUtils->getAppId();
	}

	return 0;
}

bool FakeAppIds::shouldUseRealAppIdForInterface(const EInterfaceType type)
{
	switch(type)
	{
		//case k_EInterfaceTypeClientUser:
		//case k_EInterfaceTypeClientGameServerInternal:
		//case k_EInterfaceTypeClientFriends:
		case k_EInterfaceTypeClientUtils:
		case k_EInterfaceTypeClientBilling:
		//case k_EInterfaceTypeClientMatchmaking:
		case k_EInterfaceTypeClientApps:
		case k_EInterfaceTypeClientUserStats:
		//case k_EInterfaceTypeClientNetworking:
		case k_EInterfaceTypeClientRemoteStorage:
		case k_EInterfaceTypeClientDepotBuilder:
		case k_EInterfaceTypeClientAppManager:
		case k_EInterfaceTypeClientConfigStore:
		//case k_EInterfaceTypeClientGameCoordinator:
		//case k_EInterfaceTypeClientGameServerStats:
		case k_EInterfaceTypeClientGameStats:
		case k_EInterfaceTypeClientHTTP:
		case k_EInterfaceTypeClientScreenshots:
		case k_EInterfaceTypeClientAudio:
		case k_EInterfaceTypeClientUnifiedMessages:
		case k_EInterfaceTypeClientStreamLauncher:
		case k_EInterfaceTypeClientParentalSettings:
		case k_EInterfaceTypeClientNetworkDeviceManager:
		case k_EInterfaceTypeClientMusic:
		case k_EInterfaceTypeClientRemoteClientManager:
		case k_EInterfaceTypeClientUGC:
		case k_EInterfaceTypeClientStreamClient:
		case k_EInterfaceTypeClientProductBuilder:
		case k_EInterfaceTypeClientShortcuts:
		case k_EInterfaceTypeClientGameNotifications:
		case k_EInterfaceTypeClientVideo:
		case k_EInterfaceTypeClientInventory:
		case k_EInterfaceTypeClientVR:
		case k_EInterfaceTypeClientControllerSerialized:
		case k_EInterfaceTypeClientAppDisableUpdate:
		case k_EInterfaceTypeClientSharedConnection:
		case k_EInterfaceTypeClientShader:
		//case k_EInterfaceTypeClientNetworkingSocketsSerialized:
		case k_EInterfaceTypeClientCompat:
		case k_EInterfaceTypeClientParties:
		//case k_EInterfaceTypeClientNetworkingUtilsSerialized:
		case k_EInterfaceTypeClientRemotePlay:
		//case k_EInterfaceTypeClientGameServerPacketHandler:
		case k_EInterfaceTypeClientSystemManager:
		case k_EInterfaceTypeClientSystemPerfManager:
		case k_EInterfaceTypeClientSystemDockManager:
		case k_EInterfaceTypeClientSystemAudioManager:
		case k_EInterfaceTypeClientSystemDisplayManager:
		case k_EInterfaceTypeClientTimeline:
			return true;

		default:
			return false;
	}
}

void FakeAppIds::launchApp(const AppId_t appId)
{
	lastAppLaunched = appId;
}

void FakeAppIds::setAppIdForCurrentPipe(AppId_t& appId)
{
	//Keep track of every AppId, for various reasons
	//fakeAppIdMap[*g_pClientUtils->getPipeIndex()] = appId;
	fakeAppIdMap[g_pClientUtils->getCurrentSteamPipe()] = lastAppLaunched;

	g_pLog->debug("fakeAppIdMap[%p] = %u\n", g_pClientUtils->getCurrentSteamPipe(), lastAppLaunched);

	//Do not change Steam Client itself (AppId 0)
	if (!appId)
	{
		return;
	}

	const AppId_t newAppId = getFakeAppId(appId);
	if (newAppId)
	{
		g_pLog->once("Changing AppId of %u\n", appId);
		appId = newAppId;
	}
}

void FakeAppIds::runIPCFrame(const bool post)
{
	AppId_t appId = getRealAppIdForCurrentPipe(false);
	const AppId_t fakeAppId = getFakeAppId(appId);

	if (!appId || !fakeAppId || appId == fakeAppId)
	{
		return;
	}

	if (post)
	{
		appId = fakeAppId;
	}

	if (g_config.extendedLogging.get())
	{
		g_pLog->debug("Setting AppId to %u in pipe %p\n", appId, g_pClientUtils->getCurrentSteamPipe());
	}
	g_pSteamEngine->setAppIdForCurrentPipe(appId);
}

void FakeAppIds::getServerDetails(const uint32_t handle, gameserverdetails_t& details)
{
	if (!fakeAppIdMapServer.contains(handle))
	{
		return;
	}

	const AppId_t realAppId = fakeAppIdMapServer[handle];
	fakeAppIdMapPings[*reinterpret_cast<uint64_t*>(&details.address)] = realAppId;
	details.appId = realAppId;

	g_pLog->debug("Changing appId back to %u\n", realAppId);
}

uint32_t FakeAppIds::requestInternetServerList(const AppId_t appId)
{
	const AppId_t fake = getFakeAppId(appId);
	if (!fake)
	{
		return 0;
	}

	g_pLog->debug("Replacing %u with %u\n", appId, fake);
	return fake;
}

void FakeAppIds::pingResponse(gameserverdetails_t *details)
{
	if (!details)
	{
		return;
	}

	const uint64_t ip = *reinterpret_cast<uint64_t*>(&details->address);
	if (!fakeAppIdMapPings.contains(ip))
	{
		return;
	}

	details->appId = fakeAppIdMapPings[ip];
}

void FakeAppIds::sendGamesPlayed(CNetPacket *pkt)
{
	auto msg = pkt->deserializeBody<CMsgClientGamesPlayed>();

	for(int i = 0; i < msg.games_played_size(); i++)
	{
		const auto game = msg.mutable_games_played(i);
		const uint64_t gameId = game->game_id();

		if (gameId & 0x2000000ULL)
		{
			continue;
		}

		const AppId_t fakeAppId = FakeAppIds::getFakeAppId(gameId);
		if (!fakeAppId)
		{
			continue;
		}

		g_pLog->debug("Setting %llu to %u\n", gameId, fakeAppId);
		game->set_game_id(fakeAppId);
	}

	pkt->serialize(msg);
}

void FakeAppIds::sendRichPresenceUpload(CNetPacket* pkt)
{
	auto header = pkt->deserializeHeader();
	g_pLog->debug("Routing appId %u\n", header.routing_appid());

	const auto appId = getFakeAppId(header.routing_appid());

	if (!appId)
	{
		return;
	}

	//This won't fix localized rich presences, but it's better than nothing
	header.set_routing_appid(appId);

	auto msg = pkt->deserializeBody<CMsgClientRichPresenceUpload>();
	pkt->serialize(msg, &header);
}

void FakeAppIds::sendMsg(CNetPacket* pkt)
{
	switch(pkt->getProtoBufType())
	{
		case k_EMsgClientGamesPlayed:
		case k_EMsgClientGamesPlayedNoDataBlob:
		case k_EMsgClientGamesPlayedWithDataBlob:
			sendGamesPlayed(pkt);
			break;
		
		case k_EMsgClientRichPresenceUpload:
			sendRichPresenceUpload(pkt);
			break;

		default:
			break;
	}
}
