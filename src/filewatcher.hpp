#pragma once

#include <filesystem>
#include <pthread.h>
#include <sys/inotify.h>
#include <unordered_map>


typedef void(*FileModifyEvent_t)(const std::filesystem::path&);

class CFileWatcher
{
	pthread_t watchThread;

public:
	constexpr static int WATCH_MASK = IN_CLOSE_WRITE | IN_MOVED_TO;

	int notifyFd;
	std::unordered_map<int, std::filesystem::path> fileFdMap;

	FileModifyEvent_t onModify;

	CFileWatcher(const FileModifyEvent_t onModify);
	~CFileWatcher();

	int addFile(const char* path);
	bool start();
	void stop();
};
