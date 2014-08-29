#include <Engine/StdAfx.h>
#include <Engine/GlobalEnv.h>
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
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/Camera.h>
#include <Engine/Renderer/Shader.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/SceneGraph/Scene.h>
#include <Engine/RenderObjects/SkySphere.h>
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
		return FB_NEW(Engine);
	}

	void IEngine::DeleteInstance(IEngine* e)
	{
		FB_DELETE(e);
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
		gFBEnv = FB_NEW(GlobalEnv);
		gFBEnv->pEngine = this;
		gFBEnv->pTimer = FB_NEW(Timer);
		gpTimer = gFBEnv->pTimer;
		mEngineCamera = 0;
		mScene = FB_NEW(Scene);

		m_hWnd = 0;
		mFileChangeThreadFinished = 0;
		mExitFileChangeThread = 0;
		mExiting = false;
		mFileMonitorThread = 0;
		mMonitoringDirectory = INVALID_HANDLE_VALUE;
		mINI = 0;

		mConsole = IConsole::CreateConsole();
		ParticleManager::InitializeParticleManager();

		mExitFileChangeThread = CreateEvent(0, FALSE, FALSE, 0);
	}

	//------------------------------------------------------------------------
	Engine::~Engine()
	{
		mExiting = true;

		ParticleManager::FinalizeParticleManager();
		if (mFileMonitorThread)
		{
			SetEvent(mExitFileChangeThread);
			DWORD ret = WaitForSingleObject(mFileChangeThreadFinished, 10000);
			if (ret==WAIT_FAILED)
			{
				FB_LOG_LAST_ERROR();
			}
			CloseHandle(mFileChangeThreadFinished);
		}

		mTerrain=0;
		mSkyBox=0;

		mCameras.clear();
		FreeImage_DeInitialise();
		std::cerr.rdbuf(mStdErrorStream);
		mErrorStream.close();
		xmlCleanupParser();

		mScene = 0;
		mSkyBox = 0;
		mTerrain = 0;
		mMeshObjects.clear();
		mMeshGroups.clear();
		mRenderer = 0;
		mConsole = 0;
		FB_SAFE_DEL(gFBEnv->pTimer);
		FB_SAFE_DEL(mINI);
		mScriptSystem = 0;
		mMouse = 0;
		mKeyboard = 0;
		gFBEnv->pEngine=0;	
		FB_SAFE_DEL(gFBEnv);
#ifdef USING_FB_MEMORY_MANAGER
		FBReportMemoryForModule();
#endif
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
		mINI = FB_NEW(INIReader)("Engine.ini");
		if (mINI->GetError())
		{
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Could not parse the Engine.ini file!");
			assert(0);
		}
		int threadPool = mINI->GetInteger("Render", "ThreadPool", 0);

		mScriptSystem = FB_NEW(ScriptSystem);
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
		gFBEnv->_pInternalRenderer = (Renderer*)gFBEnv->pRenderer;
		mEngineCamIdx = CreateCameraAndRegister("Engine_Camera");
		mEngineCamera = GetCamera(mEngineCamIdx);
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
	inline IScene* Engine::GetOriginalScene() const
	{
		return mScene;
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

		mRenderer = (Renderer*)IRenderer::CreateD3D11Instance();
		if (!mRenderer)
			return false;

		bool success = mRenderer->Init(threadPool);

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
		mMouse = FB_NEW(Mouse);
		mKeyboard = FB_NEW(Keyboard);
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
		// hot reloading
		HotReloading();

		FB_FOREACH(it, mCameras)
		{
			(*it)->ProcessInputData();
		}

		if (mTerrain)
			mTerrain->Update();

		// Update physics
	
		// Update Entities

		// Update AI

		// Simulations

		// Update Particles
		ParticleManager::GetParticleManager().Update(dt);
		mRenderer->UpdateCloud(dt);
		// Render
		Render(dt);

		if (mMouse) mMouse->EndFrame();
		if (mKeyboard) mKeyboard->EndFrame();
		gFBEnv->mFrameCounter++;

		// shoud be update here
		// to sync with the game.
		// light update
		if (mRenderer)
			mRenderer->Update(dt);
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


		FB_FOREACH(cam, mCameras)
		{
			(*cam)->OnInputFromEngine(mMouse, mKeyboard);
		}
		

		INPUT_LISTENER_VECTOR::iterator it = mInputListeners.begin(),
			itEnd = mInputListeners.end();
		for (; it!=itEnd; it++)
		{
			if ((*it)->IsEnabledInputLIstener())
				(*it)->OnInput(mMouse.get(), mKeyboard.get());
		}
	}

	void Engine::HotReloading()
	{
		if (!mChangedFiles.empty())
		{
			if (gFBEnv->pTimer->GetTime() - mLastChangedTime > 0.5f)
			{
				for (auto it = mChangedFiles.begin(); it != mChangedFiles.end(); )
				{
					bool shader = CheckExtension(it->c_str(), "hlsl");
					bool material = CheckExtension(it->c_str(), "material");
					bool texture = CheckExtension(it->c_str(), "png") || CheckExtension(it->c_str(), "dds");
					bool particle = CheckExtension(it->c_str(), "particle");
					bool canOpen = true;
					std::string filepath = it->c_str();

					FILE* file;
					errno_t err = fopen_s(&file, filepath.c_str(), "a+");
					if (err)
						canOpen = false;
					else
						fclose(file);

					if (canOpen)
					{
						if (shader || material || texture || particle)
						{
							if (shader)
								IShader::ReloadShader(filepath.c_str());
							else if (material)
								IMaterial::ReloadMaterial(filepath.c_str());
							else if (texture)
								ITexture::ReloadTexture(filepath.c_str());
							else if (particle)
								ParticleManager::GetParticleManager().ReloadParticle(filepath.c_str());

							auto nextit = it;
							++nextit;
							mChangedFiles.erase(it);
							it = nextit;
						}
						else
						{
							auto nextit = it;
							++nextit;
							mChangedFiles.erase(it);
							it = nextit;
						}

						FB_FOREACH(listener, mFileChangeListeners)
						{
							(*listener)->OnFileChanged(filepath.c_str());
						}
					}
					else
					{
						++it;
					}					
				}
			}
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

		mRenderer->SetDepthTexture(false);
		mRenderer->SetCloudVolumeTexture(false);

		static OBJECT_CONSTANTS objectConstants;
		objectConstants.gWorld.MakeIdentity();
		objectConstants.gWorldViewProj = mRenderer->GetCamera()->GetViewProjMat();
		mRenderer->UpdateObjectConstantsBuffer(&objectConstants);
		mRenderer->InitFrameProfiler(dt);
		// Handle RenderTargets
		mRenderer->ProcessRenderToTexture();

		if (mRenderer)
			mRenderer->SetCamera(mCurrentCamera);
		// Render Scene
		mRenderer->SetGlowRenderTarget(); // clear glow also
		mRenderer->Clear();
		mRenderer->UnSetGlowRenderTarget();
		mRenderer->Clear();
		
		mRenderer->UpdateFrameConstantsBuffer();
		// PreRender Everything
		PreRender(dt);
		IScene* scene = GetScene();
		// Render Scene
		{
			mRenderer->RestoreBlendState();
			mRenderer->RestoreDepthStencilState();
			// depth pass
			{
				D3DEventMarker mark("Depth pass");				
				mRenderer->SetDepthRenderTarget(true);
				gFBEnv->mRenderPass = RENDER_PASS::PASS_DEPTH;
				scene->Render();
				mRenderer->UnsetDepthRenderTarget();
				mRenderer->SetDepthTexture(true);
			}

			// cloud pass -- also depth pass
			{
				D3DEventMarker mark("Cloud Volumes");
				mRenderer->SetCloudVolumeTarget();
				GetScene()->RenderCloudVolumes();

				mRenderer->RestoreRenderTarget();
				mRenderer->SetCloudVolumeTexture(true);
				mRenderer->RestoreViewports();
				mRenderer->BindNoiseMap();
				gFBEnv->mRenderPass = RENDER_PASS::PASS_NORMAL;
			}


			// god ray pre pass
			if (mConsole->GetEngineCommand()->r_GodRay)
			{
				D3DEventMarker mark("GodRay pre occlusion pass");
				mRenderer->SetGodRayRenderTarget();
				gFBEnv->mRenderPass = RENDER_PASS::PASS_GODRAY_OCC_PRE;
				scene->Render();
				mRenderer->GodRay();
				gFBEnv->mRenderPass = RENDER_PASS::PASS_NORMAL;
			}

			if (mConsole->GetEngineCommand()->r_HDR)
			{
				// Set HDR RenderTarget
				mRenderer->SetHDRTarget();
				mRenderer->SetDepthTexture(true);
				mRenderer->SetCloudVolumeTexture(true);
				mRenderer->Clear();
			}
			// RENDER
			scene->Render();

			// POST_PROCESS
			if (mConsole->GetEngineCommand()->r_Glow)
				mRenderer->BlendGlow();
			if (mConsole->GetEngineCommand()->r_GodRay)
				mRenderer->BlendGodRay();
			if (mConsole->GetEngineCommand()->r_HDR)
			{
				mRenderer->RestoreRenderTarget();
				mRenderer->MeasureLuminanceOfHDRTarget();
				mRenderer->Bloom();
				mRenderer->ToneMapping();
			}
		}

		// RenderOthers
		RenderUI();
		RenderDebugHud();

		if (mConsole)
			mConsole->Render();

		// Render Profiler
		if (mConsole->GetEngineCommand()->e_profile)
			RenderFrameProfiler();

		if (mConsole->GetEngineCommand()->r_particleProfile)
		{
			ParticleManager::GetParticleManager().RenderProfile();
		}

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
	size_t Engine::CreateCameraAndRegister(const char* cameraName)
	{
		ICamera* pCamera = FB_NEW(Camera);
		return RegisterCamera(cameraName, pCamera);
	}

	//---------------------------------------------------------------------------
	size_t Engine::RegisterCamera(const char* cameraName, ICamera* pCamera)
	{
		pCamera->SetName(cameraName);
		mCameras.push_back(pCamera);
		pCamera->SetCameraIndex(mCameras.size() - 1);
		return mCameras.size() - 1;
	}

	//---------------------------------------------------------------------------
	bool Engine::SetActiveCamera(size_t idx)
	{
		ICamera* cam = GetCamera(idx);
		if (cam)
		{
			mCurrentCamera = cam;
			mRenderer->SetCamera(mCurrentCamera);
			return true;
		}

		return false;
	}


	//---------------------------------------------------------------------------
	ICamera* Engine::GetCamera(size_t idx)
	{
		if ((size_t)idx < mCameras.size())
			return mCameras[idx];

		assert(0 && "Wrong camera index!");
		return 0;
	}

	//---------------------------------------------------------------------------
	ICamera* Engine::GetCamera(const std::string& cameraName)
	{
		auto it = mCameras.begin(), itEnd = mCameras.end();
		for (; it!=itEnd; it++)
		{
			if (stricmp((*it)->GetName(), cameraName.c_str()) == 0)
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
		assert(daeFilePath);
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
		
		if (pMeshObject)
		{
			mMeshObjects[filepath] = pMeshObject;
			return (IMeshObject*)pMeshObject->Clone();
		}
		else
		{
			Log("Mesh %s is not found!", daeFilePath);
			return 0;
		}
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

	//-----------------------------------------------------------------------
	void Engine::DeleteMeshObject(IMeshObject* p)
	{
		if (!p)
			return;
		p->Delete();
	}

	//-----------------------------------------------------------------------
	void Engine::DeleteMeshGroup(IMeshGroup* p)
	{
		if (!p)
			return;
		p->Delete();
	}

	//-----------------------------------------------------------------------
	IParticleEmitter* Engine::GetParticleEmitter(const char* file, bool useSmartPtr)
	{
		IParticleEmitter* p = ParticleManager::GetParticleManager().GetParticleEmitter(file);
		if (p && !useSmartPtr)
		{
			p->AddRef();
		}

		return p;
	}

	//-----------------------------------------------------------------------
	IParticleEmitter* Engine::GetParticleEmitter(unsigned id, bool useSmartPtr)
	{
		IParticleEmitter* p = ParticleManager::GetParticleManager().GetParticleEmitter(id);
		if (p && !useSmartPtr)
		{
			p->AddRef();
		}

		return p;
	}

	//-----------------------------------------------------------------------
	// only when you are not using smart ptr
	void Engine::ReleaseParticleEmitter(IParticleEmitter* p)
	{
		p->Release();
	}

	//---------------------------------------------------------------------------
	void Engine::GetMousePos(long& x, long& y)
	{
		if (mMouse)
		{
			mMouse->GetPos(x, y);
		}
		else
		{
			assert(0 && "no mouse prepared.");
		}
	}

	bool Engine::IsMouseLButtonDown() const
	{
		if (mMouse)
		{
			return mMouse->IsLButtonDown();
		}
		else
		{
			assert(0 && "no mouse prepared.");
		}
		return false;
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
		int yStep = 18;
		IFont* pFont = mRenderer->GetFont();
		if (pFont)
			yStep = (int)pFont->GetHeight();
		
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
	std::string Engine::GetConfigStringValue(const char* section, const char* name)
	{
		if (mINI)
			return mINI->Get(section, name, "");

		return std::string("");
	}

	int Engine::GetConfigIntValue(const char* section, const char* name)
	{
		if (mINI)
			return mINI->GetInteger(section, name, 0);

		return 0;
	}

	bool Engine::GetConfigBoolValue(const char* section, const char* name)
	{
		if (mINI)
			return mINI->GetBoolean(section, name, false);
		return false;
	}


	const DWORD Engine::FILE_CHANGE_BUFFER_SIZE = 4000;
	//----------------------------------------------------------------------------
	void Engine::FileChangeMonitorThread()
	{
		mFileChangeThreadFinished = CreateEvent(0, FALSE, FALSE, "FileChangeThreadFinished");

		mFileChangeBuffer.resize(FILE_CHANGE_BUFFER_SIZE);
		memset(&mOverlapped, 0, sizeof(OVERLAPPED));
		mOverlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);

		while (!mExiting)
		{
			bool success = MonitorFileChange();
			if (!success)
			{
				Log("Fild change monitoring failed!");
				break;
			}

			DWORD dwNumBytes;
			success = GetOverlappedResult(mMonitoringDirectory,
				&mOverlapped, &dwNumBytes, false) != 0;
			if (!success || dwNumBytes == 0)
			{
				continue;				
			}
			assert(dwNumBytes >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));

			ProcessFileChange();
		}

		CloseHandle(mOverlapped.hEvent);
		CleanFileChangeMonitor();
		SetEvent(mFileChangeThreadFinished);
	}

	//----------------------------------------------------------------------------
	bool Engine::MonitorFileChange()
	{
		::CancelIo(mMonitoringDirectory);
		::CloseHandle(mMonitoringDirectory);

		mMonitoringDirectory = CreateFile(mShaderWatchDir.c_str(), FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			0, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
		if (mMonitoringDirectory == INVALID_HANDLE_VALUE)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Cannot open the shader watch directory!");
			FB_LOG_LAST_ERROR();
			return false;
		}

		ResetEvent(mOverlapped.hEvent);
		DWORD writtenBytes=0;
		memset(&mFileChangeBuffer[0], 0, FILE_CHANGE_BUFFER_SIZE);

		BOOL successful = ReadDirectoryChangesW(mMonitoringDirectory, 
			&mFileChangeBuffer[0], FILE_CHANGE_BUFFER_SIZE,
			true, FILE_NOTIFY_CHANGE_LAST_WRITE, &writtenBytes,
			&mOverlapped, 0);
		if (!successful)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "ReadDirectoryChangesW Failed!");
			DWORD lastError = GetLastError();
			Log("\t error code = %d", lastError);
			return false;
		}
		else
		{
			HANDLE handles[] = { mOverlapped.hEvent, mExitFileChangeThread };
			WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		}
		return true;
	}

	//----------------------------------------------------------------------------
	void Engine::ProcessFileChange()
	{
		Sleep(100);
		float time = gFBEnv->pTimer->GetTime();
		FILE_NOTIFY_INFORMATION* pFNI = (FILE_NOTIFY_INFORMATION*)&mFileChangeBuffer[0];
		while (pFNI)
		{
			switch (pFNI->Action)
			{
			case FILE_ACTION_MODIFIED:
			{
										 char fileName[MAX_PATH];
										 int count = WideCharToMultiByte(CP_ACP, 0, pFNI->FileName, pFNI->FileNameLength / sizeof(WCHAR),
											 fileName, _ARRAYSIZE(fileName) - 1, 0, 0);
										 fileName[count] = 0;
										 char unifiedPath[MAX_PATH] = { 0 };
										 UnifyFilepath(unifiedPath, fileName);
										 mChangedFiles.insert(unifiedPath);
										 mLastChangedTime = time;

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

	//------------------------------------------------------------------------
	void Engine::RegisterFileChangeListener(IFileChangeListener* listener)
	{
		assert(std::find(mFileChangeListeners.begin(), mFileChangeListeners.end(), listener) == mFileChangeListeners.end());
		mFileChangeListeners.push_back(listener);
	}

	//------------------------------------------------------------------------
	void Engine::RemoveFileChangeListener(IFileChangeListener* listener)
	{
		mFileChangeListeners.erase(std::remove(mFileChangeListeners.begin(), mFileChangeListeners.end(), listener),
			mFileChangeListeners.end());
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