#pragma once
#ifndef _Engine_header_included_
#define _Engine_header_included_

#include <Engine/IEngine.h>

namespace fastbird
{
	class IConsole;
	class IRenderer;
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
		virtual void SetSceneOverride(IScene* pScene) { mSceneOverride = pScene; }

		virtual void UpdateInput();
		virtual void UpdateFrame(float dt);

		virtual bool CreateTerrain(int numVertX, int numVertY, float distance, const char* heightmapFile = 0);
		virtual bool CreateSkyBox();

		virtual ICamera* CreateAndRegisterCamera(const char* cameraName);
		virtual void RegisterCamera(const char* cameraName, ICamera* pCamera);
		virtual ICamera* GetCamera(int idx);
		virtual ICamera* GetCamera(const char* cameraName);

		//---------------------------------------------------------------------
		virtual IMeshObject* GetMeshObject(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());
		virtual IMeshGroup* GetMeshGroup(const char* daeFilePath, 
			bool reload = false, const MeshImportDesc& desc = MeshImportDesc());
		virtual IParticleEmitter* GetParticleEmitter(const char* file);
		virtual IParticleEmitter* GetParticleEmitter(unsigned id);

		// priority : lower value processed first.
		virtual void AddInputListener(IInputListener* pInputListener, 
			IInputListener::INPUT_LISTEN_CATEGORY category, int priority);
		virtual void RemoveInputListener(IInputListener* pInputListener);

		virtual void RegisterUIs(std::vector<IUIObject*>& uiobj);
		virtual void UnregisterUIs();

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
		void RenderUI();
		void RenderDebugHud();

		void RenderFrameProfiler();
		void HandleUserInput();

		void FileChangeMonitorThread();
		void MonitorFileChange();
		void BackupFileChangeBuffer(size_t size);
		void ProcessFileChange();
		void CleanFileChangeMonitor();
		
		static DWORD CALLBACK FileChangeMonitorProc(LPVOID handle);
		static VOID CALLBACK NotificationCompletion(
			DWORD dwErrorCode,							// completion code
			DWORD dwNumberOfBytesTransfered,			// number of bytes transferred
			LPOVERLAPPED lpOverlapped);					// I/O information buffer

		HANDLE GetMonitoringHandle() const { return mMonitoringDirectory; }
		
		static VOID CALLBACK TerminateFileMonitor(ULONG_PTR Parameter);

		static const DWORD FILE_CHANGE_BUFFER_SIZE;

	private:
		HWND m_hWnd;
		SmartPtr<IConsole> mConsole;
		SmartPtr<IRenderer> mRenderer;
		SmartPtr<ITerrain> mTerrain;
		SmartPtr<ISkyBox> mSkyBox;
		typedef std::vector<ICamera*> CAMERA_VECTOR;
		CAMERA_VECTOR m_pCameras;
		ICamera* mEngineCamera;
		ICamera* mCurrentCamera;
		typedef std::vector<IInputListener*> INPUT_LISTENER_VECTOR;
		INPUT_LISTENER_VECTOR mInputListeners;
		SmartPtr<IMouse> mMouse;
		SmartPtr<IKeyboard> mKeyboard;
        SmartPtr<IScene> mScene;
		IScene* mSceneOverride; // life time should be managed manually.
		SmartPtr<INIReader> mINI;
		SmartPtr<IScriptSystem> mScriptSystem;

		typedef std::vector<IUIObject*> UI_OBJECTS;
		UI_OBJECTS mUIObjectsToRender;

		std::string mShaderWatchDir;
		HANDLE mFileMonitorThread;
		HANDLE mFileChangeThreadFinished;
		OVERLAPPED	mOverlapped;
		std::vector<BYTE> mFileChangeBuffer;
		std::vector<BYTE> mFileChangeBackupBuffer;
		FB_CRITICAL_SECTION mFileChangeBufferCS;
		HANDLE mMonitoringDirectory;
		std::ofstream mErrorStream;
		std::streambuf* mStdErrorStream; 

		std::map<std::string, SmartPtr<IMeshObject> > mMeshObjects;
		std::map<std::string, SmartPtr<IMeshGroup> > mMeshGroups;

		bool mExiting;

		
	};
};

#endif //_Engine_header_included_