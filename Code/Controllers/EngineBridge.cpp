#include "StdAfx.h"
#include "EngineBridge.h"
#include <Engine/GlobalEnv.h>
#include <Engine/DllMain.h>
#include <CommonLib/PerlinNoise.h>

namespace Controllers
{
	void EngineBridge::InitializeNativeEngine()
	{		
		fastbird::IEngine* pEngine = ::Create_fastbird_Engine();
		pEngine->InitEngine(fastbird::IEngine::D3D11);
	}

	void EngineBridge::FinalizeNativeEngine()
	{
		::Destroy_fastbird_Engine();
	}

	int EngineBridge::InitSwapChain(HANDLE hwnd, int width, int height)
	{
		//return gEnv->pEngine->InitSwapChain((HWND)hwnd, width, height);
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
		//gEnv->pEngine->Error(buf);
		//assert(0);
	}

	void Log(const char* szFmt, ...)
	{
		char buf[2048];
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		//gEnv->pEngine->Log(buf);
	}
}