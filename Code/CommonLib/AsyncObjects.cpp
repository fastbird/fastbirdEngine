#include <CommonLib/StdAfx.h>
#include <CommonLib/AsyncObjects.h>

using namespace fastbird;

// Synchronization
class SyncEventWin : public SyncEvent
{
protected:
	HANDLE EventHandle;

public:
	SyncEventWin(bool ManualReset, char* Name)
	{
		EventHandle = CreateEventA(NULL, ManualReset, 0, Name);
		CHECK(EventHandle);
	}

	~SyncEventWin()
	{
		CloseHandle(EventHandle);
	}

	void Trigger()
	{
		SetEvent(EventHandle);
	}

	void Reset()
	{
		ResetEvent(EventHandle);
	}

	bool Wait(DWORD WaitTime)
	{
		return WaitForSingleObject(EventHandle, WaitTime) == WAIT_OBJECT_0;
	}

	void Lock()
	{
		WaitForSingleObject(EventHandle, INFINITE);
	}

	void Unlock()
	{
	}
};

SyncEvent* fastbird::CreateSyncEvent(bool ManualReset, char* Name)
{
	return FB_NEW(SyncEventWin)(ManualReset, Name);
}
void fastbird::DeleteSyncEvent(SyncEvent* s)
{
	FB_DELETE(s);
}