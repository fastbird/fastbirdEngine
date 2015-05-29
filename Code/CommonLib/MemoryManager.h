#pragma once

#include <exception>
#include <assert.h>


#ifdef USING_FB_MEMORY_MANAGER
namespace fastbird
{
	void FBReportMemoryForModule();
	void* AllocBytes(size_t size, const char* file, size_t line, const char* func);
	void* AllocBytesAligned(size_t size, size_t align, const char* file, size_t line, const char* func);	
	void DeallocBytes(void* prt, const char* file, size_t line, const char* func);
	void DeallocBytesAligned(void* ptr, const char* file, size_t line, const char* func);
	//void PrepareDelete(void* ptr, const char* file, size_t line, const char* func);
	//void PrepareDeleteArr(void* ptr, const char* file, size_t line, const char* func);
	template <typename T>
	inline T* ConstructN(T* startp, size_t num)
	{
		for (size_t i = 0; i < num; ++i)
		{
			new (startp + i) T();
		}
		return startp;
	}

	template <typename T>
	inline void DestructN(T* startp)
	{
		size_t num = _msize(startp) / sizeof(T);
		for (size_t i = 0; i < num; ++i)
		{
			T* instance = (startp + i);
			instance->~T();
		}
	}

	template <typename T>
	inline void Delete(T* p, const char* file, size_t line, const char* func)
	{
		if (!p)
			return;
		p->~T();
		DeallocBytes(p, file, line, func);
	}

	template <typename T>
	inline void DeleteAligned(T* p, const char* file, size_t line, const char* func)
	{
		if (!p)
			return;
		p->~T();
		DeallocBytesAligned(p, file, line, func);
	}

	template <typename T>
	inline void DeleteArr(T* p, const char* file, size_t line, const char* func)
	{		
		if (!p)
			return;
		fastbird::DestructN(p);
		DeallocBytes(p, file, line, func);
	}
}

#define FB_NEW(T) new (fastbird::AllocBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
#define FB_ARRNEW(T, count) fastbird::ConstructN(static_cast<T*>(fastbird::AllocBytes(sizeof(T)*count, __FILE__, __LINE__, __FUNCTION__)), count)
#define FB_DELETE(ptr) fastbird::Delete( (ptr), __FILE__, __LINE__, __FUNCTION__)
#define FB_ARRDELETE(ptr) fastbird::DeleteArr(ptr, __FILE__, __LINE__, __FUNCTION__)
#define FB_SAFE_DEL(ptr) (ptr) ? FB_DELETE((ptr)) : 0; (ptr) = 0;
#define FB_SAFE_ARRDEL(ptr) ptr ? FB_ARRDELETE(ptr) : 0; ptr = 0;
#define FB_NEW_ALIGNED(T, A) new (fastbird::AllocBytesAligned(sizeof(T), A, __FILE__, __LINE__, __FUNCTION__)) T
#define FB_DEL_ALIGNED(ptr) (ptr) ? fastbird::DeleteAligned( (ptr), __FILE__, __LINE__, __FUNCTION__) : 0;
#else
#define FB_NEW(T) new T
#define FB_ARRNEW(T, count) new T[count]
#define FB_DELETE(ptr) delete ptr
#define FB_ARRDELETE(ptr) delete[] ptr
#endif