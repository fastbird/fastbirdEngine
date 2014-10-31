#include <CommonLib/StdAfx.h>
#include <CommonLib/LockFreeQueue.h>

namespace fastbird
{

	void TestLFQ()
	{
		LockFreeQueue<int> m;
		m.Enq(0);
	}
}