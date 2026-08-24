#include "lua.hpp"

#include "config.hpp"
#include "log.hpp"

#include <lua.h>

extern "C"
{
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
}

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/UnorderedSet.h"


void onFileChange()
{
	Lua::runLua("/home/acesls/.config/SLSsteam/plugins/example.lua");
}

lua_State* Lua::state;
std::unique_ptr<CFileWatcher> Lua::watcher = std::make_unique<CFileWatcher>(onFileChange);

namespace Config
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

namespace Log
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
	state = luaL_newstate();
	luaL_openlibs(state);

	luabridge::getGlobalNamespace(state)

	.beginNamespace("log")
		.addFunction("debug", &Log::debug)
		.addFunction("info", &Log::info)
		.addFunction("notify", &Log::notify)
	.endNamespace()

	.beginClass<CConfig>("CConfig")
		.addConstructor<void(*)()>()
		.addFunction("getAdditionalApps", &Config::getAdditionalApps)
		.addFunction("setAdditionalApps", &CConfig::setAdditionalApps)
	.endClass()

	.beginNamespace("SLS")
		.addProperty("config", &Config::get)
	.endNamespace();

	auto dir = std::filesystem::path(CConfig::getDir());
	dir.append("plugins");

	for (const auto& lua : std::filesystem::directory_iterator{ dir })
	{
		const auto path = lua.path();
		watcher->addFile(path.c_str());
		runLua(path);
	}

	watcher->start();

	LOG_DEBUG("Lua initialized\n");
}

bool Lua::runLua(const std::string& path)
{
	if (luaL_dofile(state, path.c_str()) != LUA_OK)
	{
		LOG_ERROR("Failed to run example!\n%s\n", lua_tostring(state, -1));
		return false;
	}

	return true;
}

