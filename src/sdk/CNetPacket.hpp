#pragma once

#include "CProtoBufMsgBase.hpp"
#include "steam.hpp"

#include "../log.hpp"

#include <cstdint>
#include <string>


//Helper class to make calculations more legible
class CNetPacketBody
{
public:

	ENetPacket type;
	uint32_t headerSize;
	//Header[headerSize]
	//Body[CNetPacket->size - headerSize - sizeof(CNetPacketBody)]
};

class CNetPacket
{
public:
	uint8_t __pad0x0[0x4];			//0x0
	CNetPacketBody* body;			//0x4
	uint32_t size;					//0x8
	int32_t refs;					//0xC
	CNetPacketBody* originalBody;	//0x10
	
	constexpr bool isValid() const
	{
		return body && size > sizeof(CNetPacketBody) && body->type != INVALID_NETPACKET_TYPE;
	}
	
	constexpr ENetPacket getType() const
	{
		if (!body)
		{
			return INVALID_NETPACKET_TYPE;
		}

		return body->type;
	}

	std::string getProtoBufTypeName() const;

	constexpr bool isProtoBuf() const
	{
		if (getType() == INVALID_NETPACKET_TYPE)
		{
			return INVALID_NETPACKET_TYPE;
		}

		return getType() & PROTOBUF_TYPE_MASK;
	}

	constexpr EMsg getProtoBufType() const
	{
		return static_cast<EMsg>(getType() & ~PROTOBUF_TYPE_MASK);
	}

	CMsgProtoBufHeader deserializeHeader() const;

	template<typename T>
	void serialize(const T& msg, const CMsgProtoBufHeader* header)
	{
		constexpr uintptr_t headerOffset = sizeof(CNetPacketBody);
		const uintptr_t headerSize = header ? header->ByteSizeLong() : body->headerSize;

		const uintptr_t msgOffset = headerSize + headerOffset;
		const uintptr_t newSize = msg.ByteSizeLong() + msgOffset;

		uint8_t* mem = reinterpret_cast<uint8_t*>(Steam::Plat_Alloc(newSize));

		if (!mem)
		{
			g_pLog->debug("Failed to allocate new packet body with size %u!\n", newSize);
			return;
		}

		if (header)
		{
			if (!header->SerializeToArray(mem + headerOffset, headerSize))
			{
				g_pLog->debug("Failed to serialize header!\n");
				goto failed;
			}

			auto newBdyHdr = reinterpret_cast<CNetPacketBody*>(mem);
			newBdyHdr->type = body->type;
			newBdyHdr->headerSize = headerSize;
		}
		else
		{
			memcpy(mem, body, msgOffset);
		}

		if (!msg.SerializeToArray(mem + msgOffset, msg.ByteSizeLong()))
		{
			g_pLog->debug("Failed to serialize %p!\n", getType());
			goto failed;
		}

		Steam::Plat_Free(body);

		body = reinterpret_cast<CNetPacketBody*>(mem);
		size = newSize;
		originalBody = body;

		return;

	failed:
		Steam::Plat_Free(mem);
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

	void free();
}; //0x14
