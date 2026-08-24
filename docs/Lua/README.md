# Plugin API

SLSsteam now embeds LuaJIT which allows doing basically everything paired with FFI & SLSsteam's exposed functions.
Plugins get automatically read from SLSsteam's plugin directory (config/plugins).

If you want to dive right in without reading the docs check out the example.lua in the same directory this README is in.


#### typedefs

LuaJIT doesn't have access to all of SLSsteam's internal typedefs. Currently the ones you need to be aware of are:

lm_address_t -> uintptr_t
LogLevelFlags_t -> unsigned int
AppId_t -> uint32_t


#### log:

LogLevelTrace -> LogLevelFlags_t
LogLevelOnce -> LogLevelFlags_t
LogLevelDebug -> LogLevelFlags_t
LogLevelWarn -> LogLevelFlags_t
LogLevelError -> LogLevelFlags_t
LogLevelInfo -> LogLevelFlags_t
LogLevelNotify -> LogLevelFlags_t
LogLevelNotifyLong -> LogLevelFlags_t

log.debug(msg: string): Print debug string to log
log.info(msg: string): Print info string to log
log.notify(msg: string): Create notification via notify-send
log.custom(flags: LogLevelFlags_t, msg: string): Create notification via notify-send


#### lm_module_t:

base -> lm_address_t: Module base
size -> lm_address_t: Module size
end -> lm_address_t: Module end


#### memhlp:

getModule(name: string) -> lm_module_t: Get module by name
getJmpTarget(address: lm_address_t) -> lm_address_t: Get absolute address of relative jmp
hexdump(address: lm_address_t, size: size_t) -> string: Get formatted hexdump
findPrologue(address: lm_address_t, bytes: uint8_t[]) -> lm_address_t: Find prologue of function by going backwards until bytes match
patternScan(pattern: string, module: lm_module_t) -> lm_address_t: Find a pattern in the specified modules .text section


#### VFTableInfo_t

VFTableInfo_t(typename: string, functionName: string, index: unsigned int, subClassIndex: unsigned int): Create new VFTableInfo_t. Pass 0xFFFFFFFF as index to use the decompiler to find it based on the type- & method name (only use this for IClientInterfaceMaps!). Pass the same to subClassIndex when you want the Main Class
typeName -> string
functionName -> string
address -> lm_address_t: The resolved address in current memory
init() -> bool: Initialize, returns true on success, false otherwise. Check the logs for errors
getPrintName() -> string: Returns typeName::functionName


#### LuaHook:

LuaHook(name: string, targetFn: lm_address_t, hookFn: lm_address_t): Create new LuaHook
name -> string: Hook name, solely used for logging
fn -> lm_address_t: Target function address
hookFn -> lm_address_t: Hook function address
tramp -> lm_address_t: Trampoline address
size -> lm_address_t: Stolen bytes, taken away for creating the trampoline
place(): Place the hook
remove(): Remove the hook


#### CConfig

getAdditionalApps() -> AppId_t[]: Gets all AdditionalApps
setAdditionalApps(appIds: AppId_t[]): Sets all AdditionalApps


#### CSteamEngine

getUser(index: int) -> CUser: Get the specified CUser instance. 0 is the global user
getUtils() -> IClientUtils


#### CUser
getClientApps() -> IClientApps
getClientUser() -> IClientUser
getAppManager() -> IClientAppManager
isSubscribed(appId: AppId_t) -> bool
postCallback(type: uint32_t, pCallback: lm_address_t, callbackSize: size_t): Post a callback to the Steamengine & all open pipes


#### IClientUtils

getAppId() -> AppId_t: Return the appId for the currently active pipe
getCurrentSteamPipe() -> HSteamPipe: Return the active pipe handle


#### SLS

config -> CConfig*: Gets SLSsteam config, see [CConfig](#cconfig)
steamEngine -> CSteamEngine*: Gets the Global CSteamEngine instance
registerCallback(name: string, function): Registers a callback, when it gets fired function will be invoked from SLSsteam

alloc(size: int) -> lm_address_t: Allocate memory using Steam's memory allocator
realloc(address: lm_address_t, size: int) -> lm_address_t: Reallocate memory using Steam's memory allocator
free(address: lm_address_t): Free memory using Steam's memory allocator


#### Callbacks

"SLSsteam::initialized": Fired when Steam has finished initializing CUser, making it safe to access
