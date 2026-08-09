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

#ifdef TRACE
#define LOG_TRACE(fmt, ...) g_pLog->trace(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_TRACE(fmt, ...) ((void)0)
#endif

#ifdef DEBUG
#define LOG_ONCE(fmt, ...) g_pLog->once(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) g_pLog->debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG_ONCE(fmt, ...) ((void)0)
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#define LOG_WARN(fmt, ...) g_pLog->warn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) g_pLog->error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) g_pLog->info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFY(fmt, ...) g_pLog->notify(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYLONG(fmt, ...) g_pLog->notifyLong(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYWARN(fmt, ...) g_pLog->notifyWarn(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_NOTIFYERROR(fmt, ...) g_pLog->notifyError(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

enum class LogLevel : unsigned int
{
	Trace, //Tracing for debug
	Once, //Only log once
	Debug, //Debugging statements
	Warn, //Something went wrong but it's not terrible
	Error, //Something went wrong and it's terrible/can't be recovered from. Function failed
	Info, //Log for users/external tools
	NotifyShort,
	NotifyLong,
	NotifyWarn,
	NotifyError,
	None
};

class CLog
{
	std::ofstream ofstream;
	std::unordered_set<std::string> msgHist {};
	std::shared_mutex mutex;

	constexpr const char* logLvlToStr(LogLevel& lvl)
	{
		switch(lvl)
		{
			case LogLevel::Trace: //Tracing for debug
				return "Trace";
			case LogLevel::Once: //Only log once
				return "Once";
			case LogLevel::Debug: //Debugging statements
				return "Debug";
			case LogLevel::Warn: //Something went wrong but it's not terrible
				return "Warn";
			case LogLevel::Error: //Something went wrong and it's terrible/can't be recovered from. Function failed
				return "Error";
			case LogLevel::Info: //Log for users/external tools
				return "Info";
			case LogLevel::NotifyShort:
				return "NotifyShort";
			case LogLevel::NotifyLong:
				return "NotifyLong";
			case LogLevel::NotifyWarn:
				return "NotifyWarn";
			case LogLevel::NotifyError:
				return "NotifyError";

			//Shut gcc warning up
			default:
				return "Unknown";
		}
	}

	template<typename ...Args>
	__attribute__((hot))
	void __log(LogLevel lvl, const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		if (lvl < getMinLevel())
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

		switch(lvl)
		{
			//TODO: Fix possible breakage when there's only one " in formatted
			case LogLevel::NotifyShort:
				notifySS << "notify-send -t 10000 -u \"normal\" \"SLSsteam\" \"" << formatted.c_str() << "\"";
				break;
			case LogLevel::NotifyLong:
				notifySS << "notify-send -t 30000 -u \"normal\" \"SLSsteam\" \"" << formatted.c_str() << "\"";
				break;
			case LogLevel::NotifyWarn:
				notifySS << "notify-send -u \"critical\" \"SLSsteam - Warning\" \"" << formatted.c_str() << "\"";
				break;
			case LogLevel::NotifyError:
				notifySS << "notify-send -u \"critical\" \"SLSsteam - Error\" \"" << formatted.c_str() << "\"";
				break;

			default:
				appendNewLine = false;
				break;
		}

		if (shouldNotify() && notifySS.str().size() > 0)
		{
			system(notifySS.str().c_str());
			__log(LogLevel::Debug, file, function, line, "system(\"%s\")\n", notifySS.str().c_str());
		}

		std::ostringstream prefixSS;

		if (file && function)
		{
			prefixSS << "[" << logLvlToStr(lvl) << " in " << file << ":" << function << ":" << line << "]";
		}
		else
		{
			prefixSS << "[" << logLvlToStr(lvl) << "]";
		}

		const auto prefix = prefixSS.str();

		const auto lock = std::lock_guard(mutex);

		//Prevent crashes from queued operations
		if (!ofstream.is_open())
		{
			return;
		}

		if (lvl == LogLevel::Once)
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

public:
	std::string path;

	CLog(const char* path);
	~CLog();

	template<typename ...Args>
	constexpr void trace(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Trace, file, function, line, msg, args...);
	}
	template<typename ...Args>
	constexpr void once(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Once, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void debug(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Debug, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void warn(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Warn, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void error(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Error, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void info(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::Info, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void notify(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::NotifyShort, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void notifyLong(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::NotifyLong, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void notifyWarn(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::NotifyWarn, file, function, line, msg, args...);
	}

	template<typename ...Args>
	constexpr void notifyError(const char* file, const char* function, const int line, const char* msg, Args... args)
	{
		__log(LogLevel::NotifyError, file, function, line, msg, args...);
	}

	//Do not include config.hpp in this header, otherwise things will break :) (proly due to recursive inclusion)
	static LogLevel getMinLevel();
	static bool shouldNotify();
	static CLog* createDefaultLog();
};

extern std::unique_ptr<CLog> g_pLog;
