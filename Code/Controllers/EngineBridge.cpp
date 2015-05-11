#include "StdAfx.h"
#include "EngineBridge.h"
#include <Engine/GlobalEnv.h>
#include <CommonLib/PerlinNoise.h>

fastbird::GlobalEnv* gFBEnv;
HMODULE gEngineModule = 0;
namespace Controllers
{
	void EngineBridge::InitializeNativeEngine()
	{		
		gEngineModule = fastbird::LoadFBLibrary("Engine.dll");
		if (!gEngineModule)
			return;
		typedef fastbird::IEngine* (__cdecl *CreateEngineProc)();
		CreateEngineProc createEngineProc = (CreateEngineProc)GetProcAddress(gEngineModule, "Create_fastbird_Engine");
		if (!createEngineProc)
			return;

		fastbird::IEngine* pEngine = createEngineProc();
		gFBEnv = pEngine->GetGlobalEnv();

		pEngine->InitEngine(fastbird::IEngine::D3D11);
	}

	void EngineBridge::FinalizeNativeEngine()
	{
		typedef void(__cdecl *DestroyEngineProc)();
		DestroyEngineProc destroyEngineProc = (DestroyEngineProc)GetProcAddress(gEngineModule, "Destroy_fastbird_Engine");
		if (!destroyEngineProc)
			return;
		destroyEngineProc();
		fastbird::FreeFBLibrary(gEngineModule);
	}

	int EngineBridge::InitSwapChain(fastbird::HWND_ID id, int width, int height)
	{
		//return gFBEnv->pEngine->InitSwapChain((HWND)hwnd, width, height);
		return 0;
	}

	array<Byte, 2>^ EngineBridge::GeneratePerlin(int width, int height, float persistence, float size)
	{
		array<Byte, 2>^ bytes = gcnew array<Byte, 2>(width, height);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				bytes[i, j] = Byte((fastbird::PerlinNoise2D((float)i / size, (float)j / size, persistence)*.5f + .5f) * 255.0f);
			}
		}
		return bytes;
	}

}

namespace fastbird
{
	void Error(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		//gFBEnv->pEngine->Error(buf);
		//assert(0);
	}

	void Log(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		//gFBEnv->pEngine->Log(buf);
	}
}