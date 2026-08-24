# Plugin API

SLSsteam now embeds LuaJIT which allows doing basically everything paired with FFI & SLSsteam's exposed functions.
Plugins get automatically read from SLSsteam's plugin directory (config/plugins).

If you want to dive right in without reading the docs check out the example.lua in the same directory this README is in.


#### typedefs

LuaJIT doesn't have access to all of SLSsteam's internal typedefs. Currently the ones you need to be aware of are:

lm_address_t -> uintptr_t
AppId_t -> uint32_t


#### log:

log.debug(msg: string): Print debug string to log
log.info(msg: string): Print info string to log
log.notify(msg: string): Create notification via notify-send


#### lm_module_t:

base -> lm_address_t: Module base
size -> lm_address_t: Module size
end -> lm_address_t: Module end


#### memhlp:

getModule(name: string) -> lm_module_t: Get module by name
getJmpTarget(address: lm_address_t) -> lm_address_t: Get absolute address of relative jmp
findPrologue(address: lm_address_t, bytes: uint8_t[]) -> lm_address_t: Find prologue of function by going backwards until bytes match


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


#### SLS

config -> CConfig*: Gets SLSsteam config, see [CConfig](#cconfig)
