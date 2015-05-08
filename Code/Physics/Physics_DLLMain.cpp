#include <Physics/StdAfx.h>
#include <Physics/Physics.h>

fastbird::IPhysics* gFBPhysics = 0;

extern "C"
{
	//-------------------------------------------------------------------------
	__declspec(dllexport) fastbird::IPhysics* _cdecl Create_fastbird_Physics()
	{
		if (gFBPhysics)
			return gFBPhysics;

		auto pPhysics = FB_NEW(fastbird::Physics)();
		gFBPhysics = pPhysics;
		assert(gFBPhysics);
		pPhysics->Initilaize();

		return gFBPhysics;
	}

	//-------------------------------------------------------------------------
	__declspec(dllexport) void _cdecl Destroy_fastbird_Physics()
	{
		if (!gFBPhysics)
			return;

		fastbird::Physics* physics = (fastbird::Physics*)gFBPhysics;
		physics->Deinitilaize();
		FB_DELETE(physics);
		gFBPhysics = 0;

#ifdef USING_FB_MEMORY_MANAGER
			fastbird::FBReportMemoryForModule();
#endif
	}
}