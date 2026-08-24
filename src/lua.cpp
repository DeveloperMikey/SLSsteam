#include "lua.hpp"

#include "sdk/sdk.hpp"

#include "config.hpp"
#include "hooks.hpp"
#include "log.hpp"
#include "memhlp.hpp"
#include "vftableinfo.hpp"

#include <filesystem>
#include <unordered_map>

extern "C"
{
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
}

#include "LuaBridge/Array.h"
#include "LuaBridge/List.h"
#include "LuaBridge/UnorderedSet.h"


void onFileChange(__attribute__((unused)) const std::filesystem::path& path)
{
	Lua::init();

	if (Hooks::IClientUtils_GetOfflineMode.hooked) //Ghetto way to check wheter our hooks are setup
	{
		Lua::fireCallback("SLSsteam::initialized");
	}
}

lua_State* Lua::state;
std::unique_ptr<CFileWatcher> Lua::watcher = std::make_unique<CFileWatcher>(onFileChange);
std::unordered_map<std::string, std::vector<luabridge::LuaRef>> Lua::callbacks = std::unordered_map<std::string, std::vector<luabridge::LuaRef>>();

namespace LuaConfig
{
	CConfig* get()
	{
		return &g_config;
	}

	auto getAdditionalApps(CConfig* config)
	{
		return config->addedAppIds.get();
	}
}

namespace LuaLog
{
	static void debug(const char* msg)
	{
		LOG_DEBUG("%s\n", msg);
	}

	static void info(const char* msg)
	{
		LOG_INFO("%s\n", msg);
	}

	static void notify(const char* msg)
	{
		LOG_NOTIFY("%s", msg);
	}
}

namespace LuaSDK
{
	CSteamEngine* getEngine()
	{
		return g_pSteamEngine;
	}

	std::string getAppData(IClientApps* apps, const AppId_t appId, const char* name)
	{
		char buf[4096] { };
		size_t size = apps->getAppData(appId, name, buf, sizeof(buf));
		return std::string(buf, size);
	}
}

void Lua::init()
{
	callbacks.clear();

	if (state)
	{
		lua_close(state);
	}

	state = luaL_newstate();
	luaL_openlibs(state);

	luabridge::getGlobalNamespace(state)

	.beginNamespace("log")
		.addFunction("debug", &LuaLog::debug)
		.addFunction("info", &LuaLog::info)
		.addFunction("notify", &LuaLog::notify)
	.endNamespace()

	.beginClass<lm_module_t>("lm_module_t")
		.addProperty("base", &lm_module_t::base)
		.addProperty("end", &lm_module_t::end)
		.addProperty("size", &lm_module_t::size)
	.endClass()

	.beginNamespace("memhlp")
		.addFunction("getModule", &MemHlp::getModule)
		.addFunction("getJmpTarget", &MemHlp::getJmpTarget)
		//.addFunction("hexdump", &MemHlp::hexdump)
		.addFunction("findPrologue", &MemHlp::findPrologue)
		.addFunction("patternScan", &MemHlp::patternScan)
	.endNamespace()

	.beginClass<VFTableInfo_t>("VFTableInfo_t")
		.addConstructor<void(*)(const char*, const char*, unsigned int)>()
		.addProperty("typeName", &VFTableInfo_t::typeName)
		.addProperty("functionName", &VFTableInfo_t::functionName)
		.addProperty("address", &VFTableInfo_t::address)
		.addProperty("index", &VFTableInfo_t::index)
		.addFunction("init", &VFTableInfo_t::init)
		.addFunction("getPrintName", &VFTableInfo_t::getPrintName)
	.endClass()

	.beginClass<LuaHook>("LuaHook")
		.addConstructor<void(*)(const char*, const lm_address_t, const lm_address_t)>()
		.addProperty("name", &LuaHook::name)
		.addProperty("fn", &LuaHook::fn)
		.addProperty("hookFn", &LuaHook::hookFn)
		.addProperty("tramp", &LuaHook::tramp)
		.addProperty("size", &LuaHook::size)
		.addFunction("place", &LuaHook::place)
		.addFunction("remove", &LuaHook::remove)
	.endClass()

	.beginClass<CConfig>("CConfig")
		.addFunction("getAdditionalApps", &LuaConfig::getAdditionalApps)
		.addFunction("setAdditionalApps", &CConfig::setAdditionalApps)
	.endClass()

	.beginClass<CSteamEngine>("CSteamEngine")
		.addFunction("getUser", &CSteamEngine::getUser)
		.addFunction("getUtils", &CSteamEngine::getUtils)
	.endClass()

	.beginClass<CUser>("CUser")
		.addFunction("getClientApps", &CUser::getClientApps)
		.addFunction("getClientUser", &CUser::getClientUser)
		.addFunction("getAppManager", &CUser::getAppManager)
		.addFunction("isSubscribed", &CUser::isSubscribed)
	.endClass()

	.beginClass<IClientApps>("IClientApps")
		.addFunction("getAppData", &LuaSDK::getAppData)
		.addFunction("getAppType", &IClientApps::getAppType)
	.endClass()

	.beginClass<IClientUser>("IClientUser")
		.addFunction("loggedOn", &IClientUser::loggedOn)
	.endClass()

	.beginClass<IClientUtils>("IClientUtils")
		.addFunction("getAppId", &IClientUtils::getAppId)
		.addFunction("getCurrentSteamPipe", &IClientUtils::getCurrentSteamPipe)
	.endClass()

	.beginNamespace("SLS")
		.addProperty("config", &LuaConfig::get)
		.addProperty("steamEngine", &LuaSDK::getEngine)
		.addFunction("registerCallback", &Lua::registerCallback)
	.endNamespace();

	auto dir = std::filesystem::path(CConfig::getDir());
	dir.append("plugins");

	for (const auto& lua : std::filesystem::directory_iterator { dir })
	{
		runLua(lua);
	}

	if (watcher->fileFdMap.size() < 1)
	{
		if (watcher->addFile(dir.c_str()) != -1)
		{
			watcher->start();
		}
		else
		{
			LOG_NOTIFYERROR("Failed to watch plugin directory!\nHot reload will be unavailable");
		}
	}

	LOG_DEBUG("Lua initialized\n");
}

bool Lua::runLua(const std::filesystem::path& path)
{
	if (luaL_dofile(state, path.c_str()) != LUA_OK)
	{
		LOG_ERROR("Failed to run %s!\n%s\n", path.filename().c_str(), lua_tostring(state, -1));
		return false;
	}

	return true;
}

void Lua::registerCallback(const std::string& name, luabridge::LuaRef fn)
{
	callbacks[name].emplace_back(fn);
	LOG_DEBUG("Registered lua callback for %s\n", name.c_str());
}
