#include <CommonLib/StdAfx.h>
#include <CommonLib/Time.h>
#include <ctime>
#include <iomanip>

namespace fastbird
{
	std::string GetTimeString()
	{
		auto t = std::time(nullptr);
		return std::asctime(std::localtime(&t));
	}

	std::string GetTimeStringForFileName()
	{
		auto t = std::time(nullptr);
		auto tm = std::localtime(&t);
		char buf[256];
		std::strftime(buf, 256, "%Y%m%d_%H%M%S", tm);
		return std::string(buf);
	}
}