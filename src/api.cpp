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

	std::vector<InstallCommand_t> installs;
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
		if (!Utils::isNumber(split[1].c_str()))
		{
			LOG_ERROR("Failed to run install API command, %s is not a number!\n", split[1].c_str());
			return;
		}

		if (!Utils::isNumber(split[2].c_str()))
		{
			LOG_ERROR("Failed to run install API command, %s is not a number!\n", split[2].c_str());
			return;
		}

		const AppId_t appId = std::strtoul(split[1].c_str(), nullptr, 10);
		const uint32_t library = std::strtoul(split[2].c_str(), nullptr, 10);

		const std::lock_guard guard(cmdMutex);
		installs.emplace_back
		(
			InstallCommand_t
			{
				InstallCommand_t::InstallType::Install,
				appId,
				library
			}
		);
	}
	else if (strcmp(split[0].c_str(), "uninstall") == 0 && split.size() > 1)
	{
		if (!Utils::isNumber(split[1].c_str()))
		{
			LOG_ERROR("Failed to run install API command, %s is not a number!\n", split[1].c_str());
			return;
		}

		const AppId_t appId = std::strtoul(split[1].c_str(), nullptr, 10);

		const std::lock_guard guard(cmdMutex);
		installs.emplace_back
		(
			InstallCommand_t
			{
				InstallCommand_t::InstallType::Uninstall,
				appId,
				0 //No library index for uninstall needed
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

void SLSAPI::runInstallCommands()
{
	const auto usr = g_pSteamEngine->getUser();
	if (!usr)
	{
		return;
	}

	const auto appManager = usr->getAppManager();

	while(installs.size())
	{
		const auto app = installs.begin();

		switch(app->type)
		{
			case InstallCommand_t::InstallType::Install:
				appManager->installApp(app->appId, app->libraryIndex);
				LOG_DEBUG("Installed %u to %u\n", app->appId, app->libraryIndex);
				break;

			case InstallCommand_t::InstallType::Uninstall:
				appManager->uninstallApp(app->appId);
				LOG_DEBUG("Uninstalled %u\n", app->appId);
				break;

		}

		installs.erase(app);
	}
}

void SLSAPI::runIPCFrame()
{
	const std::lock_guard guard(cmdMutex);

	runInstallCommands();
}
