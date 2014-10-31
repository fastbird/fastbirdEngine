#pragma once
#ifndef _Engine_header_included_
#define _Engine_header_included_

#include <Engine/IEngine.h>

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

	class Engine : public IEngine
	{
	public:
		Engine();

	protected:
		virtual ~Engine();

	public:
		virtual void GetGlobalEnv(GlobalEnv** outGloblEnv);

		virtual HWND CreateEngineWindow(int x, int y, int width, int height, 
			const char* title, WNDPROC winProc);
		virtual HWND GetWindowHandle() const;
		virtual bool InitEngine(int rendererType);
		virtual int InitSwapChain(HWND handle, int width, int height);

		virtual inline IRenderer* GetRenderer() const;
		virtual inline IScene* GetScene() const;
		virtual inline IScene* GetOriginalScene() const;
		virtual void SetSceneOverride(IScene* pScene) { mSceneOverride = pScene; }

		virtual void UpdateInput();
		virtual void UpdateFrame(float dt);

		virtual bool CreateTerrain(int numVertX, int numVertY, float distance, const char* heightmapFile = 0);
		virtual bool CreateSkyBox();
		

		virtual size_t CreateCameraAndRegister(const char* cameraName);
		virtual size_t RegisterCamera(const char* cameraName, ICamera* pCamera);
		virtual bool SetActiveCamera(size_t idx);
		virtual ICamera* GetCamera(size_t idx);
		virtual ICamera* GetCamera(const std::string& cameraName);
		

		//---------------------------------------------------------------------
		virtual IMeshObject* GetMeshObject(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());
		virtual IMeshGroup* GetMeshGroup(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());

		virtual const IMeshObject* GetMeshArchetype(const std::string& name) const;
		virtual void ReleaseMeshObject(IMeshObject* p);
		virtual void ReleaseMeshGroup(IMeshGroup* p);
		virtual IParticleEmitter* GetParticleEmitter(const char* file, bool useSmartPtr);
		virtual IParticleEmitter* GetParticleEmitter(unsigned id, bool useSmartPtr);
		virtual void ReleaseParticleEmitter(IParticleEmitter* p);

		virtual void GetMousePos(long& x, long& y);
		virtual bool IsMouseLButtonDown() const;
		virtual IKeyboard* GetKeyboard() const { return mKeyboard; }
		virtual IMouse* GetMouse() const { return mMouse; }

		// priority : lower value processed first.
		virtual void AddInputListener(IInputListener* pInputListener, 
			IInputListener::INPUT_LISTEN_CATEGORY category, int priority);
		virtual void RemoveInputListener(IInputListener* pInputListener);

		virtual void RegisterUIs(std::vector<IUIObject*>& uiobj);
		virtual void UnregisterUIs();

		virtual void AddMarkObject(IObject* mark);
		virtual void RemoveMarkObject(IObject* mark);

		virtual std::string GetConfigStringValue(const char* section, const char* name);
		virtual int GetConfigIntValue(const char* section, const char* name);
		virtual bool GetConfigBoolValue(const char* section, const char* name);

#ifdef _FBENGINE_FOR_WINDOWS_
		static LRESULT WinProc( HWND window, UINT msg, WPARAM wp, LPARAM lp );
#elif _FBENGINE_FOR_LINUX_

#endif
		

	protected:
		bool InitDirectX9();
		bool InitDirectX11(int threadPool);
		bool InitOpenGL();
		bool RegisterMouseAndKeyboard();

		void PreRender(float dt);
		void Render(float dt);
		void RenderMarks();
		void RenderUI();
		void RenderDebugHud();

		void RenderFrameProfiler();
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

	private:
		HWND m_hWnd;
		SmartPtr<IConsole> mConsole;
		SmartPtr<Renderer> mRenderer;
		SmartPtr<ITerrain> mTerrain;
		SmartPtr<ISkyBox> mSkyBox;
		typedef std::vector<SmartPtr<ICamera>> CAMERA_VECTOR;
		CAMERA_VECTOR mCameras;
		int mEngineCamIdx;
		ICamera* mEngineCamera;
		ICamera* mCurrentCamera;
		typedef std::vector<IInputListener*> INPUT_LISTENER_VECTOR;
		INPUT_LISTENER_VECTOR mInputListeners;
		SmartPtr<IMouse> mMouse;
		SmartPtr<IKeyboard> mKeyboard;
        SmartPtr<IScene> mScene;
		IScene* mSceneOverride; // life time should be managed manually.
		INIReader* mINI;
		SmartPtr<IScriptSystem> mScriptSystem;

		typedef std::vector<IUIObject*> UI_OBJECTS;
		UI_OBJECTS mUIObjectsToRender;

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

		std::set<std::string> mChangedFiles;
		float mLastChangedTime;
		std::vector<IFileChangeListener*> mFileChangeListeners;

		std::vector<IObject*> mMarkObjects;

		bool mExiting;

		
	};
};

#endif //_Engine_header_included_