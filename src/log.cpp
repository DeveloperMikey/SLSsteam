#include "log.hpp"

#include "config.hpp"

#include <cstdlib>
#include <memory>


std::string ELogLevel_ToString(const unsigned int lvlFlags)
{
	constexpr static auto flagToString = [](const unsigned int flag)
	{
		switch(flag)
		{
			case k_ELogLevelTrace:
				return "Trace";
			case k_ELogLevelOnce:
				return "Once";
			case k_ELogLevelDebug:
				return "Debug";
			case k_ELogLevelWarn:
				return "Warn";
			case k_ELogLevelError:
				return "Error";
			case k_ELogLevelInfo:
				return "Info";
			case k_ELogLevelNotifyShort:
				return "Notify";
			case k_ELogLevelNotifyLong:
				return "Notify Long";
			case k_ELogLevelNotifyWarn:
				return "Notify Warn";
			case k_ELogLevelNotifyError:
				return "Notify Error";

			default:
				return "Unknown";
		}
	};

	constexpr unsigned int numLogLevels = 11;
	std::ostringstream lvlStr;

	for(unsigned int i = 0; i < numLogLevels; i++)
	{
		const unsigned int flag = 1 << i;

		if (lvlFlags & flag)
		{
			if (lvlStr.str().size() > 0)
			{
				lvlStr << "|";
			}

			lvlStr << flagToString(flag);
		}
	}

	return lvlStr.str();
}

CLog::CLog(const char* path) : path(path)
{
	ofstream = std::ofstream(path);
	if (!ofstream.is_open())
	{
		//We don't want to boot without a logfile
		throw std::runtime_error("Unable to open logfile!");
	}
}

CLog::~CLog()
{
	if (ofstream.is_open())
	{
		ofstream.close();
	}
}

//Dirty workaround for not being able to access g_config from __log
ELogLevel CLog::getMinLevel()
{
	return static_cast<ELogLevel>(1 << g_config.logLevel.get());
}

bool CLog::shouldNotify()
{
	return g_config.notifications.get();
}

CLog* CLog::createDefaultLog()
{
	const char* home = getenv("HOME");
	if (home)
	{
		std::ostringstream ss;
		ss << home << "/.SLSsteam.log";

		return new CLog(ss.str().c_str());
	}

	return nullptr;
}

std::unique_ptr<CLog> g_pLog;
