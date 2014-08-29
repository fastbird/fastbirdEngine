#pragma once
#include <vector>
#include <Engine/GlobalEnv.h>

extern fastbird::GlobalEnv* gEnv;
namespace fastbird
{	
	void Log(const char* szFmt, ...);
	void Error(const char* szFmt, ...);
	typedef std::vector<char> BYTE_BUFFER;
}

#define ARRAYCOUNT(A)       (sizeof(A) / sizeof(A[0]))

#define FB_FOREACH(it, container) auto it = (container).begin();\
	auto it ## End = (container).end(); \
	for (; it!=it ## End; ++it)


#if defined(_DEBUG)
#define CHECK(exp)          if(!(exp)) { DebugBreak(); } else {}
#define VERIFY(exp)         if(!(exp)) { DebugBreak(); } else {}
#else
#define CHECK(exp)
#define VERIFY(exp)         (exp)
#endif
