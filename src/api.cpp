#include "api.hpp"

#include "config.hpp"
#include "filewatcher.hpp"
#include "utils.hpp"

#include <cerrno>
#include <mutex>


namespace SLSAPI
{
	const char* path = "/tmp/SLSsteam.API";

	std::fstream fstream;
	CFileWatcher* watcher;

	std::mutex cmdMutex;

	std::vector<SetCompatOp_t> compatOps;
	std::vector<InstallOp_t> installOps;
}

bool SLSAPI::isEnabled()
{
	return g_config.api.get() && fstream.is_open();
}

void SLSAPI::onFileChange()
{
	//Hot reload support :)
	if (!isEnabled())
	{
		return;
	}

	//Shitty way to reopen the stream. We have to do this, otherwise the fstream gets invalidated when running echo >
	fstream.close();
	fstream.open(path, std::fstream::in);

	char cmd[128];
	fstream.getline(cmd, sizeof(cmd));

	LOG_DEBUG("API Running %s\n", cmd);
	const auto split = Utils::strsplit(cmd, "|");

	if (strcmp(split[0].c_str(), "install") == 0 && split.size() > 2)
	{
		AppId_t appId;
		uint32_t library;

		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to install %s (not a number)!\n", split[1].c_str());
			return;
		}

		if (!Utils::tryConvertToNumber(split[2].c_str(), library))
		{
			LOG_ERROR("Failed to install %u to %s (not a number)!\n", appId, split[2].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		installOps.emplace_back
		(
			InstallOp_t
			{
				InstallOp_t::InstallType::Install,
				appId,
				library
			}
		);
	}

	else if (strcmp(split[0].c_str(), "uninstall") == 0 && split.size() > 1)
	{
		AppId_t appId;
		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to install %s (not a number)!\n", split[1].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		installOps.emplace_back
		(
			InstallOp_t
			{
				InstallOp_t::InstallType::Uninstall,
				appId,
				0 //No library index for uninstall needed
			}
		);
	}

	else if (strcmp(split[0].c_str(), "setcompat") == 0 && split.size() > 1)
	{
		AppId_t appId;
		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to set compat for %s (not a number)!\n", split[1].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		compatOps.emplace_back
		(
			SetCompatOp_t
			{
				appId,
				split.size() > 2 ? split[2].c_str() : "" //Empty string to clear compat tool
			}
		);
	}
}

void SLSAPI::init()
{
	fstream = std::fstream(path, std::fstream::in | std::fstream::out | std::fstream::trunc); //Open for reading, writing and also delete contents

	if (!fstream.is_open())
	{
		LOG_NOTIFYWARN("Failed to create %s (%s)!\n API will be unavailable", path, strerror(errno));
		return;
	}

	watcher = new CFileWatcher(onFileChange);
	const int fd = watcher->addFile(path);
	if (fd == -1)
	{
		LOG_NOTIFYWARN("Failed to watch %s!\n API will be unavailable", path);
		return;
	}

	watcher->start();
	LOG_DEBUG("SLSsteam API initialized!\n");
}

void SLSAPI::runCompatOps()
{
	if (!g_pClientCompat)
	{
		return;
	}

	while (compatOps.size())
	{
		const auto op = compatOps.begin();

		//Steam calls them with the same values
		g_pClientCompat->specifyCompatTool(op->appId, op->tool.c_str(), "", 250);
		LOG_DEBUG("Set compatibility tool for %u to %s\n", op->appId, op->tool.c_str());

		compatOps.erase(op);
	}
}

void SLSAPI::runInstallOps()
{
	const auto usr = g_pSteamEngine->getUser();
	if (!usr)
	{
		return;
	}

	const auto appManager = usr->getAppManager();

	while (installOps.size())
	{
		const auto op = installOps.begin();

		switch(op->type)
		{
			case InstallOp_t::InstallType::Install:
				appManager->installApp(op->appId, op->libraryIndex);
				LOG_DEBUG("Installed %u to %u\n", op->appId, op->libraryIndex);
				break;

			case InstallOp_t::InstallType::Uninstall:
				appManager->uninstallApp(op->appId);
				LOG_DEBUG("Uninstalled %u\n", op->appId);
				break;

		}

		installOps.erase(op);
	}
}

void SLSAPI::runIPCFrame()
{
	const std::lock_guard guard(cmdMutex);

	runCompatOps();
	runInstallOps();
}
