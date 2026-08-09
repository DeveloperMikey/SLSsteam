#include "log.hpp"

#include "config.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <sstream>


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

	for(int i = numLogLevels; i >= 0; i--)
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

void CLog::__log(const unsigned int flags, const char* file, const char* function, const int line, const char* msg, const va_list& vArgs)
{
	if (flags < getMinLevel())
	{
		return;
	}

	const size_t size = vsnprintf(nullptr, 0, msg, vArgs) + 1; //Allocate one more byte for zero termination
	std::string formatted;
	formatted.resize(size);
	vsnprintf(formatted.data(), size, msg, vArgs);

	std::ostringstream notifySS;
	//Notifications do not end with a newline, so we append one
	//Default statement sets false for normal logging
	bool appendNewLine = true;

	switch(flags)
	{
		//TODO: Fix possible breakage when there's only one " in formatted
		case k_ELogLevelNotifyShort:
			notifySS << "notify-send -t 10000 -u \"normal\" \"SLSsteam\" \"" << formatted.c_str() << "\"";
			break;
		case k_ELogLevelNotifyLong:
			notifySS << "notify-send -t 30000 -u \"normal\" \"SLSsteam\" \"" << formatted.c_str() << "\"";
			break;
		case k_ELogLevelNotifyWarn:
			notifySS << "notify-send -u \"critical\" \"SLSsteam - Warning\" \"" << formatted.c_str() << "\"";
			break;
		case k_ELogLevelNotifyError:
			notifySS << "notify-send -u \"critical\" \"SLSsteam - Error\" \"" << formatted.c_str() << "\"";
			break;

		default:
			appendNewLine = false;
			break;
	}

	if (shouldNotify() && notifySS.str().size() > 0)
	{
		system(notifySS.str().c_str());
		debug(file, function, line, "system(\"%s\")\n", notifySS.str().c_str());
	}

	std::ostringstream prefixSS;

	if (file && function)
	{
		prefixSS << "[" << ELogLevel_ToString(flags) << " in " << file << ":" << function << ":" << line << "]";
	}
	else
	{
		prefixSS << "[" << ELogLevel_ToString(flags) << "]";
	}

	const auto prefix = prefixSS.str();

	const auto lock = std::lock_guard(mutex);

	//Prevent crashes from queued operations
	if (!ofstream.is_open())
	{
		return;
	}

	if (flags & k_ELogLevelOnce)
	{
		for(const auto& oldMsg : msgHist)
		{
			if (oldMsg == formatted)
			{
				return;
			}
		}

		msgHist.emplace(formatted);
	}

	//ofstream << prefix << std::setfill(' ') << std::setw(80 - prefix.size()) << " " << formatted.c_str();
	//Padding makes things nicer, but basically unreadable
	ofstream << prefix << " " << formatted.c_str();

	if (appendNewLine)
	{
		ofstream << "\n";
	}

	ofstream.flush();
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


#ifdef TRACE
void CLog::trace(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelTrace, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::traceOnce(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelTrace | k_ELogLevelOnce, file, function, line, msg, vArgs);
	va_end(vArgs);
}
#else
void CLog::trace
(
	__attribute__((unused)) const char* file,
	__attribute__((unused)) const char* function,
	__attribute__((unused)) const int line,
	__attribute__((unused)) const char* msg,
	...
)
{
}
void CLog::traceOnce
(
	__attribute__((unused)) const char* file,
	__attribute__((unused)) const char* function,
	__attribute__((unused)) const int line,
	__attribute__((unused)) const char* msg,
	...
)
{
}
#endif

#ifdef DEBUG
void CLog::once(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelOnce, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::debug(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelDebug, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::debugOnce(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelDebug | k_ELogLevelOnce, file, function, line, msg, vArgs);
	va_end(vArgs);
}
#else
void CLog::once
(
	__attribute__((unused)) const char* file,
	__attribute__((unused)) const char* function,
	__attribute__((unused)) const int line,
	__attribute__((unused)) const char* msg,
	...
)
{
}
void CLog::debug
(
	__attribute__((unused)) const char* file,
	__attribute__((unused)) const char* function,
	__attribute__((unused)) const int line,
	__attribute__((unused)) const char* msg,
	...
)
{
}
void CLog::debugOnce
(
	__attribute__((unused)) const char* file,
	__attribute__((unused)) const char* function,
	__attribute__((unused)) const int line,
	__attribute__((unused)) const char* msg,
	...
)
{
}
#endif

void CLog::warn(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelWarn, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::error(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelError, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::info(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelInfo, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::notify(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelNotifyShort, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::notifyLong(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelNotifyLong, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::notifyWarn(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelNotifyWarn, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::notifyError(const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(k_ELogLevelNotifyError, file, function, line, msg, vArgs);
	va_end(vArgs);
}
void CLog::custom(const unsigned int flags, const char* file, const char* function, const int line, const char* msg, ...)
{
	va_list vArgs;
	va_start(vArgs, msg);
	__log(flags, file, function, line, msg, vArgs);
	va_end(vArgs);
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
