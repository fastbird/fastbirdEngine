//#define NOMINMAX
//#include <CommonLib/Config.h>
//#include <Windows.h>
//#include <iostream>
//#include <string>
//#include <vector>
//#include <set>
//#include <assert.h>
//#include <math.h>
//#include <stdlib.h>
//#include <FreeImage.h>
//
//#pragma comment(lib, "FreeImage.lib")
//#pragma comment(lib, "CommonLib.lib")
//#ifdef _DEBUG
//#pragma comment(lib, "Engine_Debug.lib")
//#else
//#pragma comment(lib, "Engine.lib")
//#endif
//#pragma comment(lib, "lua.lib")
//
//extern "C" {
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
//
//#include <CommonLib/Math/fbMath.h>
//#include <CommonLib/PerlinNoise.h>
//#include <Engine/IEngine.h>
//#include <Engine/DllMain.h>
//#include <Engine/GlobalEnv.h>
//#include <Engine/IRenderer.h>
//
//
//
//
//using namespace fastbird;
//
//
//BYTE permutation[] = { 151, 160, 137, 91, 90, 15,
//131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
//190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
//88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
//77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
//102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
//135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
//5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
//223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
//129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
//251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
//49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
//138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
//};
//
//static float g[] = {
//
//	1, 1, 0, 1, 
//	-1, 1, 0, 1,
//	1, -1, 0, 1,
//	-1, -1, 0, 1,
//
//	1, 0, 1, 1,
//	-1, 0, 1, 1,
//	1, 0, -1, 1,
//	-1, 0, -1, 1,
//
//	0, 1, 1, 1,
//	0, -1, 1, 1,
//	0, 1, -1, 1,
//	0, -1, -1, 1,
//
//	1, 1, 0, 1,
//	0, -1, 1, 1,
//	-1, 1, 0, 1,
//	0, -1, -1, 1
//};
//
//fastbird::GlobalEnv* gEnv = 0;
//void main()
//{
//
//	/*FreeImage_Initialise();
//	FIBITMAP* dib = FreeImage_Allocate(256, 1, 8);
//	int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
//	for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++) {
//		BYTE *bits = FreeImage_GetScanLine(dib, y);
//		for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++) {
//			// Set pixel color to green with a transparency of 128
//			*bits = permutation[x];
//			// jump to next pixel
//			bits += bytespp;
//		}
//	}
//	BOOL b = FreeImage_Save(FIF_JPEG, dib, "permutation.tif");
//
//	FreeImage_DeInitialise();*/
//
//	using namespace fastbird;
//	
//	fastbird::IEngine* pEngine = ::Create_fastbird_Engine();
//	pEngine->GetGlobalEnv(&gEnv);
//	//pEngine->CreateEngineWindow(0, 0, 1600, 900, "Game", WinProc);
//	pEngine->InitEngine(fastbird::IEngine::D3D11);
//	//pEngine->InitSwapChain(gEnv->pEngine->GetWindowHandle(), 1600, 900);
//
//	SmartPtr<ITexture> pTexture = 
//		gEnv->pRenderer->CreateTexture(g, 16, 1, PIXEL_FORMAT_R32G32B32A32_FLOAT, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEFAULT);
//	pTexture->SaveToFile("data/textures/gradient.dds");
//	SmartPtr<ITexture> pTexture2 =
//		gEnv->pRenderer->CreateTexture(permutation, 256, 1, PIXEL_FORMAT_R8_UNORM, BUFFER_USAGE_DEFAULT, BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEFAULT);
//	pTexture2->SaveToFile("data/textures/permutation.dds");
//	Destroy_fastbird_Engine();
//
//	int temp = 0;
//	temp++;
//}