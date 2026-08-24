#pragma once

#include "filewatcher.hpp"

#include <filesystem>
#include <memory>

extern "C"
{
#include <luajit-2.1/lua.h>
}


namespace Lua
{
	extern lua_State* state;
	extern std::unique_ptr<CFileWatcher> watcher;

	void init();
	bool runLua(const std::filesystem::path& path);
}
