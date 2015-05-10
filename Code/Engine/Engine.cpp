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
#include <Engine/IRenderTarget.h>
#include <Engine/Mouse.h>
#include <Engine/Keyboard.h>
#include <Engine/Engine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>
#include <Engine/Shader.h>
#include <Engine/ParticleManager.h>
#include <Engine/PointLightMan.h>
#include <Engine/Scene.h>
#include <Engine/SkySphere.h>
#include <Engine/IRenderTarget.h>
#include <Engine/IVoxelizer.h>
#include <Engine/IBillboardQuad.h>
#include <Engine/IDustRenderer.h>
#include <Engine/ScriptSystem.h>
#include <CommonLib/INIReader.h>
#include <CommonLib/StringUtils.h>
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
	FileSystem::Initialize();
	mErrorStream.open("error.log");
	mStdErrorStream = std::cerr.rdbuf(mErrorStream.rdbuf());
	char timestring[256];
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, 0, 0, timestring, 256);
	std::cerr << "Engine Initialized at " << timestring << std::endl;
	gFBEnv = FB_NEW(GlobalEnv);
	gFBEnv->pEngine = this;
	gFBEnv->pTimer = FB_NEW(Timer);
	gpTimer = gFBEnv->pTimer;
	//mEngineCamera = 0;
	mFileChangeThreadFinished = 0;
	mExitFileChangeThread = 0;
	mExiting = false;
	mFileMonitorThread = 0;
	mMonitoringDirectory = INVALID_HANDLE_VALUE;
	mINI = 0;
		
	mScriptSystem = FB_NEW(ScriptSystem);
	gFBEnv->pScriptSystem = mScriptSystem;

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
			FB_LOG_LAST_ERROR_ENG();
		}
		CloseHandle(mFileChangeThreadFinished);
	}

	mTerrain=0;
	mSkyBox=0;

	FreeImage_DeInitialise();
	std::cerr.rdbuf(mStdErrorStream);
	mErrorStream.close();
	xmlCleanupParser();
	mSkyBox = 0;
	mTerrain = 0;
	mMeshObjects.clear();
	mFractureObjects.clear();
	mMeshGroups.clear();
	mRenderer = 0;
	//mCameras.clear();
	mConsole = 0;
	FB_SAFE_DEL(gFBEnv->pTimer);
	FB_SAFE_DEL(mINI);
	mScriptSystem = 0;
	mMouse = 0;
	mKeyboard = 0;
	gFBEnv->pEngine=0;	
	FB_SAFE_DEL(gFBEnv);
	FileSystem::Finalize();
#ifdef USING_FB_MEMORY_MANAGER
	FBReportMemoryForModule();
#endif
}

//------------------------------------------------------------------------
GlobalEnv* Engine::GetGlobalEnv() const
{
	return gFBEnv;
}

//------------------------------------------------------------------------
HWND_ID Engine::CreateEngineWindow(int x, int y, int width, int height,
	const char* wndClass, const char* title, WNDPROC winProc)
{
	int eWidth = width + 16;
	int eHeight = height + 38;
	const char* myclass = "FBEngineClass" ;
	WNDCLASSEX wndclass = { sizeof(WNDCLASSEX), CS_DBLCLKS, winProc,
							0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION),
							LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW+1),
							0, myclass, LoadIcon(0,IDI_APPLICATION) } ;

	if( RegisterClassEx(&wndclass) )
	{
		auto hWnd = CreateWindowEx( 0, myclass, title,
			WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, x, y,
			eWidth, eHeight, 0, 0, GetModuleHandle(0), 0);
		auto id = FindEmptyHwndId();
		m_hWnd[id] = hWnd;
		m_hWndId[hWnd] = id;

		RegisterMouseAndKeyboard(hWnd);

		/*HWND console = GetConsoleWindow();
		RECT consoleRect;
		GetWindowRect(console, &consoleRect);
		MoveWindow(console, x+width, y, consoleRect.right-consoleRect.left, 
			consoleRect.bottom-consoleRect.top, TRUE);*/

		ShowWindow(hWnd, TRUE);
		mRequestedWndSize[hWnd] = Vec2I(width, height);

		return id;
	}

	return 0;
}

const Vec2I& Engine::GetRequestedWndSize(HWND hWnd) const
{
	auto it = mRequestedWndSize.Find(hWnd);
	if (it != mRequestedWndSize.end())
	{
		return it->second;
	}

	Error(FB_DEFAULT_DEBUG_ARG, "Cannot find the requested window size.");
	return Vec2I::ZERO;
}

//------------------------------------------------------------------------
HWND Engine::GetWindowHandle(HWND_ID id) const
{
	auto itFound = m_hWnd.Find(id);
	if (itFound != m_hWnd.end())
	{
		return itFound->second;
	}
	return 0;
}

HWND_ID Engine::GetWindowHandleId(HWND hWnd) const
{
	auto itFound = m_hWndId.Find(hWnd);
	if (itFound != m_hWndId.end())
	{
		return itFound->second;
	}
	return 0;
}

//------------------------------------------------------------------------
HWND Engine::GetMainWndHandle() const
{
	if (m_hWnd.empty()){
		Error(FB_DEFAULT_DEBUG_ARG, "No window!");
		return 0;
	}
	assert(m_hWnd.begin()->first == 1);
	return m_hWnd.begin()->second; // the first window
}

//------------------------------------------------------------------------
HWND_ID Engine::GetMainWndHandleId() const{
	if (m_hWnd.empty()){
		Error(FB_DEFAULT_DEBUG_ARG, "No window!");
		return 0;
	}
	assert(m_hWnd.begin()->first == 1);
	return m_hWnd.begin()->first; // the first window
}

HWND Engine::GetForgroundWindow() const{
	HWND hwnd = GetForgroundWindow();
	if (hwnd)
		return hwnd;

	return GetMainWndHandle();
}


//------------------------------------------------------------------------
bool Engine::InitEngine(int rendererType)
{
	mINI = FB_NEW(INIReader)("Engine.ini");
	if (mINI->GetError())
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Could not parse the Engine.ini file!");
		assert(0);
	}
	int threadPool = mINI->GetInteger("Render", "ThreadPool", 0);

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
int Engine::InitSwapChain(HWND_ID id, int width, int height)
{
	auto hWnd = GetWindowHandle(id);
	if (!hWnd)
	{
		Error(FB_DEFAULT_DEBUG_ARG, "Invalid window id");
		return 0;
	}
	int index = -1;
	if (mRenderer)
	{
		index = mRenderer->InitSwapChain(id, width, height);
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

bool Engine::RegisterMouseAndKeyboard(HWND hWnd)
{
	if (!mMouse)
	{
		mMouse = FB_NEW(Mouse);
		mKeyboard = FB_NEW(Keyboard);
	}
#ifdef _FBENGINE_FOR_WINDOWS_
	const unsigned short HID_USAGE_PAGE_GENERIC = 0x01;
	const unsigned short HID_USAGE_GENERIC_MOUSE = 0x02;
	const unsigned short HID_USAGE_GENERIC_KEYBOARD = 0x06;
		
	RAWINPUTDEVICE Rid[2];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
	Rid[0].dwFlags = 0;   
	Rid[0].hwndTarget = hWnd;
	Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC; 
	Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD; 
	Rid[1].dwFlags = 0;   
	Rid[1].hwndTarget = hWnd;

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
	static ProfilerSimple profiler(L"Update Engine");
	bool profiling = gFBEnv->pConsole->GetEngineCommand()->e_profile !=0;
	profiler.Reset();
	static ProfilerSimple profilerParticle(L"Update Particle");

	auto const renderer = gFBEnv->_pInternalRenderer;
	renderer->ProcessInputData();

	if (mTerrain)
		mTerrain->Update();

	// Update Particles
	profilerParticle.Reset();
	ParticleManager::GetParticleManager().Update(dt);
	if (profiling)
		DrawProfileResult(profilerParticle, "ProfilePosParticle", 1);
	mRenderer->UpdateCloud(dt);

	// concole;

	if (mConsole)
		mConsole->Update();

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

	// hot reloading
	HotReloading();

	if (profiling)
	{
		DrawProfileResult(profiler, "ProfilePosEngine");
	}
}

void Engine::HandleUserInput()
{
	if (mKeyboard)
	{
		if (mKeyboard->IsKeyPressed('W') && mKeyboard->IsKeyDown(VK_CONTROL) && mKeyboard->IsKeyDown(VK_LMENU))
		{
			bool wire = GetRenderer()->GetWireframe();
			GetRenderer()->SetWireframe(!wire);
		}

		if (mKeyboard->IsKeyPressed('S') && mKeyboard->IsKeyDown(VK_CONTROL) && mKeyboard->IsKeyDown(VK_LMENU))
		{
			
			auto scene = mRenderer->GetScene();
			if (scene)
			{
				scene->ToggleSkyRendering();
			}
		}

		if (mKeyboard->IsKeyPressed(VK_OEM_3)) // `
		{
			if (mConsole)
			{
				mConsole->ToggleOpen();
			}
		}
	}


	gFBEnv->_pInternalRenderer->OnInputFromEngineForCamera(mMouse, mKeyboard);
		

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
		if (gFBEnv->pTimer->IsPause() || gFBEnv->pTimer->GetTime() - mLastChangedTime > 0.5f)
		{
			for (auto it = mChangedFiles.begin(); it != mChangedFiles.end(); )
			{
				std::string filepath = it->c_str();
				auto strs = Split(filepath, "~");
				if (strs.size() >= 2)
				{
					filepath = strs[0];
				}
				const char* extension = GetFileExtension(filepath.c_str());
				bool shader = _stricmp(extension, "hlsl") == 0 || _stricmp(extension, "h")==0;
				bool material = _stricmp(extension, "material")==0;
				bool texture = _stricmp(extension, "png") == 0 || _stricmp(extension, "dds")==0;
				bool particle = _stricmp(extension, "particle")==0;
				bool xml = _stricmp(extension, "xml") == 0;
				bool hasExtension = strlen(extension) != 0;
				bool sdfFile = _stricmp(extension, "sdf") == 0;
				bool canOpen = true;
				if (!hasExtension || sdfFile)
				{
					auto nextit = it;
					++nextit;
					mChangedFiles.erase(it);
					it = nextit;
					continue;
				}
				FILE* file = 0;
				errno_t err = fopen_s(&file, filepath.c_str(), "r");
				if (!err && file)
				{
					fclose(file);
					err = fopen_s(&file, filepath.c_str(), "a+");
					if (err)
						canOpen = false;
					else if (file)
						fclose(file);
				}
					

				if (canOpen)
				{
					if (shader || material || texture || particle || xml)
					{
						if (shader)
							IShader::ReloadShader(filepath.c_str());
						else if (material)
							IMaterial::ReloadMaterial(filepath.c_str());
						else if (texture)
							ITexture::ReloadTexture(filepath.c_str());
						else if (particle)
							ParticleManager::GetParticleManager().ReloadParticle(filepath.c_str());
						else if (xml && gFBEnv->pRenderer)
							mRenderer->ReloadTextureAtlas(filepath.c_str());

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

void Engine::Render(float dt)
{
	if (!mRenderer)
		return;

	mRenderer->Render(dt);

	if (mConsole)
		mConsole->Render();

	if (mConsole->GetEngineCommand()->e_profile)
		mRenderer->RenderFrameProfiler();

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
	if (!mRenderer)
		return false;	
	auto scene = mRenderer->GetMainScene();
	if (scene)
	{
		mSkyBox = ISkyBox::CreateSkyBoxInstance();
		mSkyBox->Init();
		scene->AttachSkyBox(mSkyBox);
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------
IMeshObject* Engine::GetMeshObject(const char* daeFilePath, 
		bool reload, const MeshImportDesc& desc)
{
	assert(daeFilePath);
	std::string filepath = daeFilePath;
	ToLowerCase(filepath);
	if (gFBEnv->pConsole->GetEngineCommand()->e_NoMeshLoad)
	{
		filepath = "es/objects/defaultCube.dae";
	}
	if (!reload)
	{
		auto it = mMeshObjects.find(filepath);
		if (it!=mMeshObjects.end())
			return (IMeshObject*)it->second->Clone();
	}
	SmartPtr<IColladaImporter> pColladaImporter = IColladaImporter::CreateColladaImporter();
	pColladaImporter->ImportCollada(filepath.c_str(), desc.yzSwap, desc.oppositeCull, desc.useIndexBuffer,
		desc.mergeMaterialGroups, desc.keepMeshData, desc.generateTangent, false);
	IMeshObject* pMeshObject = pColladaImporter->GetMeshObject();
		
	if (pMeshObject)
	{
		pMeshObject->SetName(filepath.c_str());
		mMeshObjects[filepath] = pMeshObject;
		return (IMeshObject*)pMeshObject->Clone();
	}
	else
	{
		Log("Mesh %s is not found!", filepath.c_str());
		return 0;
	}
}

//---------------------------------------------------------------------------
IMeshObject* Engine::CreateMeshObject()
{
	return IMeshObject::CreateMeshObject();
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
	if (!pMeshGroup)
		return 0;
	mMeshGroups[filepath] = pMeshGroup;
	return (IMeshGroup*)pMeshGroup->Clone();
}

//---------------------------------------------------------------------------
void Engine::GetFractureMeshObjects(const char* daeFilePath, std::vector<IMeshObject*>& objects, bool reload )
{
	objects.clear();
	assert(daeFilePath);
	std::string filepath = daeFilePath;
	ToLowerCase(filepath);
	if (gFBEnv->pConsole->GetEngineCommand()->e_NoMeshLoad)
	{
		filepath = "es/objects/defaultCube.dae";
	}
	if (!reload)
	{
		auto it = mFractureObjects.find(filepath);
		if (it != mFractureObjects.end())
		{
			for (auto& data : it->second)
			{
				objects.push_back((IMeshObject*)data->Clone());
			}
			return;
		}
	}

	MeshImportDesc desc;
	SmartPtr<IColladaImporter> pColladaImporter = IColladaImporter::CreateColladaImporter();
	pColladaImporter->ImportCollada(filepath.c_str(), desc.yzSwap, desc.oppositeCull, desc.useIndexBuffer,
		desc.mergeMaterialGroups, true, desc.generateTangent, false);
	auto iterator = pColladaImporter->GetMeshIterator();
	mFractureObjects[filepath].clear();
	while (iterator.HasMoreElement())
	{
		auto data = iterator.GetNext();
		mFractureObjects[filepath].push_back(data.second);
	}
		
	if (!mFractureObjects[filepath].empty())
	{
		for (auto& data : mFractureObjects[filepath])
		{
			objects.push_back((IMeshObject*)data->Clone());
		}
	}
	else
	{
		Log("Mesh %s is not found!", filepath.c_str());
	}
}

const IMeshObject* Engine::GetMeshArchetype(const std::string& name) const
{
	auto it = mMeshObjects.find(name);
	assert(it != mMeshObjects.end());
	return it->second;
}
//-----------------------------------------------------------------------
void Engine::ReleaseMeshObject(IMeshObject* p)
{
	if (!p)
		return;
	p->Delete();
}

//-----------------------------------------------------------------------
void Engine::ReleaseMeshGroup(IMeshGroup* p)
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
				Log(FB_DEFAULT_DEBUG_ARG, "Input listeners has the same priority. sorted by address.");
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
#ifdef _FBENGINE_FOR_WINDOWS_
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
					pEngine->mMouse->PushEvent(window,  *((MouseEvent*)&raw->data.mouse));
				}
				return 0;
			case RIM_TYPEKEYBOARD:
				{
					pEngine->mKeyboard->PushEvent(window, *((KeyboardEvent*)&raw->data.keyboard));
				}
				return 0;
			}
		}
		break;

	case WM_CHAR:
		{
			if (pEngine->mKeyboard)
			{
				pEngine->mKeyboard->PushChar(window, wp);
			}
		}
		return 0; // processed

	case WM_SETFOCUS:
		{
			if (pEngine->mMouse)
				pEngine->mMouse->OnSetFocus(window);
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


const DWORD Engine::FILE_CHANGE_BUFFER_SIZE = 8000;
//----------------------------------------------------------------------------
void Engine::FileChangeMonitorThread()
{
	mFileChangeThreadFinished = CreateEvent(0, FALSE, FALSE, "FileChangeThreadFinished");

	mFileChangeBuffer.resize(FILE_CHANGE_BUFFER_SIZE);
	memset(&mOverlapped, 0, sizeof(OVERLAPPED));
	mOverlapped.hEvent = CreateEvent(0, TRUE, FALSE, 0);

	while (!mExiting)
	{
		auto handleBackup = mOverlapped.hEvent;
		memset(&mOverlapped, 0, sizeof(OVERLAPPED));
		mOverlapped.hEvent = handleBackup;
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

	if (mOverlapped.hEvent)
		CloseHandle(mOverlapped.hEvent);
	CleanFileChangeMonitor();
	SetEvent(mFileChangeThreadFinished);
}

//----------------------------------------------------------------------------
bool Engine::MonitorFileChange()
{
	//::CancelIo(mMonitoringDirectory);
	//::CloseHandle(mMonitoringDirectory);
	if (mMonitoringDirectory == INVALID_HANDLE_VALUE)
	{
		mMonitoringDirectory = CreateFile(mShaderWatchDir.c_str(), FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			0, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
		if (mMonitoringDirectory == INVALID_HANDLE_VALUE)
		{
			Log(FB_DEFAULT_DEBUG_ARG, "Cannot open the shader watch directory!");
			FB_LOG_LAST_ERROR_ENG();
			return false;
		}
	}		

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
void Engine::DrawProfileResult(ProfilerSimple& p, const char* posVarName, int tab )
{
	wchar_t buf[256];
	std::wstring tapString;
	while (tab--)
	{
		tapString.push_back('\t');
	}
	swprintf_s(buf, L"%s%s : %f", tapString.c_str(), p.GetName(), p.GetDT());
	Vec2I pos = GetLuaVarAsVec2I(gFBEnv->pScriptSystem->GetLuaState(), posVarName);
	gFBEnv->pRenderer->DrawText(pos, buf, Color::White);
}

void Engine::DrawProfileResult(wchar_t* buf, const char* posVarName, int tab )
{
	Vec2I pos = GetLuaVarAsVec2I(gFBEnv->pScriptSystem->GetLuaState(), posVarName);
	gFBEnv->pRenderer->DrawText(pos, buf, Color::White);
}

//------------------------------------------------------------------------
IScene* Engine::CreateScene()
{
	return FB_NEW(Scene);
}

void Engine::DeleteScene(IScene* p)
{
	FB_DELETE(p);
}


//------------------------------------------------------------------------
void Engine::Log(const char* szFmt, ...) const
{
	char buf[2048];

	int len = 0;
	if (gFBEnv && gFBEnv->pTimer)
	{
		Timer::TIME_PRECISION time = gFBEnv->pTimer->GetTime();
		sprintf_s(buf, "[%.3f] \n", time);
		len = strlen(buf);
	}
		
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf + len, 2048 - len , szFmt, args);
	va_end(args);
	strcat_s(buf, 2048, "\n");
	std::cout << buf;
	OutputDebugString(buf);
	buf[strlen(buf) - 1] = 0;

	if (gFBEnv && gFBEnv->pConsole)
		gFBEnv->pConsole->Log(buf);
}

//------------------------------------------------------------------------
void Engine::Error(const char* szFmt, ...) const
{
	char buf[2048];

	int len = 0;
	if (gFBEnv && gFBEnv->pTimer)
	{
		char timebuf[128];
		Timer::TIME_PRECISION time = gFBEnv->pTimer->GetTime();
		sprintf_s(timebuf, "Error [%.3f] : \n", time);
		OutputDebugString(timebuf);
	}
				
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);		
		
	strcat_s(buf, 2048, "\n");
	OutputDebugString(buf);
	buf[strlen(buf) - 1] = 0;

	if (gFBEnv && gFBEnv->pConsole)
		gFBEnv->pConsole->Log(buf);
}

//------------------------------------------------------------------------
void Engine::LogLastError(const char* file, int line, const char* function) const
{
	char buf[2048];

	DWORD err = GetLastError();
	if (err==0)
		return;

	LPVOID lpMsgBuf;

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL );

	sprintf_s(buf, 2048, "%s(%d): %s() - %s \n", file, line, function, lpMsgBuf);
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


IVoxelizer* Engine::CreateVoxelizer()
{
	return IVoxelizer::CreateVoxelizer();
}

void Engine::DeleteVoxelizer(IVoxelizer* voxelizer)
{
	if (voxelizer)
		IVoxelizer::DeleteVoxelizer(voxelizer);
}

IUIObject* Engine::CreateUIObject(bool usingSmartPtr, const Vec2I& renderTargetSize)
{
	return IUIObject::CreateUIObject(usingSmartPtr, renderTargetSize);
}
void Engine::DeleteUIObject(IUIObject* uiObject)
{
	if (uiObject)
	{
		FB_RELEASE(uiObject);
	}
}

ISkySphere* Engine::CreateSkySphere(bool usingSmartPointer)
{
	return ISkySphere::CreateSkySphere(usingSmartPointer);
}

void Engine::DeleteSkySphere(ISkySphere* skySphere)
{
	if (skySphere)
		skySphere->Delete();
}

IBillboardQuad* Engine::CreateBillboardQuad()
{
	return IBillboardQuad::CreateBillboardQuad();
}

void Engine::DeleteBillboardQuad(IBillboardQuad* quad)
{
	if (quad)
	{
		FB_DELETE(quad);
	}
}

IDustRenderer* Engine::CreateDustRenderer()
{
	return IDustRenderer::CreateDustRenderer();
}

void Engine::DeleteDustRenderer(IDustRenderer* dust)
{
	FB_DELETE(dust);
}


HWND_ID Engine::FindEmptyHwndId() const
{
	HWND_ID id = 1;
	unsigned count = 500000; // just a big number
	while (count--)
	{
		bool used = false;
		for (auto& it : m_hWnd)
		{
			if (it.first == id)
			{
				used = true;
				++id;
				break;
			}
		}
		if (!used)
		{
			return id;
		}
	}
	Error(FB_DEFAULT_DEBUG_ARG, "Hwnd id is full.");
	assert(0);
	return -1;
}

} // namespace fastbird