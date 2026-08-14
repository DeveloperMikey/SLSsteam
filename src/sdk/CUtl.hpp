#pragma once

#include "steam.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

constexpr static size_t CUTL_DEFAULT_ALLOC = 0x16;

template<typename T>
class CUtlMemory
{
public:
	T* base;
	uint32_t alloc;
	uint32_t growSize;

	CUtlMemory()
	{
		base = nullptr;
		alloc = 0;
		growSize = 0;
	}

	~CUtlMemory()
	{
		if (base)
		{
			Steam::Plat_Free(base);
		}
	};

	bool resize(const size_t newSize)
	{
		void* mem;

		if (base)
		{
			mem = Steam::Plat_Realloc(base, newSize);
		}
		else
		{
			mem = Steam::Plat_Alloc(newSize);
		}

		if (!mem)
		{
			return false;
		}

		base = reinterpret_cast<T*>(mem);
		return true;
	}
};

class CUtlBuffer
{
	typedef int(*CUtlBuffer_Function1_t)(void*);
	typedef bool(*CUtlBuffer_Resize_t)(void*, int32_t);

public:
	CUtlMemory<uint8_t> mem;			//0x0
	int32_t get;						//0xC
	int32_t put;						//0x10
	int32_t offset;						//0x14
	uint32_t flags;						//0x18
	CUtlBuffer_Function1_t fn1C;		//0x1C
	int32_t field20;					//0x20
	CUtlBuffer_Resize_t resizeFn;		//0x24
	int32_t field28;					//0x28

	bool resize(const size_t newSize);
}; //0x2C

//Null terminated wrapper
class CUtlString
{
public:
	char* str;

	~CUtlString();

	constexpr const char* get()
	{
		return str;
	}

	bool resize(const size_t newSize);
	void setValue(const char* newStr);
};

template<typename T>
class CUtlVector
{
public:
	CUtlMemory<T> mem;
	uint32_t size;

	constexpr bool resize(size_t newSize)
	{
		if (!mem.resize(newSize))
		{
			return false;
		}

		size = newSize;
		return true;
	}

	constexpr T* at(uint32_t index)
	{
		if (index >= size)
		{
			return nullptr;
		}

		return &mem.base[index];
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
