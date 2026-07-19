#pragma once

#include "steam.hpp"

#include "protobufs/steammessages_base.pb.h"

#include "../log.hpp"

#include <cstdint>
#include <cstring>


//Helper class to make calculations more legible
class CNetPacketBody
{
public:

	EMsg type;
	uint32_t headerSize;
	//Header[headerSize]
	//Body[CNetPacket->size - headerSize - sizeof(CNetPacketBody)]
};

constexpr static unsigned int MAX_PACKET_SIZE = 1024 * 1024 * 1; //1MB
constexpr static unsigned int MAX_PACKETS = 8;

//TODO: Move into anonymous namespace or something, so these don't clutter the global namespace
extern uint8_t g_packetsArray[MAX_PACKET_SIZE * MAX_PACKETS];
extern uint32_t g_packetsArrayIndex;

extern std::mutex g_packetSerializeMutex;

class CNetPacket
{
public:
	constexpr static unsigned int INVALID_MESSAGE_TYPE = 0xFFFFFFFF;
	constexpr static unsigned int PROTOBUF_TYPE_MASK = 0x80000000;

	uint8_t __pad0x0[0x4];			//0x0
	CNetPacketBody* body;			//0x4
	uint32_t size;					//0x8
	uint8_t __pad_0xC[0x4];			//0xC
	CNetPacketBody* originalBody;	//0x10
	
	constexpr bool isValid() const
	{
		return body && size >= 8 && body->type != INVALID_MESSAGE_TYPE;
	}
	
	constexpr EMsg getType() const
	{
		if (!body)
		{
			return 0;
		}

		return body->type;
	}

	const char* getProtoBufTypeName();

	constexpr bool isProtoBuf() const
	{
		return getType() & PROTOBUF_TYPE_MASK;
	}

	constexpr EMsg getProtoBufType() const
	{
		return getType() & ~PROTOBUF_TYPE_MASK;
	}

	void serializeHeader(const CMsgProtoBufHeader& header);
	CMsgProtoBufHeader deserializeHeader() const;

	template<typename T>
	constexpr void serialize(const T& msg, const CMsgProtoBufHeader* header)
	{
		constexpr uintptr_t headerOffset = sizeof(CNetPacketBody);
		const uintptr_t headerSize = header ? header->ByteSizeLong() : body->headerSize;

		const uintptr_t msgOffset = headerSize + headerOffset;
		const uintptr_t newSize = msg.ByteSizeLong() + msgOffset;

		if (newSize >= MAX_PACKET_SIZE)
		{
			g_pLog->debug("Failed to serialize %p! Buffer to small (needed %u, has %u)\n", getType(), newSize, MAX_PACKET_SIZE);
			return;
		}

		const std::lock_guard lock(g_packetSerializeMutex);
		uint8_t* mem = &g_packetsArray[g_packetsArrayIndex * MAX_PACKET_SIZE];

		if (header)
		{
			if (!header->SerializeToArray(mem + headerOffset, headerSize))
			{
				g_pLog->debug("Failed to serialize header!\n");
				return;
			}

			*reinterpret_cast<uint32_t*>(mem) = body->type;
			*reinterpret_cast<uint32_t*>(mem + sizeof(body->type)) = headerSize;
		}
		else
		{
			memcpy(mem, body, msgOffset);
		}

		if (!msg.SerializeToArray(mem + msgOffset, msg.ByteSizeLong()))
		{
			g_pLog->debug("Failed to serialize %p!\n", getType());
			return;
		}

		body = reinterpret_cast<CNetPacketBody*>(mem);
		size = newSize;
		//If I understand correctly Steam cleans up for us, that's why we crash when we free the oldBody ourself
		//However the body we allocate doesn't get freed, so we just reuse a buffer for it

		g_pLog->debug("Serialized %p into PACKETS_ARRAY at %u with size %u\n", getType(), g_packetsArrayIndex, newSize);

		g_packetsArrayIndex++;
		if (g_packetsArrayIndex >= MAX_PACKETS)
		{
			g_packetsArrayIndex = 0;
		}
	}

	template<typename T>
	constexpr void serialize(const T& msg)
	{
		serialize(msg, nullptr);
	}
	
	template<typename T>
	constexpr T deserializeBody() const
	{
		const uintptr_t msgOffset = body->headerSize + sizeof(CNetPacketBody);
		auto msg = T();

		msg.ParseFromArray(reinterpret_cast<uint8_t*>(body) + msgOffset, size - msgOffset);

		return msg;
	}
}; //0x14
