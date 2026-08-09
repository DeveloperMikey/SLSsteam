#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <openssl/sha.h>
#include <shared_mutex>
#include <sstream>
#include <unordered_set>

#define LOG_TRACE(fmt, ...) g_pLog->trace(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ONCE(fmt, ...) g_pLog->once(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) g_pLog->debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_ONCE(fmt, ...) g_pLog->debugOnce(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) g_pLog->warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) g_pLog->error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) g_pLog->info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFY(fmt, ...) g_pLog->notify(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYLONG(fmt, ...) g_pLog->notifyLong(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYWARN(fmt, ...) g_pLog->notifyWarn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYERROR(fmt, ...) g_pLog->notifyError(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_CUSTOM(lvl, fmt, ...) g_pLog->custom(lvl, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


enum ELogLevel : unsigned int
{
	k_ELogLevelTrace = 1 << 1, //Tracing for debug
	k_ELogLevelOnce = 1 << 2, //Only log once
	k_ELogLevelDebug = 1 << 3, //Debugging statements
	k_ELogLevelWarn = 1 << 4, //Something went wrong but it's not terrible
	k_ELogLevelError = 1 << 5, //Something went wrong and it's terrible/can't be recovered from. Function failed
	k_ELogLevelInfo = 1 << 6, //Log for users/external tools
	k_ELogLevelNotifyShort = 1 << 7,
	k_ELogLevelNotifyLong = 1 << 8,
	k_ELogLevelNotifyWarn = 1 << 9,
	k_ELogLevelNotifyError = 1 << 10,
	k_ELogLevelNone = 1 << 11
};

std::string ELogLevel_ToString(unsigned int lvlFlags);

class CLog
{
	std::ofstream ofstream;
	std::unordered_set<std::string> msgHist {};
	std::shared_mutex mutex;

public:
	template<typename ...Args>
	__attribute__((hot))
	void __log(const unsigned int flags, const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		if (flags < getMinLevel())
		{
			return;
		}

		const size_t size = snprintf(nullptr, 0, msg, args...) + 1; //Allocate one more byte for zero termination
		std::string formatted;
		formatted.resize(size);
		snprintf(formatted.data(), size, msg, args...);

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

	std::string path;

	CLog(const char* path);
	~CLog();

	#ifdef TRACE
	template<typename ...Args>
	constexpr void trace(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelTrace, file, function, line, msg, args...);
	}
	#else
	template<typename ...Args>
	constexpr void trace
	(
		__attribute__((unused)) const char* file,
		__attribute__((unused)) const char* function,
		__attribute__((unused)) const int line,
		__attribute__((unused)) const char* msg,
		__attribute__((unused)) Args... args
	)
	{
	}
	#endif

	#ifdef DEBUG
	template<typename ...Args>
	constexpr void once(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelOnce, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void debug(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelDebug, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void debugOnce(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelDebug | k_ELogLevelOnce, file, function, line, msg, args...);
	}
	#else
	template<typename ...Args>
	constexpr void once
	(
		__attribute__((unused)) const char* file,
		__attribute__((unused)) const char* function,
		__attribute__((unused)) const int line,
		__attribute__((unused)) const char* msg,
		__attribute__((unused)) Args... args
	)
	{
	}
	template<typename ...Args>
	constexpr void debug
	(
		__attribute__((unused)) const char* file,
		__attribute__((unused)) const char* function,
		__attribute__((unused)) const int line,
		__attribute__((unused)) const char* msg,
		__attribute__((unused)) Args... args
	)
	{
	}
	template<typename ...Args>
	constexpr void debugOnce
	(
		__attribute__((unused)) const char* file,
		__attribute__((unused)) const char* function,
		__attribute__((unused)) const int line,
		__attribute__((unused)) const char* msg,
		__attribute__((unused)) Args... args
	)
	{
	}
	#endif

	template<typename ...Args>
	constexpr void warn(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelWarn, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void error(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelError, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void info(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelInfo, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void notify(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelNotifyShort, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void notifyLong(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelNotifyLong, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void notifyWarn(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelNotifyWarn, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void notifyError(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(k_ELogLevelNotifyError, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void custom(const unsigned int flags, const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(flags, file, function, line, msg, args...);
	}

	//Do not include config.hpp in this header, otherwise things will break :) (proly due to recursive inclusion)
	static ELogLevel getMinLevel();
	static bool shouldNotify();
	static CLog* createDefaultLog();
};

extern std::unique_ptr<CLog> g_pLog;
