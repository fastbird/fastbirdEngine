#pragma once
#include <vector>
#include <memory>
#include <assert.h>
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
for (; it != it ## End; ++it)

#define if_assert_pass(V) assert((V)); \
if ((V))

#define if_assert_fail(V) assert((V)); \
if (!(V))

#define ValueNotExistInVector(arr, v)	(std::find(arr.begin(), arr.end(), v) == arr.end())
#define DeleteValuesInVector(arr, v) \
	unsigned arr ## SizeBefore = arr.size(); \
	arr.erase(std::remove(arr.begin(), arr.end(), v), arr.end()); \
	unsigned arr ## SizeAfter = arr.size();


#if defined(_DEBUG)
#define CHECK(exp)          if(!(exp)) { DebugBreak(); } else {}
#define VERIFY(exp)         if(!(exp)) { DebugBreak(); } else {}
#else
#define CHECK(exp)
#define VERIFY(exp)         (exp)
#endif

template <typename T>
bool operator == (const std::weak_ptr<T>& a, const std::weak_ptr<T>& b)
{
	return a.lock() == b.lock();
}

template <typename T>
bool operator != (const std::weak_ptr<T>& a, const std::weak_ptr<T>& b)
{
	return !(a.lock() == b.lock());
}

template <typename T>
bool operator == (const std::weak_ptr<T>& a, const std::shared_ptr<T>& b)
{
	return a.lock() == b;
}

template <typename T>
bool operator != (const std::weak_ptr<T>& a, const std::shared_ptr<T>& b)
{
	return !(a.lock() == b);
}

// not change often.
#include <CommonLib/System.h>