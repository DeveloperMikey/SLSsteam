#pragma once

#include "steam.hpp"

#include <cstdint>

enum EInterfaceType : uint32_t
{
	k_EInterfaceTypeClientUser = 0x1,
	k_EInterfaceTypeClientGameServerInternal = 0x2,
	k_EInterfaceTypeClientFriends = 0x3,
	k_EInterfaceTypeClientUtils = 0x4,
	k_EInterfaceTypeClientBilling = 0x5,
	k_EInterfaceTypeClientMatchmaking = 0x6,
	k_EInterfaceTypeClientApps = 0x8,
	k_EInterfaceTypeClientUserStats = 0xb,
	k_EInterfaceTypeClientNetworking = 0xc,
	k_EInterfaceTypeClientRemoteStorage = 0xd,
	k_EInterfaceTypeClientDepotBuilder = 0x10,
	k_EInterfaceTypeClientAppManager = 0x11,
	k_EInterfaceTypeClientConfigStore = 0x12,
	k_EInterfaceTypeClientGameCoordinator = 0x13,
	k_EInterfaceTypeClientGameServerStats = 0x14,
	k_EInterfaceTypeClientGameStats = 0x15,
	k_EInterfaceTypeClientHTTP = 0x16,
	k_EInterfaceTypeClientScreenshots = 0x17,
	k_EInterfaceTypeClientAudio = 0x18,
	k_EInterfaceTypeClientUnifiedMessages = 0x19,
	k_EInterfaceTypeClientStreamLauncher = 0x1a,
	k_EInterfaceTypeClientParentalSettings = 0x1b,
	k_EInterfaceTypeClientNetworkDeviceManager = 0x1d,
	k_EInterfaceTypeClientMusc = 0x1e,
	k_EInterfaceTypeClientRemoteClientManager = 0x1f,
	k_EInterfaceTypeClientUGC = 0x20,
	k_EInterfaceTypeClientStreamClient = 0x21,
	k_EInterfaceTypeClientProductBuilder = 0x22,
	k_EInterfaceTypeClientShortcuts = 0x23,
	k_EInterfaceTypeClientGameNotifications = 0x25,
	k_EInterfaceTypeClientVideo = 0x26,
	k_EInterfaceTypeClientInventory = 0x27,
	k_EInterfaceTypeClientVR = 0x28,
	k_EInterfaceTypeClientControllerSerialized = 0x29,
	k_EInterfaceTypeClientAppDisableUpdate = 0x2a,
	k_EInterfaceTypeClientSharedConnection = 0x2c,
	k_EInterfaceTypeClientShader = 0x2d,
	k_EInterfaceTypeClientNetworkingSocketsSerialized = 0x2e,
	k_EInterfaceTypeClientCompat = 0x30,
	k_EInterfaceTypeClientParties = 0x31,
	k_EInterfaceTypeClientNetworkingUtilsSerialized = 0x32,
	k_EInterfaceTypeClientRemotePlay = 0x34,
	k_EInterfaceTypeClientGameServerPacketHandler = 0x35,
	k_EInterfaceTypeClientSystemManager = 0x36,
	k_EInterfaceTypeClientSystemPerfManager = 0x39,
	k_EInterfaceTypeClientSystemDockManager = 0x3a,
	k_EInterfaceTypeClientSystemAudioManager = 0x3b,
	k_EInterfaceTypeClientSystemDisplayManager = 0x3c,
	k_EInterfaceTypeClientTimeline = 0x3d
};


class CUser;

class CSteamEngine
{
public:
	CUser* getUser(const uint32_t index);
	void setAppIdForCurrentPipe(const AppId_t appId);
};

extern CSteamEngine* g_pSteamEngine;
