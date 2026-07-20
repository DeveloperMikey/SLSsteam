#pragma once

#include "protobufs/enums_clientserver.pb.h"

#include <cstdint>


class CProtoBufMsgBase;

class CAPIJob
{
public:
	uint32_t sendAndRecv(CProtoBufMsgBase* send, uint32_t timeOut, CProtoBufMsgBase* recv, EMsg targetType);
};
