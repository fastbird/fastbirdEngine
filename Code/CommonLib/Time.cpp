#include <CommonLib/StdAfx.h>
#include <CommonLib/Time.h>
#include <ctime>
#include <iomanip>

namespace fastbird
{
	std::string GetTimeString()
	{
		auto t = std::time(nullptr);
		char buffer[1024];
		struct tm newtime;
		localtime_s(&newtime, &t);
		asctime_s(buffer, &newtime);
		return std::string(buffer);
	}

	std::string GetTimeStringForFileName()
	{
		auto t = std::time(nullptr);
		struct tm newtime;
		localtime_s(&newtime, &t);
		char buf[256];
		std::strftime(buf, 256, "%Y%m%d_%H%M%S", &newtime);
		return std::string(buf);
	}
}