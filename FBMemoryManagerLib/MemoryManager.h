/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

/**
\file FBMemoryManager.h
Provide a ceteralized method for memory allocation and deallocation
\author Jungwan Byun
\defgroup FBMemoryManagerLib
Provide a ceteralized method for memory allocation and deallocation
*/
#pragma once
#include <cstddef>
#ifdef NOT_USING_FB_MEMORY_MANAGER
#define FB_NEW(T) new T
#define FB_ARRAY_NEW(T, count) new T[count]
#define FB_DELETE(ptr) delete ptr
#define FB_ARRAY_DELETE(ptr) delete[] ptr
#else

namespace fb
{
	/// \ingroup FBMemoryManager
	void FBReportMemoryForModule();
    void* AllocBytes(size_t size, const char* file, size_t line, const char* func);
	void* AllocBytesAligned(size_t size, size_t align, const char* file, size_t line, const char* func);
	void DeallocBytes(void* prt, const char* file, size_t line, const char* func);
	void DeallocBytesAligned(void* ptr, const char* file, size_t line, const char* func);
	
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
		fb::DestructN(p);
		DeallocBytes(p, file, line, func);
	}
}
/// \addtogroup FBMemoryManagerLib
/// @{
#define FB_NEW(T) new (fb::AllocBytes(sizeof(T), __TFILE__, __LINE__, __TFUNCTION__)) T
#define FB_ARRAY_NEW(T, count) fb::ConstructN(static_cast<T*>(fb::AllocBytes(sizeof(T)*count, __TFILE__, __LINE__, __TFUNCTION__)), count)
#define FB_DELETE(ptr) fb::Delete( (ptr), __TFILE__, __LINE__, __TFUNCTION__)
#define FB_ARRAY_DELETE(ptr) fb::DeleteArr(ptr, __TFILE__, __LINE__, __TFUNCTION__)
#define FB_SAFE_DELETE(ptr) (ptr) ? FB_DELETE((ptr)) : 0; (ptr) = 0;
#define FB_NEW_ALIGNED(T, A) new (fb::AllocBytesAligned(sizeof(T), A, __TFILE__, __LINE__, __TFUNCTION__)) T
#define FB_DELETE_ALIGNED(ptr) (ptr) ? fb::DeleteAligned( (ptr), __TFILE__, __LINE__, __TFUNCTION__) : 0;
/// @}
#endif