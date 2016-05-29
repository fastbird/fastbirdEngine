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

#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBThread/AsyncObjects.h"
#include <thread>
typedef void *HANDLE;
namespace fb
{

	//---------------------------------------------------------------------------
	struct ThreadInfo
	{
		char ThreadName[128];
		std::thread::id mThreadID;

		ThreadInfo()
		{
		}
	};

	extern __declspec(thread) ThreadInfo* GThreadDesc;
	void SwitchThread();
	void SetThreadName(DWORD ThreadID, const char* ThreadName);

	//---------------------------------------------------------------------------
	class FB_DLL_THREAD ThreadSafeCounter
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(ThreadSafeCounter);

	public:
		ThreadSafeCounter();
		ThreadSafeCounter(int _Value);
		~ThreadSafeCounter();

		int operator *() const;
		int operator ++();
		int operator --();
		int operator +=(int Amount);
		int operator -=(int Amount);
	};
	//---------------------------------------------------------------------------
	class Thread;
	FB_DECLARE_SMART_PTR_STRUCT(ThreadHandle);
	struct ThreadHandle
	{
	private:
		FB_DECLARE_PIMPL_NON_COPYABLE(ThreadHandle);

	public:
		ThreadHandle(Thread* Thread, int StackSize, char* ThreadName);
		~ThreadHandle();
		bool IsValid();
		HANDLE GetNativeHandle();
		void Join();
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(Thread);
	class FB_DLL_THREAD Thread
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(Thread);
	protected:
		Thread();
		~Thread();

	public:
		ThreadInfo* mThreadDesc;
		static void RegisterThread(const char* ThreadName);

		void CreateThread(int StackSize, const char* ThreadName);
		void RegisterThread();

		void StartRun();
		void EndRun();

		bool IsForceExit();
		void ForceExit(bool Wait);
		bool IsRunning();
		bool IsJoinable();
		void Join();

		// Interface
		virtual bool Init() { return true; }
		// Returns 'repeat?' flag.
		virtual bool Run() = 0;
		virtual void Exit() {}
	};
}