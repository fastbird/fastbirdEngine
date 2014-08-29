#pragma once
#include <thread>
#include <mutex>
namespace fastbird
{

	//---------------------------------------------------------------------------
	struct FB_CRITICAL_SECTION
	{
		FB_CRITICAL_SECTION()
		{
		}

		~FB_CRITICAL_SECTION()
		{
		}

		void Lock()
		{
			mMutex.lock();
		}

		void Unlock()
		{
			mMutex.unlock();
		}

	private:

		std::mutex mMutex;
	};

	//---------------------------------------------------------------------------
	struct LOCK_CRITICAL_SECTION
	{
		LOCK_CRITICAL_SECTION(FB_CRITICAL_SECTION* cs)
		:mCS(cs)
		{
			mCS->Lock();
		}
		LOCK_CRITICAL_SECTION(FB_CRITICAL_SECTION& cs)
			:mCS(&cs)
		{
			mCS->Lock();
		}
		~LOCK_CRITICAL_SECTION()
		{
			mCS->Unlock();
		}

		FB_CRITICAL_SECTION* mCS;
	};

	//---------------------------------------------------------------------------
	struct FB_READ_WRITE_CS
	{
	public:
		FB_READ_WRITE_CS()
			: mNumReaders(0)
		{
			mReaderCleared = CreateEvent(NULL, TRUE, TRUE, NULL);
		}
		~FB_READ_WRITE_CS()
		{
			WaitForSingleObject(mReaderCleared, INFINITE);
			CloseHandle(mReaderCleared);
		}
		void EnterReader()
		{
			mWriteCS.Lock();
			mReaderCountCS.Lock();
			if (++mNumReaders == 1)
				ResetEvent(mReaderCleared);
			mReaderCountCS.Unlock();
			mWriteCS.Unlock();
		}
		void LeaveReader()
		{
			mReaderCountCS.Lock();
			if (--mNumReaders == 0)
				SetEvent(mReaderCleared);
			mReaderCountCS.Unlock();			
		}
		void EnterWriter()
		{
			mWriteCS.Lock();
			WaitForSingleObject(mReaderCleared, INFINITE);
		}
		void LeaveWriter()
		{
			mWriteCS.Unlock();
		}
	private:
		FB_CRITICAL_SECTION mWriteCS;
		FB_CRITICAL_SECTION mReaderCountCS;
		volatile int mNumReaders;
		HANDLE mReaderCleared;
	};

	//-----------------------------------------------------------------------
	struct READ_LOCK
	{
		READ_LOCK(FB_READ_WRITE_CS& lock)
		: mReadWrite(&lock)
		{
			mReadWrite->EnterReader();
		}
		~READ_LOCK()
		{
			mReadWrite->LeaveReader();
		}

		FB_READ_WRITE_CS* mReadWrite;
	};

	struct WRITE_LOCK
	{
		WRITE_LOCK(FB_READ_WRITE_CS& lock)
		: mReadWrite(&lock)
		{
			mReadWrite->EnterWriter();
		}
		~WRITE_LOCK()
		{
			mReadWrite->LeaveWriter();
		}

		FB_READ_WRITE_CS* mReadWrite;
	};

	//-----------------------------------------------------------------------
	class SyncBase
	{
	public:
		virtual ~SyncBase() {}
		virtual void Lock() = 0;
		virtual void Unlock() = 0;
	};

	//
	// Event
	//
	class SyncEvent : public SyncBase
	{
	public:
		virtual void Trigger() = 0;
		virtual void Reset() = 0;
		virtual bool Wait(DWORD WaitTime = 0xffffffff) = 0;
	};

	SyncEvent* CreateSyncEvent(bool ManualReset = FALSE, char* Name = NULL);
	void DeleteSyncEvent(SyncEvent* s);
}

