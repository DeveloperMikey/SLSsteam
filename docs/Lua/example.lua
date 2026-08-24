local ffi = require("ffi")

ffi.cdef[[
	typedef void(*PostCallback_t)(void*, uint32_t, void*, uint32_t, uint32_t);
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

local config = SLS.config

function addappid(appid)
	local apps = config:getAdditionalApps(config)
	table.insert(apps, appid)
	log.debug("Added app " .. appid)
	config:setAdditionalApps(apps)
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

log.notify("Luas loaded!")
