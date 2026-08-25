#include "lua.hpp"

#include "sdk/sdk.hpp"

#include "config.hpp"
#include "hooks.hpp"
#include "log.hpp"
#include "memhlp.hpp"
#include "vftableinfo.hpp"

#include "libmem/libmem.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
	Lua::fireCallback(Lua::Callbacks::SLSsteam_LuaReload);
	Lua::init();
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

	std::unordered_set<AppId_t> getAdditionalApps(CConfig* config)
	{
		return config->addedAppIds.get();
	}

	double getDouble(CConfig* config, const char* name, const double defaultValue)
	{
		return config->getSetting<double>(config->rootNode, name, defaultValue);
	}

	int64_t getInt(CConfig* config, const char* name, const uint64_t defaultValue)
	{
		return config->getSetting<int64_t>(config->rootNode, name, defaultValue);
	}

	std::string getString(CConfig* config, const char* name, const std::string& defaultValue)
	{
		return config->getSetting<std::string>(config->rootNode, name, defaultValue);
	}

	std::unordered_set<double> getDoubleList(CConfig* config, const char* name)
	{
		return config->getList<double>(config->rootNode, name);
	}

	std::unordered_set<int64_t> getIntList(CConfig* config, const char* name)
	{
		return config->getList<int64_t>(config->rootNode, name);
	}

	std::unordered_set<std::string> getStringList(CConfig* config, const char* name)
	{
		return config->getList<std::string>(config->rootNode, name);
	}
}

namespace LuaLog
{
	void debug(const char* msg)
	{
		LOG_DEBUG("%s\n", msg);
	}

	void info(const char* msg)
	{
		LOG_INFO("%s\n", msg);
	}

	void notify(const char* msg)
	{
		LOG_NOTIFY("%s", msg);
	}

	void custom(const LogLevelFlags_t lvl, const char* msg)
	{
		LOG_CUSTOM(lvl, "%s\n", msg);
	}
}

namespace LuaMemHlp
{
	std::string hexdump(const lm_address_t address, const size_t size)
	{
		return MemHlp::hexdump(reinterpret_cast<void*>(address), size);
	}

	lm_address_t getUserDataPtr(lm_address_t data)
	{
		return reinterpret_cast<lm_address_t>(reinterpret_cast<luabridge::detail::Userdata*>(data)->getPointer());
	}
}

namespace LuaSDK
{
	lm_address_t alloc(int size)
	{
		return reinterpret_cast<lm_address_t>(Steam::Plat_Alloc(size));
	}

	lm_address_t realloc(lm_address_t address, int size)
	{
		return reinterpret_cast<lm_address_t>(Steam::Plat_Realloc(reinterpret_cast<void*>(address), size));
	}

	void free(lm_address_t address)
	{
		return Steam::Plat_Free(reinterpret_cast<void*>(address));
	}
	
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

	void postCallback(CUser* pUser, const uint32_t type, const lm_address_t pCallback, const uint32_t callbackSize)
	{
		pUser->postCallback(static_cast<ECallbackType>(type), reinterpret_cast<void*>(pCallback), callbackSize);
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
		.addConstant("LogLevelTrace", 1 << 0)
		.addConstant("LogLevelOnce", 1 << 1)
		.addConstant("LogLevelDebug", 1 << 2)
		.addConstant("LogLevelWarn", 1 << 3)
		.addConstant("LogLevelError", 1 << 4)
		.addConstant("LogLevelInfo", 1 << 5)
		.addConstant("LogLevelNotify", 1 << 6)
		.addConstant("LogLevelNotifyLong", 1 << 7)
		.addFunction("debug", &LuaLog::debug)
		.addFunction("info", &LuaLog::info)
		.addFunction("notify", &LuaLog::notify)
		.addFunction("custom", &LuaLog::custom)
	.endNamespace()

	.beginClass<lm_module_t>("lm_module_t")
		.addProperty("base", &lm_module_t::base)
		.addProperty("end", &lm_module_t::end)
		.addProperty("size", &lm_module_t::size)
	.endClass()

	.beginNamespace("memhlp")
		.addFunction("getModule", &MemHlp::getModule)
		.addFunction("getJmpTarget", &MemHlp::getJmpTarget)
		.addFunction("hexdump", &LuaMemHlp::hexdump)
		.addFunction("findPrologue", &MemHlp::findPrologue)
		.addFunction("patternScan", &MemHlp::patternScan)

		.addFunction("getUserDataPtr", &LuaMemHlp::getUserDataPtr)
	.endNamespace()

	.beginClass<VFTableInfo_t>("VFTableInfo_t")
		.addConstructor<void(*)(const char*, const char*, unsigned int, unsigned int)>()
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

		.addFunction("getDouble", &LuaConfig::getDouble)
		.addFunction("getInt", &LuaConfig::getInt)
		.addFunction("getString", &LuaConfig::getString)
		.addFunction("getIntList", &LuaConfig::getIntList)
		.addFunction("getDoubleList", &LuaConfig::getDoubleList)
		.addFunction("getStringList", &LuaConfig::getStringList)
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
		.addFunction("postCallback", &LuaSDK::postCallback)
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

		.addFunction("alloc", LuaSDK::alloc)
		.addFunction("realloc", LuaSDK::realloc)
		.addFunction("free", LuaSDK::free)
	.endNamespace();

	auto dir = std::filesystem::path(CConfig::getDir());
	dir.append("plugins");

	if (!std::filesystem::exists(dir))
	{
		if (!std::filesystem::create_directory(dir))
		{
			LOG_NOTIFYERROR("Failed to create plugins directory!\nPlugins will be unavailable\n");
			return;
		}
	}

	for (const auto& lua : std::filesystem::directory_iterator { dir })
	{
		runLua(lua);
	}

	Lua::fireCallback(Lua::Callbacks::SLSsteam_ConfigLoaded);

	if (Hooks::IClientUtils_GetOfflineMode.hooked) //Ghetto way to check wheter our hooks are setup
	{
		Lua::fireCallback(Lua::Callbacks::SLSsteam_Initialized);
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

	LOG_DEBUG("Ran %s\n", path.filename().c_str());
	return true;
}

void Lua::registerCallback(const std::string& name, luabridge::LuaRef fn)
{
	callbacks[name].emplace_back(fn);
	LOG_DEBUG("Registered lua callback for %s\n", name.c_str());
}
