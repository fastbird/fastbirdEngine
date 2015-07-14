#pragma once
#ifndef _Engine_header_included_
#define _Engine_header_included_

#include <Engine/IEngine.h>
#include <CommonLib/VectorMap.h>

namespace fastbird
{
	class Renderer;
	class IConsole;
	class ITerrain;
	class IInputListener;
	class IMouse;
	class IKeyboard;
    class IScene;
	class INIReader;
	class ISkyBox;
	class IScriptSystem;
	class ParticleManager;
	class ITrailObject;
	class Engine : public IEngine
	{
		VectorMap<HWND_ID, HWND> mWindowHandles;
		VectorMap<HWND, HWND_ID> mWindowHandleIds;
		VectorMap<HWND, Vec2I> mRequestedWndSize;
		HWND_ID FindEmptyHwndId() const;

		SmartPtr<IMouse> mMouse;
		SmartPtr<IKeyboard> mKeyboard;
		typedef std::vector<IInputListener*> INPUT_LISTENER_VECTOR;
		INPUT_LISTENER_VECTOR mInputListeners;

		SmartPtr<IConsole> mConsole;
		SmartPtr<Renderer> mRenderer;
		
		SmartPtr<ITerrain> mTerrain;
		SmartPtr<ISkyBox> mSkyBox;	
		
		INIReader* mINI;
		SmartPtr<IScriptSystem> mScriptSystem;	

		std::string mShaderWatchDir;
		HANDLE mFileMonitorThread;
		HANDLE mFileChangeThreadFinished;
		HANDLE mExitFileChangeThread;
		OVERLAPPED	mOverlapped;
		std::vector<BYTE> mFileChangeBuffer;
		HANDLE mMonitoringDirectory;
		std::ofstream mErrorStream;
		std::streambuf* mStdErrorStream;

		std::map<std::string, SmartPtr<IMeshObject> > mMeshObjects;
		std::map<std::string, SmartPtr<IMeshGroup> > mMeshGroups;
		std::map<std::string, std::vector< SmartPtr<IMeshObject> >  > mFractureObjects;

		std::set<std::string> mChangedFiles;
		float mLastChangedTime;
		std::vector<IFileChangeListener*> mFileChangeListeners;	
		LuaObject mInputOverride;
		bool mExiting;
		
		std::set<std::string> mIgnoreFileChanges;
	

	public:

		Engine();
		virtual ~Engine();

	protected:
		virtual void FinishSmartPtr();


	public:

		virtual GlobalEnv* GetGlobalEnv() const;

		virtual HWND_ID CreateEngineWindow(int x, int y, int width, int height,
			const char* wndClass, const char* title, unsigned style, unsigned exStyle, 
			WNDPROC winProc);
		virtual void DestroyEngineWindow(HWND_ID hwndId);
		virtual const Vec2I& GetRequestedWndSize(HWND hWnd) const;
		virtual const Vec2I& GetRequestedWndSize(HWND_ID hWndId) const;
		virtual HWND GetWindowHandle(HWND_ID id) const;
		virtual HWND_ID GetWindowHandleId(HWND hWnd) const;
		virtual HWND_ID GetWindowHandleIdWithMousePoint() const;
		virtual HWND GetMainWndHandle() const;
		virtual HWND_ID GetMainWndHandleId() const;
		virtual HWND GetForegroundWindow(HWND_ID* id = 0) const;
		virtual HWND_ID GetForegroundWindowId() const;
		virtual bool IsMainWindowForground() const;
		virtual bool InitEngine(int rendererType);
		virtual bool InitSwapChain(HWND_ID id, int width, int height);
		virtual inline IRenderer* GetRenderer() const;
		virtual void UpdateInput();
		virtual void UpdateFrame(float dt);

		virtual bool CreateTerrain(int numVertX, int numVertY, float distance, const char* heightmapFile = 0);
		virtual bool CreateSkyBox();	

		//---------------------------------------------------------------------
		virtual IMeshObject* GetMeshObject(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());
		virtual IMeshObject* CreateMeshObject();
		virtual IMeshGroup* GetMeshGroup(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());

		virtual void GetFractureMeshObjects(const char* daeFilePath, std::vector<IMeshObject*>& objects, bool reload = false);

		virtual const IMeshObject* GetMeshArchetype(const std::string& name) const;
		virtual void ReleaseMeshObject(IMeshObject* p);
		virtual void ReleaseMeshGroup(IMeshGroup* p);
		virtual IParticleEmitter* GetParticleEmitter(const char* file, bool useSmartPtr);
		virtual IParticleEmitter* GetParticleEmitter(unsigned id, bool useSmartPtr);
		virtual void ReleaseParticleEmitter(IParticleEmitter* p);

		virtual ITrailObject* CreateTrailObject();
		virtual void ReleaseTrailObject(ITrailObject* trail);


		virtual void GetMousePos(long& x, long& y);
		virtual bool IsMouseLButtonDown() const;
		virtual IKeyboard* GetKeyboard() const { return mKeyboard; }
		virtual IMouse* GetMouse() const { return mMouse; }

		// priority : lower value processed first.
		virtual void AddInputListener(IInputListener* pInputListener, 
			IInputListener::INPUT_LISTEN_CATEGORY category, int priority);
		virtual void RemoveInputListener(IInputListener* pInputListener);	

		virtual std::string GetConfigStringValue(const char* section, const char* name);
		virtual int GetConfigIntValue(const char* section, const char* name);
		virtual bool GetConfigBoolValue(const char* section, const char* name);

		virtual void DrawProfileResult(ProfilerSimple& p, const char* posVarName, int tab = 0);
		virtual void DrawProfileResult(wchar_t* buf, const char* posVarName, int tab = 0);

	protected:
		bool InitDirectX9();
		bool InitDirectX11(int threadPool);
		bool InitOpenGL();
		
		bool RegisterMouseAndKeyboard(HWND hWnd);
		void Render(float dt);

		void HandleUserInput();

		void FileChangeMonitorThread();
		bool MonitorFileChange();
		void ProcessFileChange();
		void CleanFileChangeMonitor();
		void HotReloading();
		
		static DWORD CALLBACK FileChangeMonitorProc(LPVOID handle);
		HANDLE GetMonitoringHandle() const { return mMonitoringDirectory; }
		static const DWORD FILE_CHANGE_BUFFER_SIZE;

		virtual void RegisterFileChangeListener(IFileChangeListener* listener);
		virtual void RemoveFileChangeListener(IFileChangeListener* listener);

		virtual IScene* CreateScene();
		virtual void DeleteScene(IScene* p);

#ifdef _FBENGINE_FOR_WINDOWS_
		virtual LRESULT WinProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);
#else
#endif

		virtual void Log(const char* szFmt, ...) const;
		virtual void Error(const char* szFmt, ...) const;
		virtual void LogLastError(const char* file, int line, const char* function) const;


		virtual IVoxelizer* CreateVoxelizer();
		virtual void DeleteVoxelizer(IVoxelizer* voxelizer);

		virtual IUIObject* CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize);
		virtual void DeleteUIObject(IUIObject* uiObject);

		virtual ISkySphere* CreateSkySphere(bool usingSmartPointer);
		virtual void DeleteSkySphere(ISkySphere* skySphere);

		virtual IBillboardQuad* CreateBillboardQuad();
		virtual void DeleteBillboardQuad(IBillboardQuad* quad);
	
		virtual IDustRenderer* CreateDustRenderer();
		virtual void DeleteDustRenderer(IDustRenderer* dust);

		virtual TextManipulator* CreateTextManipulator();
		virtual void DeleteTextManipulator(TextManipulator* mani);
		
		virtual void SetInputOverride(const LuaObject& func);

		virtual void StopFileChangeMonitor(const char* filepath);
		virtual void ResumeFileChangeMonitor(const char* filepath);

		virtual IVideoPlayer* CreateVideoPlayer(VideoPlayerType::Enum type);
		virtual void ReleaseVideoPlayer(IVideoPlayer* player);
	};
};

#endif //_Engine_header_included_