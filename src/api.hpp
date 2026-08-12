#pragma once

#include "sdk/sdk.hpp"

#include <cstdint>
#include <fstream>
#include <mutex>
#include <vector>


class CFileWatcher;

namespace SLSAPI
{
	struct InstallCommand_t
	{
		AppId_t appId;
		uint32_t libraryIndex;
	};

	extern const char* path;
	extern std::fstream fstream;
	extern CFileWatcher* watcher;

	extern std::mutex executionMutex;
	extern std::vector<InstallCommand_t> installs;
	extern std::vector<AppId_t> uninstalls;

	bool isEnabled();
	void onFileChange();
	void init();

	void runIPCFrame();
}
