#pragma once

#include "sdk/sdk.hpp"

#include <cstdint>
#include <fstream>
#include <mutex>
#include <vector>


class CFileWatcher;

namespace SLSAPI
{
	struct InstallOp_t
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

	struct SetCompatOp_t
	{
		AppId_t appId;
		std::string tool;
	};

	extern const char* path;
	extern std::fstream fstream;
	extern CFileWatcher* watcher;

	extern std::mutex cmdMutex;

	extern std::vector<InstallOp_t> installOps;
	extern std::vector<SetCompatOp_t> compatOps;

	bool isEnabled();
	void onFileChange();
	void init();

	void runCompatOps();
	void runInstallOps();
	void runIPCFrame();
}
