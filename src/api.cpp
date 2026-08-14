#include "api.hpp"

#include "config.hpp"
#include "filewatcher.hpp"
#include "utils.hpp"

#include <cerrno>
#include <mutex>
#include <sstream>


namespace SLSAPI
{
	const char* path = "/tmp/SLSsteam.API";

	std::fstream fstream;
	CFileWatcher* watcher;

	std::mutex cmdMutex;

	std::vector<CompatOp_t> compatOps;
	std::vector<LibraryOp_t> libraryOps;
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

	//Compatibility Manager
	if (strcmp(split[0].c_str(), "dumpcompat") == 0 && split.size() > 1)
	{
		AppId_t appId;
		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to dump compat for %s (not a number)!\n", split[1].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		compatOps.emplace_back(CompatOp_t { CompatOp_t::OpType::Dump, appId, "" } );
	}

	else if (strcmp(split[0].c_str(), "getcompat") == 0 && split.size() > 1)
	{
		AppId_t appId;
		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to get compat for %s (not a number)!\n", split[1].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		compatOps.emplace_back(CompatOp_t { CompatOp_t::OpType::Get, appId, "" } );
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
		compatOps.emplace_back(CompatOp_t { CompatOp_t::OpType::Set, appId, split.size() > 2 ? split[2].c_str() : "" } );
	}

	else if (strcmp(split[0].c_str(), "dumplibraries") == 0)
	{
		const std::lock_guard guard(cmdMutex);
		libraryOps.emplace_back(LibraryOp_t { LibraryOp_t::OpType::Dump, 0, 0 } );
	}

	//Application Manager
	else if (strcmp(split[0].c_str(), "install") == 0 && split.size() > 2)
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
		libraryOps.emplace_back(LibraryOp_t { LibraryOp_t::OpType::Install, appId, library } );
	}

	else if (strcmp(split[0].c_str(), "uninstall") == 0 && split.size() > 1)
	{
		AppId_t appId;
		if (!Utils::tryConvertToNumber(split[1].c_str(), appId))
		{
			LOG_ERROR("Failed to uninstall %s (not a number)!\n", split[1].c_str());
			return;
		}

		const std::lock_guard guard(cmdMutex);
		libraryOps.emplace_back(LibraryOp_t { LibraryOp_t::OpType::Uninstall, appId, 0 } );
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

		switch(op->type)
		{
			case CompatOp_t::OpType::Dump:
			{
				CUtlVector<const char*> tools { }; //Do not allocate anything, the function does for us.
				g_pClientCompat->getCompatToolsForApp(op->appId, &tools);

				std::ostringstream toolsSS;
				for (size_t i = 0; i < tools.size; i++)
				{
					const char* name = *tools.at(i);
					const char* displayName = g_pClientCompat->getDisplayName(name);

					if (toolsSS.str().size() > 0)
					{
						toolsSS << ", ";
					}

					toolsSS << "\"" << displayName << "\" (" << name << ")";
				}

				const auto str = toolsSS.str();
				LOG_API("Dump compatibility tools for %u: %s\n", op->appId, str.c_str());

				break;
			}

			case CompatOp_t::OpType::Get:
			{
				if (!g_pClientCompat->isCompatToolEnabled(op->appId))
				{
					LOG_API("Get compatibility tool for %u: Compatibility tool is disabled!\n", op->appId);
					break;
				}

				const char* name = g_pClientCompat->getCompatToolName(op->appId);
				const char* displayName = g_pClientCompat->getDisplayName(name);

				LOG_API("Get compatibility tool for %u: \"%s\" (%s)\n", op->appId, displayName, name);

				break;
			}

			case CompatOp_t::OpType::Set:
			{
				//Steam calls them with the same values
				g_pClientCompat->specifyCompatTool(op->appId, op->tool.c_str(), "", 250);
				LOG_DEBUG("Set compatibility tool for %u to %s\n", op->appId, op->tool.c_str());

				break;
			}
		}


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

	while (libraryOps.size())
	{
		const auto op = libraryOps.begin();

		switch(op->type)
		{
			case LibraryOp_t::OpType::Dump:
			{
				const size_t num = appManager->getNumLibraryFolders();
				char pathBuf[0x1000]; //Same size steam passes
				char labelBuf[0x80]; //Same size steam passes
				
				std::ostringstream outSS;
				for (size_t i = 0; i < num; i++)
				{
					size_t pathLen = appManager->getLibraryFolderPath(i, pathBuf, sizeof(pathBuf));
					size_t labelLen = appManager->getLibraryFolderLabel(i, labelBuf, sizeof(labelBuf));

					if (labelLen)
					{
						LOG_API("Library \"%s\" at \"%s\" has index %u\n", std::string(labelBuf, labelLen).c_str(), std::string(pathBuf, pathLen).c_str(), i);
					}
					else
					{
						LOG_API("Library at \"%s\" has index %u\n", std::string(pathBuf, pathLen).c_str(), i);
					}
				}

				break;
			}

			case LibraryOp_t::OpType::Install:
			{
				appManager->installApp(op->appId, op->libraryIndex);
				LOG_DEBUG("Installed %u to %u\n", op->appId, op->libraryIndex);

				break;
			}

			case LibraryOp_t::OpType::Uninstall:
			{
				appManager->uninstallApp(op->appId);
				LOG_DEBUG("Uninstalled %u\n", op->appId);

				break;
			}
		}

		libraryOps.erase(op);
	}
}

void SLSAPI::runIPCFrame()
{
	const std::lock_guard guard(cmdMutex);

	runCompatOps();
	runInstallOps();
}
