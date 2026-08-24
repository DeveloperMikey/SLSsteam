local ffi = require("ffi")

ffi.cdef[[
	typedef void(*PostCallback_t)(void*, uint32_t, void*, uint32_t, uint32_t);
	typedef bool(*IClientUser_BLoggedOn_t)(void*);
]]

log.debug("Luas loading :)")

local modSteamClient = memhlp.getModule("steamclient.so")

local postCallbackPtr = memhlp.getJmpTarget(memhlp.patternScan("E8 ? ? ? ? 8B 75 ? 89 D8", modSteamClient))

local trampFn;

local function hkPostCallback(user, type, pCallback, callbackSize, a4)
	log.debug("PostCallback " .. type)
	trampFn(user, type, pCallback, callbackSize, a4)
end

local detourFn = ffi.cast("PostCallback_t", hkPostCallback)
local lh = LuaHook("PostCalback", tonumber(postCallbackPtr), tonumber(ffi.cast("intptr_t", detourFn)))

trampFn = ffi.cast("PostCallback_t", lh:place())
log.debug("Postcallback hooked!")

-- We cast to intptr_t since lua messes up the conversion to unsigned integer otherwise
local clientUserMapLoggedOn = VFTableInfo_t("14IClientUserMap", "BLoggedOn", tonumber(ffi.cast("intptr_t", 0xffffffff)))
if not clientUserMapLoggedOn:init() then
	log.notify("Failed to parse IClientUserMap!")
end

-- IClientUser is subclass 1 of CUser
local clientUserLoggedOn = VFTableInfo_t("5CUser", "BLoggedOn", clientUserMapLoggedOn.index, 1)
clientUserLoggedOn:init()
local clientUserLoggedOnFn = ffi.cast("IClientUser_BLoggedOn_t", clientUserLoggedOn.address)

local function initialized()
	local config = SLS.config
	local engine = SLS.steamEngine
	local user = engine:getUser(0)
	local clientUser = user:getClientUser()
	local apps = user:getClientApps()

	-- This is just an example how to add arbitrary functions, you can also use clientUser:loggedOn
	local loggedOn = clientUserLoggedOnFn(clientUser)

	log.debug("IClientUser::BLoggedOn -> " .. tostring(loggedOn))

	function addappid(appId)
		if user:isSubscribed(appId) then
			log.debug(appId .. " is already subscribed! Not adding to additionalApps...")
		end

		local appList = config:getAdditionalApps(config)
		table.insert(appList, appId)
		log.debug("Added app " .. appId)
		config:setAdditionalApps(appList)
	end

	addappid(236430) --  #DARK SOULS™ II
	addappid(271940) -- # Dark Souls II - Pre-Order DLC ROW
	addappid(271941) -- # Dark Souls II - Digital Extras
	addappid(271942) -- # Dark Souls™ II Crown of the Sunken King
	addappid(271943) -- # DARK SOULS™ II Crown of the Old Iron King
	addappid(271944) -- # DARK SOULS™ II Crown of the Ivory King
	addappid(284450) -- # DARK SOULS™ II - Season Pass
	addappid(287360) -- # Dark Souls II - Pre-Order DLC JP
	addappid(287770) -- # Darks Souls II JP Retail Pre-Order DLC 1
	addappid(287771) -- # Darks Souls II JP Retail Pre-Order DLC 2
	addappid(289120) -- # Darks Souls II JP Retail Pre-Order DLC 3
	addappid(289121) -- # Darks Souls II JP Retail Pre-Order DLC 4
	addappid(289122) -- # Darks Souls II JP Retail Pre-Order DLC 5
	addappid(355700) -- # Dark Souls II Upgrade to DX11 (no content)

	log.notify("Lua apps added!")
end

SLS.registerCallback("SLSsteam::initialized", initialized)
log.notify("Luas loaded!")
