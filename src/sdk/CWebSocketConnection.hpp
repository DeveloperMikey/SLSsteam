#pragma once

#include <cstdint>

enum EWebSocketConnectionSendType : uint32_t
{
	k_EWebSocketConnectionSendRaw = 2
};


class CWebSocketConnection
{
public:
	bool buildAndAsyncSendFrame(const uint32_t type, void* data, const uint32_t dataSize);
	bool buildAndAsyncSendFrameHk(const uint32_t type, void* data, const uint32_t dataSize);
};

extern CWebSocketConnection* g_pWebSocketConnection;
