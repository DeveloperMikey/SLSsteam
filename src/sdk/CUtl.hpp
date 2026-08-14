#pragma once

#include "steam.hpp"

#include <cstdint>
#include <cstring>


template<typename T>
class CUtlMemory
{
public:

	T* base;
	uint32_t alloc;
	uint32_t growSize;

	CUtlMemory() : CUtlMemory(0x16) { }

	CUtlMemory(unsigned int size)
	{
		alloc = size;
		growSize = 0;

		base = reinterpret_cast<T*>(Steam::Plat_Alloc(this->alloc * sizeof(T)));
	}

	~CUtlMemory()
	{
		if (base)
		{
			Steam::Plat_Free(base);
		}
	}
};

template<typename T>
class CUtlVector
{
public:

	CUtlMemory<T> memory;
	uint32_t size;

	constexpr T* at(uint32_t index)
	{
		if (index >= size)
		{
			return nullptr;
		}

		return &memory.base[index];
	};

	constexpr bool swap(uint32_t index, uint32_t index2)
	{
		if (index > size || index2 > size)
		{
			return false;
		}

		T buf = *at(index2);
		*at(index2) = *at(index);
		*at(index) = buf;

		return true;
	}
};

template<typename T>
class CUtlBuffer
{
	typedef int(*CUtlBuffer_Function1_t)(void*);
	typedef bool(*CUtlBuffer_Resize_t)(void*, int32_t);

public:
	CUtlBuffer() : CUtlBuffer(0x16) { }

	CUtlBuffer(unsigned int size)
	{
		//Never ever not zero the buffer, unless you want steam to implode
		memset(reinterpret_cast<void*>(this), 0, sizeof(CUtlBuffer));
		mem = CUtlMemory<T>(size);
	}

	CUtlMemory<T> mem;					//0x0
	int32_t get;						//0xC
	int32_t put;						//0x10
	int32_t offset;						//0x14
	uint32_t flags;						//0x18
	CUtlBuffer_Function1_t fn1C;		//0x1C
	int32_t field20;					//0x20
	CUtlBuffer_Resize_t resize;			//0x24
	int32_t field28;					//0x28
}; //0x2C
