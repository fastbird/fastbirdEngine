#include "StdAfx.h"
#include "InputHandler.h"
#include "CameraMan.h"
#include "QuickSortTask.h"
#include "PhyObj.h"
#include <Engine/IVoxelizer.h>
#include <Engine/IRenderTarget.h>
#include <CommonLib/threads.h>
#include <CommonLib/Profiler.h>
#include <Physics/IPhysics.h>
#include <Physics/RigidBody.h>
using namespace fastbird;

#define RUN_PARALLEL_EXAMPLE 0

fastbird::GlobalEnv* gFBEnv = 0;
fastbird::IPhysics* gFBPhysics = 0;
HMODULE gEngineModule = 0;
HMODULE gPhysicsModule = 0;

CameraMan* gCameraMan = 0;
InputHandler* gInputHandler = 0;
TaskScheduler* gTaskSchedular = 0;
IMeshObject* gMeshObject = 0;
std::vector< IMeshObject* > gVoxels;

//-----------------------------------------------------------------------------
void UpdateFrame()
{
	gFBEnv->pTimer->Tick();
	float elapsedTime = gpTimer->GetDeltaTime();
	gFBEnv->pEngine->UpdateInput();
	gFBPhysics->Update(elapsedTime);
	gCameraMan->Update(elapsedTime);
	gFBEnv->pEngine->UpdateFrame(elapsedTime);
}

//-----------------------------------------------------------------------------
void RunGame()
{
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateFrame();
		}
	}
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK WinProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
					 PostQuitMessage(0);
	}
		break;

	}
	return gFBEnv->pEngine->WinProc(window, msg, wp, lp);
}



//-----------------------------------------------------------------------------
bool InitEngine()
{
	//-------------------------------------------------------------------------
	// Engine
	//-------------------------------------------------------------------------
	gEngineModule = fastbird::LoadFBLibrary("Engine.dll");
	if (!gEngineModule)
		return false;
	typedef fastbird::IEngine* (__cdecl *CreateEngineProc)();
	CreateEngineProc createEngineProc = (CreateEngineProc)GetProcAddress(gEngineModule, "Create_fastbird_Engine");
	if (!createEngineProc)
		return false;

	fastbird::IEngine* pEngine = createEngineProc();
	gFBEnv = pEngine->GetGlobalEnv();
	pEngine->InitEngine(fastbird::IEngine::D3D11);
	fastbird::HWND_ID id = pEngine->CreateEngineWindow(0, 0, 1600, 900, "EngineApp", "Game powered by fastbird engine", WinProc);	
	pEngine->InitSwapChain(id, 1600, 900);
	gpTimer = gFBEnv->pTimer;

	if (gFBEnv->pRenderer)
	{
		gFBEnv->pRenderer->SetClearColor(0, 0, 0, 1);
	}
	gInputHandler = FB_NEW(InputHandler)();
	gCameraMan = FB_NEW(CameraMan)(gFBEnv->pRenderer->GetCamera());
	gFBEnv->pEngine->AddInputListener(gInputHandler, IInputListener::INPUT_LISTEN_PRIORITY_INTERACT, 0);

	gTaskSchedular = FB_NEW(TaskScheduler)(6);


	//-------------------------------------------------------------------------
	// Physics
	//-------------------------------------------------------------------------
	gPhysicsModule = fastbird::LoadFBLibrary("Physics.dll");
	if (!gPhysicsModule)
		return false;
	
	typedef fastbird::IPhysics* (__cdecl *CreatePhysicsProc)();
	CreatePhysicsProc createPhysicsProc = (CreatePhysicsProc)GetProcAddress(gPhysicsModule, "Create_fastbird_Physics");
	if (!createPhysicsProc)
		return false;

	gFBPhysics = createPhysicsProc();

	return true;
}

void FinalizeEngine()
{
	typedef void(__cdecl *DestroyPhysicsProc)();
	DestroyPhysicsProc destroyPhysicsProc = (DestroyPhysicsProc)GetProcAddress(gPhysicsModule, "Destroy_fastbird_Physics");
	if (!destroyPhysicsProc)
		return;
	destroyPhysicsProc();
	fastbird::FreeFBLibrary(gPhysicsModule);


	typedef void(__cdecl *DestroyEngineProc)();
	DestroyEngineProc destroyEngineProc = (DestroyEngineProc)GetProcAddress(gEngineModule, "Destroy_fastbird_Engine");
	if (!destroyEngineProc)
		return;
	destroyEngineProc();
	fastbird::FreeFBLibrary(gEngineModule);
	gFBEnv = 0;
}

//-----------------------------------------------------------------------------
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::vector<int> test;
	test.push_back(0);
	test.push_back(0);


	//----------------------------------------------------------------------------
	// 1. How to init engine.
	//----------------------------------------------------------------------------
	bool succ = InitEngine();

	if (!succ)
	{
		// failed to initialize!
		return 1;
	}

	//----------------------------------------------------------------------------
	// 2. How to create sky -  see Data/Materials/skybox.material
	//----------------------------------------------------------------------------
	gFBEnv->pEngine->CreateSkyBox();

	//----------------------------------------------------------------------------
	// 3. How to load model file and material.
	//----------------------------------------------------------------------------
	gMeshObject = gFBEnv->pEngine->GetMeshObject("data/objects/CommandModule/CommandModule.dae"); // using collada file.
	if (gMeshObject)
	{
		//gMeshObject->AttachToScene();
		gMeshObject->SetMaterial("data/objects/CommandModule/CommandModule.material");
	}


	//----------------------------------------------------------------------------
	// 4. How to use voxelizer (Need to include <Engine/IVoxelizer.h>)
	//----------------------------------------------------------------------------
	IVoxelizer* voxelizer = gFBEnv->pEngine->CreateVoxelizer();
	bool ret = voxelizer->RunVoxelizer("data/objects/etc/spaceship.dae", 64, false, true);
	assert(ret);
	IMeshObject* voxelObject = gFBEnv->pEngine->GetMeshObject("data/objects/etc/cube.dae");
	const IVoxelizer::HULLS& h = voxelizer->GetHulls();
	Vec3 offset(30, 0, 0);
	for (const auto& v : h)
	{
		gVoxels.push_back((IMeshObject*)voxelObject->Clone());
		IMeshObject* m = gVoxels.back();
		m->SetPos(offset + v*2.0f);
		//m->AttachToScene();
	}
	gFBEnv->pEngine->DeleteVoxelizer(voxelizer);
	gFBEnv->pEngine->ReleaseMeshObject(voxelObject);
	voxelObject = 0;

	//----------------------------------------------------------------------------
	// 5. How to use Render To Texture (Need to include <Engine/IRenderTarget.h>)
	//----------------------------------------------------------------------------
	if (gMeshObject)
	{
		RenderTargetParam param;
		param.mEveryFrame = false;
		param.mSize = Vec2I(1024, 1024);
		param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
		param.mShaderResourceView = true;
		param.mMipmap = false;
		param.mCubemap = false;
		param.mHasDepth = false;
		param.mUsePool = false;
		IRenderTarget* rtt = gFBEnv->pRenderer->CreateRenderTarget(param);
		rtt->GetScene()->AttachObject(gMeshObject);
		ICamera* pRTTCam = rtt->GetCamera();
		pRTTCam->SetPos(Vec3(-5, 0, 0));
		pRTTCam->SetDir(Vec3(1, 0, 0));
		rtt->Render();
		rtt->GetRenderTargetTexture()->SaveToFile("rtt.png");
		gFBEnv->pRenderer->DeleteRenderTarget(rtt);
	}

#if RUN_PARALLEL_EXAMPLE
	//----------------------------------------------------------------------------
	// 6. How to parallel computing. (Need to include <CommonLib/threads.h>)
	// reference : Efficient and Scalable Multicore Programming
	//----------------------------------------------------------------------------
	int numInts = INT_MAX / 100;
	int* pInts = FB_ARRNEW(int, numInts);
	{
		//------------------------------------------------------------------------
		// 7. how to profile (Need to include <CommonLib/Profiler.h>)
		//------------------------------------------------------------------------
		Profiler pro("RandomGeneration");
		DWORD random = 0;
		for (int i = 0; i < numInts; i++)
		{
			pInts[i] = random & 0x7fffffff;
			random = random * 196314165 + 907633515;
		}
	}
	QuickSortTask* pQuickSort = FB_NEW(QuickSortTask)(pInts, 0, numInts, 0);
	// single threaded
	{
		Profiler pro("QuickSort_SingleThread.");
		pQuickSort->Execute(0);
	}
	for (int i = 0; i<numInts - 1; i++)
	{
		assert(pInts[i] <= pInts[i + 1]);
	}
	FB_SAFE_DEL(pQuickSort);

	DWORD random = 0;
	for (int i = 0; i < numInts; i++)
	{
		pInts[i] = random & 0x7fffffff;
		random = random * 196314165 + 907633515;
	}
	// multi threaded.
	pQuickSort = FB_NEW(QuickSortTask)(pInts, 0, numInts, 0);
	gTaskSchedular->AddTask(pQuickSort);
	{
		Profiler pro("QuickSort_MultiThread.");
		pQuickSort->Sync(); // wait to finish
	}
	for (int i = 0; i<numInts - 1; i++)
	{
		assert(pInts[i] <= pInts[i + 1]);
	}
	FB_SAFE_DEL(pQuickSort);

	FB_ARRDELETE(pInts);
#endif // RUN_PARALLEL_EXAMPLE
	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	// Physics Test
	//----------------------------------------------------------------------------
	std::vector<PhyObj*> PhyObjs;
	PhyObjs.reserve(200);
	for (int i = 0; i < 100; i++)
	{
		Transformation transform;
		transform.SetDir(RandomDirection());
		transform.SetTranslation(Vec3(-20, 0, 0) + Random(Vec3(-10, -10, -10), Vec3(10, 10, 10)));
		float mass = Random(0.5f, 2.f);
		PhyObjs.push_back(
			FB_NEW(PhyObj)(transform, mass, 1, 0xffffffff, 0.01f, 0.01f)
			);
		auto obj = PhyObjs.back();
		obj->SetMeshObj((IMeshObject*)gMeshObject->Clone());
		obj->CreateRigidBody();
		auto rigidBody = obj->GetRigidBody();
		rigidBody->RegisterToWorld();
		rigidBody->ApplyCentralImpulse(Random(Vec3(10, -10, -10), Vec3(10, 10, 10)));

	}

	for (int i = 0; i < 100; i++)
	{
		Transformation transform;
		transform.SetDir(RandomDirection());
		transform.SetTranslation(Vec3(20, 0, 0) + Random(Vec3(-10, -10, -10), Vec3(10, 10, 10)));
		float mass = Random(0.5f, 2.f);
		PhyObjs.push_back(
			FB_NEW(PhyObj)(transform, mass, 1, 0xffffffff, 0.01f, 0.01f)
			);
		auto obj = PhyObjs.back();
		obj->SetMeshObj((IMeshObject*)gMeshObject->Clone());
		obj->CreateRigidBody();
		auto rigidBody = obj->GetRigidBody();
		rigidBody->RegisterToWorld();
		rigidBody->ApplyCentralImpulse(Random(Vec3(-10, -10, -10), Vec3(-10, 10, 10)));

	}

	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	// Entering game loop.
	//----------------------------------------------------------------------------
	RunGame();
	gTaskSchedular->Finalize();

	gFBEnv->pEngine->ReleaseMeshObject(gMeshObject);
	for (auto& m : gVoxels)
	{
		gFBEnv->pEngine->ReleaseMeshObject(m);
	}
	for (auto obj : PhyObjs)
	{
		FB_DELETE(obj);
	}
	PhyObjs.clear();
	FB_SAFE_DEL(gCameraMan);
	FB_SAFE_DEL(gInputHandler);
	FB_SAFE_DEL(gTaskSchedular);
	if (gFBEnv)
	{
		FinalizeEngine();
	}
}

//---------------------------------------------------------------------------
namespace fastbird
{
	void Error(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		gFBEnv->pEngine->Error(buf);
		//assert(0);
	}

	void Log(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		gFBEnv->pEngine->Log(buf);
	}
} // namespace fastbird