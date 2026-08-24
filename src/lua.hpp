#pragma once

#include "filewatcher.hpp"
#include "log.hpp"

#include <filesystem>
#include <memory>
#include <unordered_map>

extern "C"
{
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
}

#include "LuaBridge/LuaBridge.h"


namespace Lua
{
	namespace Callbacks
	{
		constexpr const char* SLSsteam_Initialized = "SLSsteam::initialized";
	}

	extern lua_State* state;
	extern std::unique_ptr<CFileWatcher> watcher;
	extern std::unordered_map<std::string, std::vector<luabridge::LuaRef>> callbacks;

	void init();
	bool runLua(const std::filesystem::path& path);

	template<typename ...Args>
	unsigned int fireCallback(const char* name, Args... args)
	{
		if (!callbacks.contains(name))
		{
			return 0;
		}

		unsigned int calls = 0;
		const auto& functions = callbacks.at(name);
		for (const auto& fn : functions)
		{
			try
			{
				fn(args...);
				calls++;
			}
			catch (luabridge::LuaException& exc)
			{
				LOG_ERROR("Failed running lua callback %s\n%s\n", name, exc.what());
			}
		}

		return calls;
	}

	void registerCallback(const std::string& name, luabridge::LuaRef fn);
}
