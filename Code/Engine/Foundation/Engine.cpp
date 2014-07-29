#include <Engine/StdAfx.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/ITerrain.h>
#include <Engine/ISkyBox.h>
#include <Engine/IConsole.h>
#include <Engine/IScriptSystem.h>
#include <Engine/IMeshObject.h>
#include <Engine/IMeshGroup.h>
#include <Engine/IParticleEmitter.h>
#include <Engine/IColladaImporter.h>
#include <Engine/Foundation/Mouse.h>
#include <Engine/Foundation/Keyboard.h>
#include <Engine/Foundation/Engine.h>
#include <Engine/Renderer/Camera.h>
#include <Engine/Renderer/Shader.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/SceneGraph/Scene.h>
#include <CommonLib/Math/Vec2I.h>
#include <Engine/RenderObjects/UIObject.h>
#include <CommonLib/INIReader.h>
#include <CommonLib/StringUtils.h>
#include <Engine/ScriptSystem/ScriptSystem.h>
#include <UI/IWinBase.h>
#include <FreeImage.h>
#include <libxml/parser.h>


namespace fastbird
{
	//------------------------------------------------------------------------
	IEngine* IEngine::CreateInstance()
	{
		return new Engine();
	}

	//------------------------------------------------------------------------
	Engine::Engine()
		: mSceneOverride(0)
	{
		mErrorStream.open("error.log");
		mStdErrorStream = std::cerr.rdbuf(mErrorStream.rdbuf());
		char timestring[256];
		GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, 0, 0, timestring, 256);
		std::cerr << "Engine Initialized at " << timestring << std::endl;
		gFBEnv = new GlobalEnv;
		gFBEnv->pEngine = this;
		gFBEnv->pTimer = new Timer;
		gpTimer = gFBEnv->pTimer;
		mEngineCamera = 0;
        mScene = new Scene();

		m_hWnd = 0;
		mFileChangeThreadFinished = 0;
		mExiting = false;
		mFileMonitorThread = 0;
		mMonitoringDirectory = INVALID_HANDLE_VALUE;

		mConsole = IConsole::CreateConsole();
		ParticleManager::InitializeParticleManager();
	}

	//------------------------------------------------------------------------
	Engine::~Engine()
	{
		mExiting = true;

		ParticleManager::FinalizeParticleManager();
		if (mFileMonitorThread)
		{
			QueueUserAPC((PAPCFUNC)TerminateFileMonitor, mFileMonitorThread, 0);
			DWORD ret = WaitForSingleObject(mFileChangeThreadFinished, 1000);
			if (ret==WAIT_FAILED)
			{
				FB_LOG_LAST_ERROR();
			}
			CloseHandle(mFileChangeThreadFinished);
		}

		mTerrain=0;
		mSkyBox=0;

		CAMERA_VECTOR::iterator it = m_pCameras.begin(), itEnd = m_pCameras.end();
		for (; it!=itEnd; it++)
		{
			delete *it;
		}
		SAFE_DELETE(gFBEnv->pTimer);
		gFBEnv->pEngine=0;		
		SAFE_DELETE(gFBEnv);
		FreeImage_DeInitialise();
		std::cerr.rdbuf(mStdErrorStream);
		mErrorStream.close();
		xmlCleanupParser();

		mScene = 0;
		mSkyBox = 0;
		mTerrain = 0;
		mConsole = 0;
		mMeshObjects.clear();
		mMeshGroups.clear();
		mRenderer = 0;
	}

	//------------------------------------------------------------------------
	void Engine::GetGlobalEnv(GlobalEnv** outGlobalEnv)
	{
		*outGlobalEnv = gFBEnv;
	}

	//------------------------------------------------------------------------
	HWND Engine::CreateEngineWindow(int x, int y, int width, int height, 
		const char* title, WNDPROC winProc)
	{
		width+=16;
		height+=38;
		const char* myclass = "FBEngineClass" ;
		WNDCLASSEX wndclass = { sizeof(WNDCLASSEX), CS_DBLCLKS, winProc,
								0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION),
								LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW+1),
								0, myclass, LoadIcon(0,IDI_APPLICATION) } ;

		if( RegisterClassEx(&wndclass) )
		{
			m_hWnd = CreateWindowEx( 0, myclass, title,
					   WS_OVERLAPPEDWINDOW, x, y,
					   width, height, 0, 0, GetModuleHandle(0), 0 ) ;

			/*HWND console = GetConsoleWindow();
			RECT consoleRect;
			GetWindowRect(console, &consoleRect);
			MoveWindow(console, x+width, y, consoleRect.right-consoleRect.left, 
				consoleRect.bottom-consoleRect.top, TRUE);*/

			ShowWindow(m_hWnd, TRUE);

			return m_hWnd;
		}

		return 0;
	}

	//------------------------------------------------------------------------
	HWND Engine::GetWindowHandle() const
	{
		return m_hWnd;
	}

	//------------------------------------------------------------------------
	bool Engine::InitEngine(int rendererType)
	{
		mINI = new INIReader("Engine.ini");
		if (mINI->GetError())
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Could not parse the Engine.ini file!");
			assert(0);
		}
		int threadPool = mINI->GetInteger("Render", "ThreadPool", 0);

		mScriptSystem = new ScriptSystem();
		gFBEnv->pScriptSystem = mScriptSystem;

		FreeImage_Initialise();
		bool successful = true;
		
		switch((RENDERER_TYPE)rendererType)
		{
		case D3D9:
			successful = InitDirectX9();
			break;

		case D3D11:
			successful = InitDirectX11(threadPool);
			break;

		case OPENGL:
			successful = InitOpenGL();
			break;

		default:
			Log(FB_DEFAULT_DEBUG_ARG, "Not supported rendererType!");
			Log("  rendererType = %d", rendererType);
			return false;
		}
		gFBEnv->pRenderer = GetRenderer();
		mEngineCamera = CreateAndRegisterCamera("Engine_Camera");
		mCurrentCamera = mEngineCamera;
		if (mRenderer)
			mRenderer->SetCamera(mEngineCamera);
		RegisterMouseAndKeyboard();

		//mShaderWatchDir = "code/engine/renderer/shaders";
		mShaderWatchDir = ".";

		mFileMonitorThread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)FileChangeMonitorProc, 0, 0, 0) ;
		if (mFileMonitorThread == INVALID_HANDLE_VALUE)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Failed to create the FileChangeNotifier thread!");
		}

		return successful;
	}
	//width height are set here. need initialize fone and debug hud after it.
	//-------------------------------------------------------------------------
	int Engine::InitSwapChain(HWND handle, int width, int height)
	{
		int index = -1;
		if (mRenderer)
		{
			index = mRenderer->InitSwapChain(handle, width, height);
		}
		else
		{
			Error("No Renderer while init swap chain.");
		}
		return index;
	}

	//-------------------------------------------------------------------------
	inline IRenderer* Engine::GetRenderer() const
	{
		return mRenderer;
	}

	//-------------------------------------------------------------------------
	inline IScene* Engine::GetScene() const
	{
		return mSceneOverride ? mSceneOverride : mScene;
	}

	//-------------------------------------------------------------------------
	bool Engine::InitDirectX9()
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Initializing DirectX9...");

		Log(FB_DEFAULT_DEBUG_ARG, "DirectX9 Initialized.");
		return true;
	}

	//-------------------------------------------------------------------------
	bool Engine::InitDirectX11(int threadPool)
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Initializing DirectX11...");

		mRenderer = IRenderer::CreateD3D11Instance();
		if (!mRenderer)
			return false;

		bool success = mRenderer->Init(0);

		Log(FB_DEFAULT_DEBUG_ARG, "DirectX11 Initialized.");

		return success;
	}

	bool Engine::InitOpenGL()
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Initializing OpenGL...");

		Log(FB_DEFAULT_DEBUG_ARG, "OpenGL initialized...");

		return true;
	}

	bool Engine::RegisterMouseAndKeyboard()
	{
		mMouse = new Mouse;
		mKeyboard = new Keyboard;
#ifdef _FBENGINE_FOR_WINDOWS_
		const unsigned short HID_USAGE_PAGE_GENERIC = 0x01;
		const unsigned short HID_USAGE_GENERIC_MOUSE = 0x02;
		const unsigned short HID_USAGE_GENERIC_KEYBOARD = 0x06;
		
		RAWINPUTDEVICE Rid[2];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
		Rid[0].dwFlags = 0;   
		Rid[0].hwndTarget = m_hWnd;
		Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC; 
		Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD; 
		Rid[1].dwFlags = 0;   
		Rid[1].hwndTarget = m_hWnd;

		HRESULT hr = RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));
		if (FAILED(hr))
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Registering Raw input devices failed!");
			return false;
		}
		
		return true;
#elif _FBENGINE_FOR_LINUX_

#endif
	}

	void Engine::UpdateInput()
	{
		// Send User Input
		HandleUserInput();
	}

	void Engine::UpdateFrame(float dt)
	{
		if (mTerrain)
			mTerrain->Update();

		// Update physics
	
		// Update Entities

		// Update AI

		// Simulations

		// Update Particles
		ParticleManager::GetParticleManager().Update(dt);

		// Render
		Render(dt);

		if (mMouse) mMouse->EndFrame();
		if (mKeyboard) mKeyboard->EndFrame();
		gFBEnv->mFrameCounter++;
	}

	void Engine::HandleUserInput()
	{
		if (mKeyboard)
		{
			if (mKeyboard->IsKeyPressed('W') && mKeyboard->IsKeyDown(VK_CONTROL))
			{
				bool wire = GetRenderer()->GetWireframe();
				GetRenderer()->SetWireframe(!wire);
			}

			if (mKeyboard->IsKeyPressed('S') && mKeyboard->IsKeyDown(VK_CONTROL))
			{
				GetScene()->ToggleSkyRendering();
			}

			if (mKeyboard->IsKeyPressed(VK_OEM_3)) // `
			{
				if (mConsole)
				{
					mConsole->ToggleOpen();
				}
			}
		}
		

		INPUT_LISTENER_VECTOR::iterator it = mInputListeners.begin(),
			itEnd = mInputListeners.end();
		for (; it!=itEnd; it++)
		{
			if ((*it)->IsEnabledInputLIstener())
				(*it)->OnInput(mMouse.get(), mKeyboard.get());
		}
	}

	void Engine::PreRender(float dt)
	{
		GetScene()->PreRender();
	}

	void Engine::Render(float dt)
	{
		if (!mRenderer)
			return;

		static OBJECT_CONSTANTS objectConstants;
		objectConstants.gWorld.MakeIdentity();
		objectConstants.gWorldViewProj = mRenderer->GetCamera()->GetViewProjMat();
		mRenderer->UpdateObjectConstantsBuffer(&objectConstants);
		mRenderer->InitFrameProfiler(dt);
		// Handle RenderTargets
		mRenderer->ProcessRenderToTexture();

		if (mRenderer)
			mRenderer->SetCamera(mEngineCamera);
		// Render Scene
		mRenderer->Clear();
		mRenderer->UpdateFrameConstantsBuffer();
		// PreRender Everything
		PreRender(dt);	
		// Render Scene
		GetScene()->Render();

		// RenderOthers
		RenderUI();
		RenderDebugHud();

		if (mConsole)
			mConsole->Render();

		// Render Profiler
		if (mConsole->GetEngineCommand()->e_profile)
			RenderFrameProfiler();

		mRenderer->Present();
	}

	//---------------------------------------------------------------------------
	bool Engine::CreateTerrain(int numVertX, int numVertY, float distance, 
		const char* heightmapFile)
	{
		mTerrain = ITerrain::CreateTerrainInstance(numVertX, numVertY, distance, heightmapFile);
		return true;
	}

	//---------------------------------------------------------------------------
	bool Engine::CreateSkyBox()
	{
		mSkyBox = ISkyBox::CreateSkyBoxInstance();
		mSkyBox->Init();
		GetScene()->AttachSkyBox(mSkyBox);
		return true;	
	}

	//---------------------------------------------------------------------------
	ICamera* Engine::CreateAndRegisterCamera(const char* cameraName)
	{
		ICamera* pCamera = new Camera();
		RegisterCamera(cameraName, pCamera);
		return pCamera;
	}

	//---------------------------------------------------------------------------
	void Engine::RegisterCamera(const char* cameraName, ICamera* pCamera)
	{
		pCamera->SetName(cameraName);
		m_pCameras.push_back(pCamera);
	}

	//---------------------------------------------------------------------------
	ICamera* Engine::GetCamera(int idx)
	{
		if ((size_t)idx < m_pCameras.size())
			return m_pCameras[idx];

		assert(0 && "Wrong camera index!");
		return 0;
	}

	//---------------------------------------------------------------------------
	ICamera* Engine::GetCamera(const char* cameraName)
	{
		auto it = m_pCameras.begin(), itEnd = m_pCameras.end();
		for (; it!=itEnd; it++)
		{
			if (stricmp((*it)->GetName(), cameraName)==0)
			{
				return *it;
			}
		}
		assert(0 && "Wrong camera name!");
		return 0;
	}

	//---------------------------------------------------------------------------
	IMeshObject* Engine::GetMeshObject(const char* daeFilePath, 
			bool reload, const MeshImportDesc& desc)
	{
		std::string filepath = daeFilePath;
		ToLowerCase(filepath);
		if (!reload)
		{
			auto it = mMeshObjects.find(filepath);
			if (it!=mMeshObjects.end())
				return (IMeshObject*)it->second->Clone();
		}
		SmartPtr<IColladaImporter> pColladaImporter = IColladaImporter::CreateColladaImporter();
		pColladaImporter->ImportCollada(daeFilePath, desc.yzSwap, desc.oppositeCull, desc.useIndexBuffer, 
			desc.mergeMaterialGroups, desc.keepMeshData, desc.generateTangent, false);
		IMeshObject* pMeshObject = pColladaImporter->GetMeshObject();
		assert(pMeshObject);
		mMeshObjects[filepath] = pMeshObject;
		return (IMeshObject*)pMeshObject->Clone();
	}

	//---------------------------------------------------------------------------
	IMeshGroup* Engine::GetMeshGroup(const char* daeFilePath, 
			bool reload, const MeshImportDesc& desc)
	{
		std::string filepath = daeFilePath;
		ToLowerCase(filepath);
		if (!reload)
		{
			auto it = mMeshGroups.find(filepath);
			if (it!=mMeshGroups.end())
				return (IMeshGroup*)it->second->Clone();
		}
		SmartPtr<IColladaImporter> pColladaImporter = IColladaImporter::CreateColladaImporter();
		pColladaImporter->ImportCollada(daeFilePath, desc.yzSwap, desc.oppositeCull, desc.useIndexBuffer, 
			desc.mergeMaterialGroups, desc.keepMeshData, desc.generateTangent, true);
		IMeshGroup* pMeshGroup = pColladaImporter->GetMeshGroup();
		assert(pMeshGroup);
		mMeshGroups[filepath] = pMeshGroup;
		return (IMeshGroup*)pMeshGroup->Clone();
	}

	IParticleEmitter* Engine::GetParticleEmitter(const char* file)
	{
		return ParticleManager::GetParticleManager().GetParticleEmitter(file);		
	}

	IParticleEmitter* Engine::GetParticleEmitter(unsigned id)
	{
		return ParticleManager::GetParticleManager().GetParticleEmitter(id);
	}

	//---------------------------------------------------------------------------
	struct InputListenerSorter
	{
		bool operator() (IInputListener* a, IInputListener*b)
		{
			if (a->mFBInputListenerCategory < b->mFBInputListenerCategory)
			{
				return true;
			}
			else if (a->mFBInputListenerCategory == b->mFBInputListenerCategory)
			{
				if (a->mFBInputListenerPriority > b->mFBInputListenerPriority)
				{
					return true;
				}
				else if (a->mFBInputListenerPriority == b->mFBInputListenerPriority)
				{
					IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Input listeners has the same priority. sorted by address.");
					return a<b;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	};

	//--------------------------------------------------------------------------
	// priority : lower value processed first.
	void Engine::AddInputListener(IInputListener* pInputListener, 
			IInputListener::INPUT_LISTEN_CATEGORY category, int priority)
	{
		pInputListener->mFBInputListenerCategory = category;
		pInputListener->mFBInputListenerPriority = priority;

		mInputListeners.push_back(pInputListener);
		InputListenerSorter sorter;
		std::sort(mInputListeners.begin(), mInputListeners.end(), sorter);

	}

	//--------------------------------------------------------------------------
	void Engine::RemoveInputListener(IInputListener* pInputListener)
	{
		INPUT_LISTENER_VECTOR::iterator it = mInputListeners.begin(),
			itEnd = mInputListeners.end();
		for (; it!=itEnd; it++)
		{
			if ((*it) == pInputListener)
			{
				mInputListeners.erase(it);
				return;
			}
		}
	}

	//--------------------------------------------------------------------------
	void Engine::RegisterUIs(std::vector<IUIObject*>& uiobj)
	{
		mUIObjectsToRender.swap(uiobj);		
	}

	void Engine::UnregisterUIs()
	{
		mUIObjectsToRender.clear();
	}

	//--------------------------------------------------------------------------
#ifdef _FBENGINE_FOR_WINDOWS_
	LRESULT IEngine::WinProc( HWND window, UINT msg, WPARAM wp, LPARAM lp )
	{
		if (gFBEnv && gFBEnv->pEngine)
			return Engine::WinProc(window, msg, wp, lp);
		else
			return DefWindowProc(window, msg, wp, lp); // not processed
	}

	LRESULT Engine::WinProc( HWND window, UINT msg, WPARAM wp, LPARAM lp )
	{
		Engine* pEngine = (Engine*)gFBEnv->pEngine;
		switch(msg)
		{

		case WM_INPUT:
			{
				UINT dwSize = 40;
				static BYTE lpb[40];

				GetRawInputData((HRAWINPUT)lp, RID_INPUT, 
								lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
				RAWINPUT* raw = (RAWINPUT*)lpb;
    
				switch(raw->header.dwType)
				{
				case RIM_TYPEMOUSE:
					{
						pEngine->mMouse->PushEvent( *((MouseEvent*)&raw->data.mouse));
					}
					return 0;
				case RIM_TYPEKEYBOARD:
					{
						pEngine->mKeyboard->PushEvent(*((KeyboardEvent*)&raw->data.keyboard));
					}
					return 0;
				}
			}
			break;

		case WM_CHAR:
			{
				if (pEngine->mKeyboard)
				{
					pEngine->mKeyboard->PushChar(wp);
				}
			}
			return 0; // processed

		case WM_SETFOCUS:
			{
				if (pEngine->mMouse)
					pEngine->mMouse->OnSetFocus();
			}
			return 0;

		case WM_KILLFOCUS :
			{
				if (pEngine->mKeyboard)
					pEngine->mKeyboard->OnKillFocus();
				if (pEngine->mMouse)
					pEngine->mMouse->OnKillFocus();
			}
			return 0;

	/*	case WM_ACTIVATE:
			{
				if (WA_ACTIVE==LOWORD(wp))
				{
					BringWindowToTop(m_hWnd);
				}
			}
			return 0; // processed
			*/
		}


		return DefWindowProc(window, msg, wp, lp); // not processed
	}
#elif _FBENGINE_FOR_LINUX_

#endif

	//----------------------------------------------------------------------------
	void Engine::RenderUI()
	{
		UI_OBJECTS::reverse_iterator it = mUIObjectsToRender.rbegin(), itEnd = mUIObjectsToRender.rend();
		for (; it!=itEnd; it++)
		{
			(*it)->PreRender(); // temporary :)
			(*it)->Render();
			(*it)->PostRender();
		}		
	}
	
	void Engine::RenderDebugHud()
	{
		mRenderer->RenderDebugHud();
	}

	//----------------------------------------------------------------------------
	void Engine::RenderFrameProfiler()
	{
		wchar_t msg[255];
		int x = 1100;
		int y=20;
		int yStep = 16;
		const RENDERER_FRAME_PROFILER& profiler = mRenderer->GetFrameProfiler();

		swprintf_s(msg, 255, L"FrameRate = %.0f", profiler.FrameRateDisplay); 
		mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y+=yStep;

		swprintf_s(msg, 255, L"Num draw calls = %d", profiler.NumDrawCall); 
		mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y+=yStep;

		swprintf_s(msg, 255, L"Num vertices = %d", profiler.NumVertexCount);
		mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y+=yStep*2;

		
		swprintf_s(msg, 255, L"Num draw indexed calls = %d", profiler.NumDrawIndexedCall);
		mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y+= yStep;

		swprintf_s(msg, 255, L"Num draw indices = %d", profiler.NumIndexCount);
		mRenderer->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));		
	}
	const DWORD Engine::FILE_CHANGE_BUFFER_SIZE = 400;
	//----------------------------------------------------------------------------
	void Engine::FileChangeMonitorThread()
	{
		mFileChangeThreadFinished = CreateEvent(0, FALSE, FALSE, "FileChangeThreadFinished");

		mMonitoringDirectory = CreateFile(mShaderWatchDir.c_str(), FILE_LIST_DIRECTORY, 
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
			0, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
		if (mMonitoringDirectory==INVALID_HANDLE_VALUE)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Cannot open the shader watch directory!");
			FB_LOG_LAST_ERROR();
			return;
		}
		mFileChangeBuffer.resize(FILE_CHANGE_BUFFER_SIZE);
		mFileChangeBackupBuffer.resize(FILE_CHANGE_BUFFER_SIZE);
		MonitorFileChange();

		while(!mExiting)
			DWORD rc = ::SleepEx(INFINITE, true);
		
		SetEvent(mFileChangeThreadFinished);
	}

	//----------------------------------------------------------------------------
	void Engine::MonitorFileChange()
	{
		memset(&mOverlapped, 0, sizeof(OVERLAPPED));
		LOCK_CRITICAL_SECTION lock(mFileChangeBufferCS);
		DWORD writtenBytes=0;
		BOOL successful = ReadDirectoryChangesW(mMonitoringDirectory, 
			&mFileChangeBuffer[0], mFileChangeBuffer.size(),
			true, FILE_NOTIFY_CHANGE_LAST_WRITE, &writtenBytes, 
			&mOverlapped, NotificationCompletion);
		if (!successful)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "ReadDirectoryChangesW Failed!");
			DWORD lastError = GetLastError();
			Log("\t error code = %d", lastError);
			return;
		}
	}

	//----------------------------------------------------------------------------
	void Engine::BackupFileChangeBuffer(size_t size)
	{
		LOCK_CRITICAL_SECTION lock(mFileChangeBufferCS);
		assert(size<FILE_CHANGE_BUFFER_SIZE);
		memset(&mFileChangeBackupBuffer[0], 0, mFileChangeBackupBuffer.size());
		memcpy(&mFileChangeBackupBuffer[0], &mFileChangeBuffer[0], size);
		memset(&mFileChangeBuffer[0], 0, mFileChangeBuffer.size());
	}

	//----------------------------------------------------------------------------
	void Engine::ProcessFileChange()
	{
		LOCK_CRITICAL_SECTION lock(mFileChangeBufferCS);
		FILE_NOTIFY_INFORMATION* pFNI = (FILE_NOTIFY_INFORMATION*)&mFileChangeBackupBuffer[0];
		while(pFNI)
		{
			switch(pFNI->Action)
			{
			case FILE_ACTION_MODIFIED:
				{
					char fileName[MAX_PATH];
					int count = WideCharToMultiByte(CP_ACP, 0, pFNI->FileName, pFNI->FileNameLength / sizeof(WCHAR),
						fileName, _ARRAYSIZE(fileName)-1, 0, 0);
					fileName[count]=0;
					char unifiedPath[MAX_PATH] = {0};
					UnifyFilepath(unifiedPath, fileName);
					count = strlen(unifiedPath);
					if (CheckExtension(unifiedPath, "hlsl"))
						IMaterial::ReloadShader(unifiedPath);
					else if (CheckExtension(unifiedPath, "material"))
						IMaterial::ReloadMaterial(unifiedPath);
				}
				break;
			}
			pFNI = pFNI->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)(((char*)pFNI) + pFNI->NextEntryOffset) : 0;
		}
	}
	//----------------------------------------------------------------------------
	void Engine::CleanFileChangeMonitor()
	{
		::CancelIo(mMonitoringDirectory);
		::CloseHandle(mMonitoringDirectory);
		mMonitoringDirectory = 0;
	}

	//----------------------------------------------------------------------------
	DWORD CALLBACK Engine::FileChangeMonitorProc(LPVOID handle)
	{
		Engine* pEngine = (Engine*)gFBEnv->pEngine;
		pEngine->FileChangeMonitorThread();

		return 0;
	}

	//----------------------------------------------------------------------------
	VOID CALLBACK Engine::NotificationCompletion(
			DWORD dwErrorCode,							// completion code
			DWORD dwNumberOfBytesTransfered,			// number of bytes transferred
			LPOVERLAPPED lpOverlapped)					// I/O information buffer
	{
		Engine* pEngine = (Engine*)gFBEnv->pEngine;
		if (dwErrorCode)
		{
			return;
		}
		DWORD dwNumBytes;
		bool success = GetOverlappedResult(pEngine->GetMonitoringHandle(), 
			lpOverlapped, &dwNumBytes, false)!=0;
		if (!success)
			return;
		assert(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));
		if(!dwNumberOfBytesTransfered)
			return;

		pEngine->BackupFileChangeBuffer(dwNumberOfBytesTransfered);
		pEngine->ProcessFileChange();
		pEngine->MonitorFileChange();

	}

	//----------------------------------------------------------------------------
	VOID CALLBACK Engine::TerminateFileMonitor(ULONG_PTR Parameter)
	{
		Engine* pEngine = (Engine*)gFBEnv->pEngine;
		pEngine->CleanFileChangeMonitor();
	}

	//------------------------------------------------------------------------
	void IEngine::Log(const char* szFmt, ...)
	{
		char buf[2048];

		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);
		strcat_s(buf, 2048, "\n");
		std::cout << buf;
		OutputDebugString(buf);

		if (gFBEnv && gFBEnv->pConsole)
			gFBEnv->pConsole->Log(buf);
	}

	//------------------------------------------------------------------------
	void IEngine::Error(const char* szFmt, ...)
	{
		char buf[2048];

		Timer::TIME_PRECISION time = gFBEnv->pTimer->GetTime();
		sprintf_s(buf, "[%.3f] ", time);
		int len = strlen(buf);
		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf+len, 2048-len, szFmt, args);
		va_end(args);
		
		strcat_s(buf, 2048, "\n");

		std::cout << buf;
		std::cerr << buf;
		std::cout.flush();
		std::cerr.flush();
		OutputDebugString(buf);		

		if (gFBEnv && gFBEnv->pConsole)
			gFBEnv->pConsole->Log(buf);
	}

	//------------------------------------------------------------------------
	void IEngine::LogLastError(const char* file, int line, const char* function)
	{
		char buf[2048];

		DWORD err = GetLastError();
		if (err==0)
			return;

		LPVOID lpMsgBuf;

		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf, 0, NULL );

		sprintf_s(buf, 2048, "%s(%d): %s() - %s - %s \n", file, line, function, lpMsgBuf);
		OutputDebugString(buf);


		LocalFree(lpMsgBuf);
	}

	void Error(const char* szFmt, ...)
	{
		char buf[2048];

		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);

		if (gFBEnv && gFBEnv->pEngine)
			gFBEnv->pEngine->Error(buf);
	}

	void Log(const char* szFmt, ...)
	{
		char buf[2048];

		va_list args;
		va_start(args, szFmt);
		vsprintf_s(buf, 2048, szFmt, args);
		va_end(args);

		if (gFBEnv && gFBEnv->pEngine)
			gFBEnv->pEngine->Log(buf);
	}

} // namespace fastbird