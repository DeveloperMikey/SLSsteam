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
		enum class InstallType
		{
			Install,
			Uninstall
		};

		InstallType type;
		AppId_t appId;
		uint32_t libraryIndex;
	};

	extern const char* path;
	extern std::fstream fstream;
	extern CFileWatcher* watcher;

	extern std::mutex cmdMutex;

	extern std::vector<InstallCommand_t> installs;

	bool isEnabled();
	void onFileChange();
	void init();

	void runInstallCommands();
	void runIPCFrame();
}
