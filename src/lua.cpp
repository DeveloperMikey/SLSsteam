#include "lua.hpp"

#include "config.hpp"
#include "hooks.hpp"
#include "log.hpp"
#include "memhlp.hpp"

#include <filesystem>
#include <lua.h>
#include <unordered_map>
#include <vector>

extern "C"
{
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
}

#include "LuaBridge/Array.h"
#include "LuaBridge/List.h"
#include "LuaBridge/UnorderedSet.h"
#include "LuaBridge/LuaBridge.h"


void onFileChange(__attribute__((unused)) const std::filesystem::path& path)
{
	Lua::init();
}

lua_State* Lua::state;
std::unique_ptr<CFileWatcher> Lua::watcher = std::make_unique<CFileWatcher>(onFileChange);

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

void Lua::init()
{
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
		.addFunction("findPrologue", &MemHlp::findPrologue)
		.addFunction("patternScan", &MemHlp::patternScan)
	.endNamespace()

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

	.beginNamespace("SLS")
		.addProperty("config", &LuaConfig::get)
	.endNamespace();

	auto dir = std::filesystem::path(CConfig::getDir());
	dir.append("plugins");

	for (const auto& lua : std::filesystem::directory_iterator { dir })
	{
		runLua(lua);
	}

	if (watcher->fileFdMap.size() < 1)
	{
		watcher->addFile(dir.c_str());
		watcher->start();
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

