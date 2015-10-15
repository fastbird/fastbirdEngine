// The "fastbird engine" library is distributed under the MIT license:
// 
// Copyright (c) 2013 fastbird(jungwan82@naver.com)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#pragma once
#ifndef IEngine_header_included
#define IEngine_header_included
#include <CommonLib/SmartPtr.h>
#include <vector>
#include <Engine/GlobalEnv.h>
#include <Engine/IInputListener.h>
#include <Engine/VideoPlayerType.h>

#define FB_DEFAULT_DEBUG_ARG "%s(%d): %s() - %s", __FILE__, __LINE__, __FUNCTION__
#define FB_LOG(msg) gFBEnv->pEngine->Log(FB_DEFAULT_DEBUG_ARG, (msg));
#define FB_LOG_LAST_ERROR_ENG() gFBEnv->pEngine->LogLastError(__FILE__, __LINE__, __FUNCTION__)
namespace fastbird
{
	struct GlobalEnv;
	class IRenderer;
	class ICamera;
	class IScene;
	class IFont;
	class Vec2I;
	class Vec3;
	class Vec2;
	class IUIObject;
	class IMeshObject;
	class IMeshGroup;
	class IParticleEmitter;
	class IFileChangeListener;
	class IObject;
	class ProfilerSimple;
	class IVoxelizer;
	class ISkySphere;
	class IBillboardQuad;
	class IDustRenderer;
	class TextManipulator;
	class LuaObject;
	class ITrailObject;
	class IVideoPlayer;
	
	typedef unsigned HWND_ID;
	static const HWND_ID INVALID_HWND_ID = (HWND_ID)-1;
	class IEngine : public ReferenceCounter
	{
	public:
		static IEngine* CreateInstance();
		static void DeleteInstance(IEngine* e);

		virtual ~IEngine(){}
		virtual GlobalEnv* GetGlobalEnv() const = 0;
		virtual HWND_ID CreateEngineWindow(int x, int y, int width, int height,
			const char* wndClass, const char* title, unsigned style, unsigned exStyle,
			WNDPROC winProc) = 0;
		virtual void DestroyEngineWindow(HWND_ID hwndId) = 0;
		virtual const Vec2I& GetRequestedWndSize(HWND hWnd) const = 0;
		virtual const Vec2I& GetRequestedWndSize(HWND_ID hWndId) const = 0;
		virtual HWND GetWindowHandle(HWND_ID id) const = 0;
		virtual HWND_ID GetWindowHandleId(HWND hWnd) const = 0;
		virtual HWND_ID GetWindowHandleIdWithMousePoint() const = 0;
		virtual HWND GetMainWndHandle() const = 0;
		virtual HWND GetForegroundWindow(HWND_ID* id = 0) const = 0;
		virtual HWND_ID GetForegroundWindowId() const = 0;
		virtual HWND_ID GetMainWndHandleId() const = 0;
		virtual bool IsMainWindowForground() const = 0;
		virtual bool InitSwapChain(HWND_ID id, int width, int height) = 0;
		enum RENDERER_TYPE 
		{ 
			D3D9=0, 
			D3D11=1, 
			OPENGL=2
		};
		virtual bool InitEngine(int rendererType) = 0;

		virtual inline IRenderer* GetRenderer() const = 0;		

		virtual void UpdateInput() = 0;
		virtual void UpdateFrame(float dt) = 0;
		
		// Terrain
		// numVertX * numVertY = (2^n+1) * (2^m+1)
		virtual bool CreateTerrain(int numVertX, int numVertY, float distance, const char* heightmapFile = 0) = 0;
		virtual bool CreateSkyBox() = 0;

		// priority : lower value processed first.
		virtual void AddInputListener(IInputListener* pInputListener, 
			IInputListener::INPUT_LISTEN_CATEGORY category, int priority) = 0;
		virtual void RemoveInputListener(IInputListener* pInputListener) = 0;

		struct MeshImportDesc
		{
			MeshImportDesc()
				: yzSwap(false), oppositeCull(true),
				useIndexBuffer(false), mergeMaterialGroups(false),
				keepMeshData(false), generateTangent(true)
			{
			}
			bool yzSwap;
			bool oppositeCull;
			bool useIndexBuffer;
			bool mergeMaterialGroups;
			bool keepMeshData;
			bool generateTangent;
		};
		virtual IMeshObject* GetMeshObject(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc()) = 0;
		virtual IMeshObject* CreateMeshObject() = 0;
		virtual IMeshGroup* GetMeshGroup(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc()) = 0;
		virtual void GetFractureMeshObjects(const char* daeFilePath, std::vector<IMeshObject*>& objects, bool reload=false) = 0;
		virtual const IMeshObject* GetMeshArchetype(const std::string& name) const = 0;
		virtual void ReleaseMeshObject(IMeshObject* p) = 0;
		virtual void ReleaseMeshGroup(IMeshGroup* p) = 0;
		virtual IParticleEmitter* GetParticleEmitter(const char* file, bool useSmartPtr) = 0;
		virtual IParticleEmitter* GetParticleEmitter(unsigned id, bool useSmartPtr) = 0;
		virtual void ReleaseParticleEmitter(IParticleEmitter* p) = 0;

		virtual ITrailObject* CreateTrailObject() = 0;
		virtual void ReleaseTrailObject(ITrailObject* trail) = 0;

		// usually you should not use these functions.
		virtual void GetMousePos(long& x, long& y) = 0;
		virtual bool IsMouseLButtonDown() const = 0;
		virtual IKeyboard* GetKeyboard() const = 0;
		virtual IMouse* GetMouse() const = 0;

		virtual std::string GetConfigStringValue(const char* section, const char* name) = 0;
		virtual int GetConfigIntValue(const char* section, const char* name) = 0;
		virtual bool GetConfigBoolValue(const char* section, const char* name) = 0;

		virtual void RegisterFileChangeListener(IFileChangeListener* listener) = 0;
		virtual void RemoveFileChangeListener(IFileChangeListener* listener) = 0;		

		// you have resposible to delete it.
		virtual IScene* CreateScene() = 0;
		virtual void DeleteScene(IScene* p) = 0;

		virtual void DrawProfileResult(ProfilerSimple& p, const char* posVarName, int tab = 0) = 0;
		virtual void DrawProfileResult(wchar_t* buf, const char* posVarName, int tab = 0) = 0;

#ifdef _FBENGINE_FOR_WINDOWS_
		// return processed
		virtual LRESULT WinProc( HWND window, UINT msg, WPARAM wp, LPARAM lp ) = 0;
#elif _FBENGINE_FOR_LINUX_

#endif

		virtual void Log(const char* szFmt, ...) const = 0;
		virtual void Error(const char* szFmt, ...) const = 0;
		virtual void LogLastError(const char* file, int line, const char* function) const = 0;

		virtual IVoxelizer* CreateVoxelizer() = 0;
		virtual void DeleteVoxelizer(IVoxelizer* voxelizer) = 0;

		virtual IUIObject* CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize) = 0;
		virtual void DeleteUIObject(IUIObject* uiObject) = 0;

		virtual ISkySphere* CreateSkySphere(bool usingSmartPointer) = 0;
		virtual void DeleteSkySphere(ISkySphere* skySphere) = 0;

		virtual IBillboardQuad* CreateBillboardQuad() = 0;
		virtual void DeleteBillboardQuad(IBillboardQuad* quad) = 0;

		virtual IDustRenderer* CreateDustRenderer() = 0;
		virtual void DeleteDustRenderer(IDustRenderer* dust) = 0;

		virtual TextManipulator* CreateTextManipulator() = 0;
		virtual void DeleteTextManipulator(TextManipulator* mani) = 0;

		virtual void SetInputOverride(const LuaObject& func) = 0;

		virtual void StopFileChangeMonitor(const char* filepath) = 0;
		virtual void ResumeFileChangeMonitor(const char* filepath) = 0;
		
		virtual IVideoPlayer* CreateVideoPlayer(VideoPlayerType::Enum type) = 0;
		virtual void ReleaseVideoPlayer(IVideoPlayer* player) = 0;

		virtual void ChangeSize(HWND_ID id, const Vec2I& size) = 0;
	};

	typedef SmartPtr<IEngine> EnginePtr;
};

#endif