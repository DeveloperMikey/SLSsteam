-- Please for the love of god add an include guard to only run your code once
-- In debug releases the LuaState gets rebuild on every hot reload
-- But in release builds it does not to prevent hooking over and over
-- while the steamclient is potentially executing code we are in the process
-- of hooking
if ExampleSetup then
	return
end
ExampleSetup = true

local ffi = require("ffi")

ffi.cdef[[
	typedef void(*PostCallback_t)(void*, uint32_t, void*, uint32_t, uint32_t);
	typedef bool(*IClientUser_BLoggedOn_t)(void*);
]]

log.debug("Luas loading :)")

-- Define global Hooks table if it does not exist. We store our LuaHooks
-- in it to prevent Lua's garbage collector from removing our hooks
Hooks = Hooks or {}

local modSteamClient = memhlp.getModule("steamclient.so")

local postCallbackPtr = memhlp.getJmpTarget(memhlp.patternScan("E8 ? ? ? ? 8B 75 ? 89 D8", modSteamClient))

local trampFn

local function hkPostCallback(user, type, pCallback, callbackSize, a4)
	-- Lua is not thread safe, so we use a recursive_mutex to prevent
	-- multiple threads using the same lua_State simultaneously 
	-- LuaMutex() locks automatically, ~LuaMutex() unlocks automatically
	-- but since Lua doesn't run the garbage collector as soon as the function ends we call unlock manually
	-- SLSsteam will automatically lock the shared LuaMutex before firing a callback/rerunning Luas
	local mutex = LuaMutex()

	log.debug("PostCallback " .. type)
	trampFn(user, type, pCallback, callbackSize, a4)

	mutex:unlock()
end

local detourFn = ffi.cast("PostCallback_t", hkPostCallback)
local lh = LuaHook("PostCalback", tonumber(postCallbackPtr), tonumber(ffi.cast("intptr_t", detourFn)))

table.insert(Hooks, lh)

trampFn = ffi.cast("PostCallback_t", lh:place())
log.debug("Postcallback hooked!")

-- We cast to intptr_t since lua messes up the conversion to unsigned integer otherwise
local clientUserMapLoggedOn = VFTableInfo_t("14IClientUserMap", "BLoggedOn", tonumber(ffi.cast("intptr_t", 0xffffffff)), tonumber(ffi.cast("intptr_t", 0xffffffff)))
if not clientUserMapLoggedOn:init() then
	log.notify("Failed to parse IClientUserMap!")
end

-- IClientUser is subclass 1 of CUser
local clientUserLoggedOn = VFTableInfo_t("5CUser", "BLoggedOn", clientUserMapLoggedOn.index, 0)
clientUserLoggedOn:init()
local clientUserLoggedOnFn = ffi.cast("IClientUser_BLoggedOn_t", clientUserLoggedOn.address)

local injectedApps = {}

local function initialized()
	local config = SLS.config
	local engine = SLS.steamEngine
	local user = engine:getUser(0)
	local clientUser = user:getClientUser()
	local apps = user:getClientApps()

	-- This line is confusing, I am sure we can do better in the future
	-- We do this because the lua object is wrapped in a UserData object from LuaBridge
	-- So to get the actual object it's pointing at we need to convert it
	-- Otherwise calling the resolved BLoggedOn with it will read the from the wrong memory address
	local rawClientUser = ffi.cast("void*", memhlp.getUserDataPtr(tonumber(ffi.cast("intptr_t", clientUser))))

	log.debug("IClientUser::BLoggedOn -> " .. tostring(clientUser:loggedOn())) -- SLS wrapped function call
	-- This is just an example how to call arbitrary functions
	log.debug("IClientUser::BLoggedOn -> " .. tostring(clientUserLoggedOnFn(rawClientUser))) -- Raw function call

	local function addappid(appId)
		if user:isSubscribed(appId) then
			log.debug(appId .. " is already subscribed! Not adding to additionalApps...")
		end

		local appList = config:getAdditionalApps()
		table.insert(appList, appId)
		table.insert(injectedApps, appId)
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

	log.custom(log.LogLevelDebug | log.LogLevelOnce, "Custom deduplicated log")

	log.info("Lua apps added!")
end

local function configLoaded()
	local config = SLS.config
	local someList = config:getIntList("TestAppIds")

	for k, v in ipairs(someList) do
		log.debug("TestAppIds " .. k .. " -> " .. v)
	end

	log.info("Lua config loaded")
end

SLS.registerCallback("SLSsteam::initialized", initialized)
SLS.registerCallback("SLSsteam::configLoaded", configLoaded)
log.notify("example.lua loaded!")
