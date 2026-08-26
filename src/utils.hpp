#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace Utils
{
	double calculateEntropy(const std::vector<uint8_t>& bytes);
	int exec(const std::vector<std::string>& exe, const std::vector<std::string>& args, std::string* stdOut);
	bool isNumber(const char* str);
	std::string getFileSHA256(const char* filePath);
	std::vector<std::string> strsplit(char* str, const char* delimeter);

	template<typename T>
	bool tryConvertToNumber(const char* str, T& out)
	{
		if constexpr (std::is_same_v<T, int32_t>)
		{
			if (!isNumber(str))
			{
				return false;
			}

			out = std::stoi(str);
			return true;
		}

		else if constexpr (std::is_same_v<T, uint32_t>)
		{
			if (!isNumber(str))
			{
				return false;
			}

			out = std::stoul(str);
			return true;
		}

		//TODO: Add GCC error when compiling this path
		return false;
	}
}
