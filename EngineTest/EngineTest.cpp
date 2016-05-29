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

// EngineTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EngineTest.h"
#include "MeshTest.h"
#include "SkyBoxTest.h"
#include "ParticleTest.h"
#include "AudioTest.h"
#include "VideoTest.h"
#include "TextTest.h"
#include "LuaTest.h"
#include "FractalTest.h"
#include "GenerateNoise.h"
#include "ComputeShaderTest.h"
#include "TaskTest.h"

#include "FBCommonHeaders/Helpers.h"
#include "FBEngineFacade/EngineFacade.h"
#include "FBFileSystem/FileSystem.h"
#include "FBLua/LuaObject.h"
using namespace fb;
EngineFacadePtr gEngine;
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND gHWnd = 0;
MeshTestPtr gMeshTest;
SkyBoxTestPtr gSkyBoxTest;
ParticleTestPtr gParticleTest;
AudioTestPtr gAudioTest;
VideoTestPtr gVideoTest;
TextTestPtr gTextTest;
LuaTestPtr gLuaTest;
FractalTestPtr gFractalTest;
GenerateNoisePtr gGenerateNoise;
ComputeShaderTestPtr gComputeShaderTest;
TaskTestPtr gTaskTest;

int _FBPrint(lua_State* L);


void UpdateFrame(){
	gpTimer->Tick();
	auto dt = gpTimer->GetDeltaTime();
	gEngine->UpdateFileMonitor();
	gEngine->UpdateInput();
	gEngine->Update(dt);

	if (gParticleTest)
		gParticleTest->Update(dt);
	if (gMeshTest)
		gMeshTest->Update(dt);
	if (gTextTest)
		gTextTest->Update();
	if (gAudioTest)
		gAudioTest->Update(dt);

	gEngine->Render();
	gEngine->EndInput();
}

void InitEngine(){
	gEngine = EngineFacade::Create();
	RECT rect;
	GetClientRect(gHWnd, &rect);
	auto width = rect.right - rect.left;
	auto height = rect.bottom - rect.top;
	gEngine->InitRenderer("FBRendererD3D11");
	gEngine->InitCanvas((HWindow)gHWnd);
}

void DeinitEngine(){
	gEngine = 0;
}

void StartTest(){
	gEngine->SetEnvironmentMap("data/environment.dds");
	gEngine->SetMainCameraPos(Vec3(0, -5, 0));
	gEngine->EnableCameraInput(true);	
	auto L = LuaUtils::GetLuaState();
	LUA_SETCFUNCTION(L, _FBPrint);
	
	gGenerateNoise = GenerateNoise::Create();

	gMeshTest = MeshTest::Create();	
	gMeshTest->SetCameraTarget();
	//gSkyBoxTest = SkyBoxTest::Create();
	//gParticleTest = ParticleTest::Create();
	//gAudioTest = AudioTest::Create();
	//gVideoTest = VideoTest::Create();	
	//gTextTest = TextTest::Create();
	//gLuaTest = LuaTest::Create();	
	//gFractalTest = FractalTest::Create();
	//gEngine->AddRendererObserver(IRendererObserver::DefaultRenderEvent, gFractalTest);
	//gComputeShaderTest = ComputeShaderTest::Create();
	gTaskTest = TaskTest::Create();
}

void EndTest(){
	gEngine->PrepareQuit();
	gTaskTest = 0;
	gFractalTest = 0;
	gTextTest = 0;
	gVideoTest = 0;
	gParticleTest = 0;
	gMeshTest = 0;
	gSkyBoxTest = 0;
	gAudioTest = 0;
	gLuaTest = 0;
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int testarray[][3] = {
		{1, 2, 3},
		{ 1, 2, 3 },
		{ 1, 2, 3 },
		{ 1, 2, 3 },
		{ 1, 2, 3 },

	};
	int count = ARRAYCOUNT(testarray);

 	// TODO: Place code here.
	MSG msg = {};
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ENGINETEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	InitEngine();
	StartTest();

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENGINETEST));

	// Main message loop:
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
	EndTest();
	DeinitEngine();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENGINETEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ENGINETEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT rect = { 0, 0, 1600, 900 };

   AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
   gHWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

   if (!gHWnd)
   {
      return FALSE;
   }

   ShowWindow(gHWnd, nCmdShow);
   UpdateWindow(gHWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return gEngine->WinProc((HWindow)hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

int _FBPrint(lua_State* L)
{
	std::string msg = std::string("Lua: ") + LuaUtils::checkstring(L, -1);
	std::wstring tagRemoved = EngineFacade::GetInstance().StripTextTags(msg.c_str());
	Logger::Log(FB_DEFAULT_LOG_ARG, fb::WideToAnsi(tagRemoved.c_str()));	
	return 0;
}