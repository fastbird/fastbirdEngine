#include <CommonLib/StdAfx.h>
#include <CommonLib/MemoryManager.h>
#include <CommonLib/threads.h>
#include <map>

namespace fastbird
{
	unsigned long long gNumMemoryAllocation = 0;
	FB_CRITICAL_SECTION gMemCS;
	struct MemLoc
	{
		MemLoc(const char* file, size_t line, const char* func)
		{
			if (file)  mFile = file;
			mLine = line;
			if (func) mFunc = func;
		}
		MemLoc()
		{}
		std::string mFile;
		size_t mLine;
		std::string mFunc;
	};
	typedef std::map<void*, MemLoc> LINEDATA;
	LINEDATA& GetMemAllocLines()
	{
		static LINEDATA gMemoryAllocLines;
		return gMemoryAllocLines;
	}
	std::ofstream& GetMemStatFile()
	{
		static std::ofstream gMemoryFile("memory.txt");
		return gMemoryFile;
	}

	//-----------------------------------------------------------------------
	void* AllocBytes(size_t size, const char* file, size_t line, const char* func)
	{
		void* p = malloc(size);
		if (!p)
			throw std::bad_alloc();
		
		++gNumMemoryAllocation;
		LOCK_CRITICAL_SECTION lock(gMemCS);
		GetMemAllocLines()[p] = MemLoc(file, line, func);
		return p;
	}

	void* AllocBytesAligned(size_t size, size_t align, const char* file, size_t line, const char* func)
	{
		void* p = _aligned_malloc(size, align);
		if (!p)
			throw std::bad_alloc();

		++gNumMemoryAllocation;
		LOCK_CRITICAL_SECTION lock(gMemCS);
		GetMemAllocLines()[p] = MemLoc(file, line, func);
		return p;
	}

	//-----------------------------------------------------------------------
	void DeallocBytes(void* ptr, const char* file, size_t line, const char* func)
	{
		if (!ptr)
			return;
		LOCK_CRITICAL_SECTION lock(gMemCS);
		auto it = GetMemAllocLines().find(ptr);
		if_assert_pass(it != GetMemAllocLines().end())
		{
			/*if (file &&  it->second.mFile != file)
			{
			if (strstr(file, "smartptr.h") == 0 && strstr(file, "SmartPtr.h") == 0)
			{
			Log("Memory(%s, %d, %s) is not deleted in the file where it was allocated. deallocated = (%s, %d)",
			it->second.mFile.c_str(), it->second.mLine, it->second.mFunc.c_str(), file, line);
			}
			}*/
			GetMemAllocLines().erase(it);
		}
		--gNumMemoryAllocation;
		free(ptr);
	}

	//-----------------------------------------------------------------------
	void DeallocBytesAligned(void* ptr, const char* file, size_t line, const char* func)
	{
		if (!ptr)
			return;
		LOCK_CRITICAL_SECTION lock(gMemCS);
		auto it = GetMemAllocLines().find(ptr);
		assert(it != GetMemAllocLines().end());
		/*if (file && it->second.mFile != file)
		{
			Log("Memory(%s, %d, %s) is not deleted in the file where it was allocated. deallocated = (%s, %d)",
				it->second.mFile.c_str(), it->second.mLine, it->second.mFunc.c_str(), file, line);
		}*/
		GetMemAllocLines().erase(it);
		--gNumMemoryAllocation;
		_aligned_free(ptr);

	}

	//-----------------------------------------------------------------------
	void FBReportMemoryForModule()
	{
		LINEDATA& m = GetMemAllocLines();
		auto it = m.begin();
		auto itEnd = m.end();
		for (; it != itEnd; ++it)
		{
			char buffer[500];
			sprintf_s(buffer, "%s(%d) : memory(%x) not released \n", it->second.mFile.c_str(), it->second.mLine, it->first);
			OutputDebugString(buffer);
		}
	}
}