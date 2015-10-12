#include <CommonLib/StdAfx.h>
#include <CommonLib/System.h>
#include <CommonLib/Debug.h>
namespace fastbird
{
	//------------------------------------------------------------------------
	int GetNumProcessors()
	{
#if defined(_MSC_VER)
		SYSTEM_INFO SI;
		GetSystemInfo(&SI);
		return SI.dwNumberOfProcessors;
#else
		return 1;
#endif
	}

	//------------------------------------------------------------------------
	HMODULE GetCurrentModule()
	{ // NB: XP+ solution!
		HMODULE hModule = NULL;
		GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCTSTR)GetCurrentModule,
			&hModule);

		return hModule;
	}

	//------------------------------------------------------------------------
	void LogLastError(const char* file, int line, const char* function)
	{
		char buf[2048];

		DWORD err = GetLastError();
		if (err == 0)
			return;

		LPVOID lpMsgBuf;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);

		sprintf_s(buf, 2048, "%s(%d): %s() - %s \n", file, line, function, (char*)lpMsgBuf);
		OutputDebugStringA(buf);
		std::cerr << buf;

		LocalFree(lpMsgBuf);
	}

	//------------------------------------------------------------------------
	const char* GetLastErrorString(const char* file, int line, const char* function)
	{
		static char buf[2048];

		DWORD err = GetLastError();
		if (err == 0)
			return "";

		LPVOID lpMsgBuf;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);

		sprintf_s(buf, 2048, "%s(%d): %s() - %s \n", file, line, function, (char*)lpMsgBuf);
		LocalFree(lpMsgBuf);
		return buf;
	}

	//------------------------------------------------------------------------
	const char* FBGetComputerName()
	{
		static char buf[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
		DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
		int success = GetComputerName(buf, &size);
		if (!success)
		{
			FB_LOG_LAST_ERROR();
		}
		return buf;
	}
}