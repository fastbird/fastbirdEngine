#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <set>
#include <list>
#include <assert.h>
#include <Engine/GlobalEnv.h>

extern fastbird::GlobalEnv* gFBEnv;
namespace fastbird
{	
	void Log(const char* szFmt, ...);
	void Error(const char* szFmt, ...);
	typedef std::vector<char> BYTE_BUFFER;

	// name : like Engine.dll or UI.dll etc..
	// this function will append postfix. so the final name will be
	// Engine_Debug.dll or UI_Debug.dll in Debug build.
	// Engine_Release.dll or Engine_Release.dll in Release build.
	HMODULE LoadFBLibrary(const char* name);
	void FreeFBLibrary(HMODULE module);
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

template <class T>
void ClearWithSwap(T& m)
{
	T empty;
	std::swap(m, empty);
}

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
#include <CommonLib/Math/fbMath.h>
#include <CommonLib/tinyxml2.h>