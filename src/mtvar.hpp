#pragma once

#include <mutex>
#include <shared_mutex>


template<typename T> class MTVariable
{
private:
	T instance;
	std::shared_mutex mutex;

public:
	MTVariable() : instance(empty()) { }
	MTVariable(T instance) : instance(instance) { }
	MTVariable(const MTVariable<T>& cpy) : instance(cpy.instance) { }
	~MTVariable() { }

	//Returns default instance
	T empty()
	{
		return T();
	}

	T get()
	{
		const auto lock = std::shared_lock(mutex);
		//Return copy
		return T(instance);
	}

	void set(const T& value)
	{
		const auto lock = std::lock_guard(mutex);
		instance = value;
	}

	void operator=(const T& val)
	{
		set(val);
	}

	void operator=(const MTVariable<T>& other)
	{
		set(other.instance);
	}
};
