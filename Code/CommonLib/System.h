#pragma once

inline int GetNumProcessors()
{
#if defined(_MSC_VER)
    SYSTEM_INFO SI;
    GetSystemInfo(&SI);
    return SI.dwNumberOfProcessors;
#else
    return 1;
#endif
}



inline HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}


//------------------------------------------------------------------------
inline void LogLastError(const char* file, int line, const char* function)
{
	char buf[2048];

	DWORD err = GetLastError();
	if (err == 0)
		return;

	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	sprintf_s(buf, 2048, "%s(%d): %s() - %s - %s \n", file, line, function, lpMsgBuf);
	OutputDebugStringA(buf);

	LocalFree(lpMsgBuf);
}

#define FB_LOG_LAST_ERROR() LogLastError(__FILE__, __LINE__, __FUNCTION__)