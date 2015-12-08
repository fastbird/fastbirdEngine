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

#include "stdafx.h"
#include "EngineFacade.h"
#include "EngineOptions.h"
#include "Voxelizer.h"
#include "MeshFacade.h"
#include "FBTimer/Profiler.h"
#include "FBFileSystem/FileSystem.h"
#include "FBSystemLib/System.h"
#include "FBLua/LuaObject.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/Camera.h"
#include "FBRenderer/RendererOptions.h"
#include "FBRenderer/Font.h"
#include "FBSceneManager/SceneManager.h"
#include "FBSceneManager/Scene.h"
#include "FBSceneManager/DirectionalLight.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"
#include "FBSceneObjectFactory/SkySphere.h"
#include "FBThreadLib/TaskScheduler.h"
#include "FBConsole/Console.h"
#include "FBInputManager/InputManager.h"
#include "FBFileMonitor/FileMonitor.h"
#include "FBVideoPlayer/VideoPlayerOgg.h"
#include "FBParticleSystem/ParticleSystem.h"
#include "FBAudioPlayer/AudioManager.h"
using namespace fb;
namespace fb{
	void InitEngineLua();
}
class EngineFacade::Impl{
public:
	static const int MainWindowId = 1;	
	static HWindow MainWindowHandle;
	EngineFacadeWeakPtr mSelfPtr;
	std::map<HWindowId, HWindow> mWindowById;
	std::map<HWindow, HWindowId> mWindowIdByHandle;
	HWindowId mNextWindowId;
	lua_State* mL;
	
	// Engine Objects
	ConsolePtr mConsole;
	TaskSchedulerPtr mTaskSchedular;
	InputManagerPtr mInputManager;
	RendererPtr mRenderer;
	SceneManagerPtr mSceneManager;
	ScenePtr mMainScene;
	CameraPtr mMainCamera;
	ScenePtr mTemporalOverridingScene;
	bool mLockSceneOverriding;
	SceneObjectFactoryPtr mSceneObjectFactory;
	ParticleSystemPtr mParticleSystem;	
	EngineOptionsPtr mEngineOptions;
	FileMonitorPtr mFileMonitor;
	AudioManagerPtr mAudioManager;
	bool mRayFromCursorCalced;

	// EngineFacade
	std::vector<MeshFacadePtr> mTempMeshes;
	std::map<std::string, std::vector< MeshFacadePtr> > mFractureObjects;
	LuaObject mInputOverride;

	//---------------------------------------------------------------------------
	Impl()
		: mL(0)
		, mNextWindowId(MainWindowId)
		, mRayFromCursorCalced(false)
		, mLockSceneOverriding(false)
	{
		FileSystem::StartLoggingIfNot();
		auto filepath = "_FBEngineFacade.log";		
		FileSystem::BackupFile(filepath, 5, "Backup_Log");
		Logger::Init(filepath);
		FileSystem::BackupFile("_Global.log", 5, "Backup_Log");
		Logger::InitGlobalLog("_Global.log");
		mL = LuaUtils::OpenLuaState();
		InitEngineLua();
		mInputManager = InputManager::Create();
		mConsole = Console::Create();
		mTaskSchedular = TaskScheduler::Create(6);
		
		mSceneManager = SceneManager::Create();
		if (!mSceneManager){
			Logger::Log(FB_ERROR_LOG_ARG, "Cannot create SceneManager.");
		}
		else{
			mMainScene = mSceneManager->CreateScene("MainScene");
			if (!mMainScene){
				Logger::Log(FB_ERROR_LOG_ARG, "Cannot create the main scene.");
			}
		}		
		mEngineOptions = EngineOptions::Create();
		mRenderer = Renderer::Create();
		mSceneObjectFactory = SceneObjectFactory::Create();
		mAudioManager = AudioManager::Create();
		mAudioManager->Init();

		mInputManager->RegisterInputConsumer(mRenderer, IInputConsumer::Priority77_CAMERA);
	}

	~Impl(){
		mAudioManager->Deinit();
		SkySphere::DestroySharedEnvRT();
		LuaUtils::CloseLuaState(mL);
		FileSystem::StopLogging();
		Logger::Release();
	}

	HWindowId FindEmptyHwndId()
	{
		return mNextWindowId++;
	}

	void PrepareFileMonitor(){
		mFileMonitor = FileMonitor::Create();
		mFileMonitor->AddObserver(IFileChangeObserver::FileChange_Engine, mRenderer);
		mFileMonitor->StartMonitor(".");
		mFileMonitor->AddObserver(IFileChangeObserver::FileChange_Engine, mSelfPtr.lock());
	}

	HWindowId CreateEngineWindow(int x, int y, int width, int height, const char* wndClass, 
		const char* title, unsigned style, unsigned exStyle, WNDPROC winProc)
	{
		Vec2I resol(width, height);
		if (width == 0 || height == 0){
			// check the config
			resol = Renderer::GetInstance().GetRendererOptions()->r_resolution;
			width = resol.x;
			height = resol.y;
		}
		else{
			Renderer::GetInstance().GetRendererOptions()->r_resolution = resol;
		}

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRect(&rect, style, false);

		int eWidth = rect.right - rect.left;
		int eHeight = rect.bottom - rect.top;
		WNDCLASSEX wndclass = { sizeof(WNDCLASSEX), CS_DBLCLKS, winProc,
			0, 0, GetModuleHandle(0), LoadIcon(0, IDI_APPLICATION),
			NULL, HBRUSH(COLOR_WINDOW + 1),
			0, wndClass, LoadIcon(0, IDI_APPLICATION) };

		WNDCLASSEX classInfo;
		BOOL registered = GetClassInfoEx(GetModuleHandle(NULL), wndClass, &classInfo);
		if (!registered)
		{
			registered = RegisterClassEx(&wndclass);
		}
		if (registered)
		{
			auto hWnd = CreateWindowEx(exStyle, wndClass, title,
				style, x, y,
				eWidth, eHeight, 0, 0, GetModuleHandle(0), 0);
			auto id = FindEmptyHwndId();
			if (id == MainWindowId){
				MainWindowHandle = (HWindow)hWnd;
				Renderer::GetInstance().SetMainWindowStyle(style);
				InputManager::GetInstance().SetMainWindowHandle((HWindow)hWnd);
				PrepareFileMonitor();				
			}

			mWindowById[id] = (HWindow)hWnd;
			mWindowIdByHandle[(HWindow)hWnd] = id;			
			ShowWindow(hWnd, TRUE);
			return id;
		}
		return INVALID_HWND_ID;
	}

	void DestroyEngineWindow(HWindowId windowId){
		mRenderer->DeinitCanvas(windowId);
		auto it = mWindowById.find(windowId);
		if (it != mWindowById.end()){
			DestroyWindow((HWND)it->second);
		}
		else{
			Logger::Log(FB_ERROR_LOG_ARG, "Window not found");
		}
	}

	HWindowId GetMainWindowHandleId() const{
		return MainWindowId;
	}

	HWindow GetMainWindowHandle() const{
		return MainWindowHandle;
	}

	HWindow GetWindowHandleById(HWindowId hwndId) const{
		auto it = mWindowById.find(hwndId);
		if (it != mWindowById.end()){
			return it->second;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the window handle by id(%u)", hwndId).c_str());
		return INVALID_HWND;
	}


	bool InitRenderer(const char* pluginName){		
		bool success = Renderer::GetInstance().PrepareRenderEngine(pluginName);		
		if (success){			
			mParticleSystem = ParticleSystem::Create();			
		}
		return success;
	}

	bool InitCanvas(HWindowId id, int width, int height){
		if (!mRenderer){
			Logger::Log(FB_ERROR_LOG_ARG, "Renderer is not initialized.");
			return false;
		}
		auto window = mWindowById[id];		
		bool sucess = mRenderer->InitCanvas(id, window, width, height);
		if (!sucess){
			return INVALID_HWND_ID;
		}
		else{			
			mInputManager->AddHwndInterested(window);
			auto rt = mRenderer->GetRenderTarget(id);
			if (!rt){
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("RenderTarget(%u) is not initialized.", id).c_str());
			}
			else{
				auto rtObservers = mInputManager->GetRenderTargetObservers();
				for (auto observer : rtObservers){
					rt->AddObserver(IRendererObserver::DefaultRenderEvent, observer);
				}
				if (id == MainWindowId){
					rt->RegisterScene(mMainScene);
					mMainCamera = rt->GetCamera();
					mMainCamera->SetMainCamera(true);
					SkySphere::CreateSharedEnvRT();
				}
			}
			return true;
		}
	}

	bool InitCanvas(HWindow hwnd){
		// Window is not created by EngineFacade.
		// The following function calls need to be performed.
		if (mWindowIdByHandle.empty()){
			MainWindowHandle = (HWindow)hwnd;
			Renderer::GetInstance().SetMainWindowStyle(GetWindowStyle(hwnd));
			InputManager::GetInstance().SetMainWindowHandle(hwnd);			
			PrepareFileMonitor();			
		}

		auto idIt = mWindowIdByHandle.find(hwnd);
		HWindowId id;
		if (idIt == mWindowIdByHandle.end()){
			id = FindEmptyHwndId();
			mWindowIdByHandle[hwnd] = id;
			mWindowById[id] = hwnd;
		}
		else{
			id = idIt->second;
		}
		Vec2I size = GetWindowClientSize(hwnd);
		return InitCanvas(id, size.x, size.y);		
	}
	

	void OverrideMainScene(IScenePtr scene){
		if (mLockSceneOverriding)
			return;
		if (scene)
			Renderer::GetInstance().GetMainRenderTarget()->RegisterScene(scene);
		else
			Renderer::GetInstance().GetMainRenderTarget()->RegisterScene(mMainScene);		
	}

	void OverrideMainScene(){
		if (mLockSceneOverriding)
			return;
		mTemporalOverridingScene = mSceneManager->CreateScene( FormatString("TemporalScene%d%u", std::rand(), gpTimer->GetTickCount()).c_str() );
		OverrideMainScene(mTemporalOverridingScene);
	}

	ScenePtr GetMainScene() const{
		return mMainScene;
	}

	ScenePtr GetCurrentScene() const{
		return std::static_pointer_cast<Scene>(Renderer::GetInstance().GetMainRenderTarget()->GetScene());
	}

	void SetDrawClouds(bool draw){
		mMainScene->SetDrawClouds(draw);
	}

	void UpdateInput(){
		mRayFromCursorCalced = false;
		mInputManager->Update();
	}

	void Update(TIME_PRECISION dt){		
		mSceneManager->Update(dt);
		mSceneObjectFactory->Update(dt);
		mParticleSystem->Update(dt);
		mAudioManager->Update(dt);
		mInputManager->EndFrame(gpTimer->GetTime());		
	}

	void Render(){
		mRenderer->Render();
		if (mEngineOptions->e_profile){
			mRenderer->DisplayFrameProfiler();
			auto numSpatialObjects = mMainScene->GetNumSpatialObjects();
			auto numObjects = mMainScene->GetNumObjects();
			wchar_t msg[255];
			int x = 1300;
			int y = 110;
			int yStep = 20;			
			
			swprintf_s(msg, 255, L"Num SpatialObjects = %d", numSpatialObjects);
			mRenderer->QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
			y += yStep;

			swprintf_s(msg, 255, L"Num Objects = %d", numObjects);
			mRenderer->QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
			y += yStep;
		}
	}

	void AddTempMesh(MeshFacadePtr mesh){
		mTempMeshes.push_back(mesh);
	}

	std::vector<MeshFacadePtr> CreateFractureMeshes(const char* daeFilePath){
		std::vector<MeshFacadePtr> ret;
		MeshImportDesc desc;
		desc.keepMeshData = true;
		auto meshObjects = mSceneObjectFactory->CreateMeshObjects(daeFilePath, desc);
		for (auto meshObject : meshObjects){
			auto meshFacade = MeshFacade::Create();
			meshFacade->SetMeshObject(meshObject);
			ret.push_back(meshFacade);
		}
		return ret;
	}

	void GetFractureMeshObjects(const char* daeFilePath, std::vector<MeshFacadePtr>& objects){
		if (ValidCStringLength(daeFilePath))
			return;

		objects.clear();
		std::string filepath(daeFilePath);
		if (mEngineOptions->e_NoMeshLoad)
		{
			filepath = "EssentialEngineData/objects/defaultCube.dae";
		}
		std::string keyFilepath = filepath;
		ToLowerCase(keyFilepath);
		auto it = mFractureObjects.find(keyFilepath);
		if (it != mFractureObjects.end())
		{
			for (auto& data : it->second)
			{
				objects.push_back(data->Clone());
			}
			return;
		}

		auto meshes = CreateFractureMeshes(filepath.c_str());
		mFractureObjects[keyFilepath] = meshes;
		for (auto& mesh : meshes){
			objects.push_back(mesh->Clone());
		}
	}

	const Ray3& GetWorldRayFromCursor(){
		static Ray3 worldRay(Vec3::ZERO, Vec3::UNIT_Y);
		if (mRayFromCursorCalced){
			return worldRay;
		}

		auto cam = mMainCamera;
		if (cam)
		{
			mRayFromCursorCalced = true;
			auto injector = mInputManager->GetInputInjector();
			long x, y;
			injector->GetMousePos(x, y);
			worldRay = cam->ScreenPosToRay(x, y);
		}
		else
		{
			Logger::Log(FB_ERROR_LOG_ARG, "No main camera found.");
		}

		return worldRay;
	}

	void AddDirectionalLightCoordinates(DirectionalLightIndex::Enum idx, Real phi, Real theta){
		if (!mMainScene){
			Logger::Log(FB_ERROR_LOG_ARG, "No main scene found.");
			return;
		}
		auto light = mMainScene->GetDirectionalLight(idx);
		assert(light);
		light->AddPhi(phi);
		light->AddTheta(theta);
	}

	const Vec3& GetLightDirection(DirectionalLightIndex::Enum idx){
		if (!mMainScene){
			Logger::Log(FB_ERROR_LOG_ARG, "No main scene found");
			return Vec3::UNIT_Y;
		}
		auto light = mMainScene->GetDirectionalLight(idx);
		return light->GetDirection();
	}

	const Vec3& GetLightDiffuse(DirectionalLightIndex::Enum idx){
		if (!mMainScene){
			Logger::Log(FB_ERROR_LOG_ARG, "No main scene found");
			return Vec3::UNIT_Y;
		}
		auto light = mMainScene->GetDirectionalLight(idx);
		return light->GetDiffuse();
	}

	const Real GetLightIntensity(DirectionalLightIndex::Enum idx){
		if (!mMainScene){
			Logger::Log(FB_ERROR_LOG_ARG, "No main scene found");
			return 0.5f;
		}
		auto light = mMainScene->GetDirectionalLight(idx);
		return light->GetIntensity();
	}

	void SetLightIntensity(IScenePtr iscene, DirectionalLightIndex::Enum idx, Real intensity){
		if (!iscene){
			Logger::Log(FB_ERROR_LOG_ARG, "Not valid scene");
			return;
		}
		auto scene = std::static_pointer_cast<Scene>(iscene);
		auto light = scene->GetDirectionalLight(idx);
		light->SetIntensity(intensity);
	}

	AudioId PlayAudio(const char* filepath){
		return mAudioManager->PlayAudio(filepath);
	}

	AudioId PlayAudio(const char* filepath, const Vec3& pos){
		return mAudioManager->PlayAudio(filepath, pos.x, pos.y, pos.z);
	}

	AudioId PlayAudio(const char* filepath, const AudioProperty& prop){
		return mAudioManager->PlayAudio(filepath, prop);
	}

	bool SetAudioPosition(AudioId id, const Vec3& pos){
		return mAudioManager->SetPosition(id, pos.x, pos.y, pos.z);
	}

	void SetListenerPosition(const Vec3& pos){
		return mAudioManager->SetListenerPosition(pos.x, pos.y, pos.z);
	}
};
HWindow EngineFacade::Impl::MainWindowHandle = INVALID_HWND;

//---------------------------------------------------------------------------
EngineFacade* sFacadeRaw = 0;
static EngineFacadeWeakPtr sFacade;
EngineFacadePtr EngineFacade::Create(){
	if (sFacade.expired()){
		EngineFacadePtr p(new EngineFacade, [](EngineFacade* obj){ delete obj; });
		sFacade = p;
		sFacadeRaw = p.get();
		p->mImpl->mSelfPtr = p;
		return p;
	}
	return sFacade.lock();
}

EngineFacade& EngineFacade::GetInstance(){
	if (sFacade.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "EngineFacade is already deleted. Program will crash.");
	}
	return *sFacadeRaw;
}

bool EngineFacade::HasInstance(){
	return !sFacade.expired();
}

EngineFacade::EngineFacade()
	:mImpl(new Impl)
{

}
EngineFacade::~EngineFacade(){

}

HWindowId EngineFacade::CreateEngineWindow(int x, int y, int width, int height,
	const char* wndClass, const char* title, unsigned style, unsigned exStyle,
	WNDPROC winProc){
	return mImpl->CreateEngineWindow(x, y, width, height, wndClass, title, style, exStyle, winProc);
}

void EngineFacade::DestroyEngineWindow(HWindowId windowId){
	mImpl->DestroyEngineWindow(windowId);
}

HWindowId EngineFacade::GetMainWindowHandleId() const{
	return mImpl->GetMainWindowHandleId();
}

HWindow EngineFacade::GetMainWindowHandle() const{
	return mImpl->GetMainWindowHandle();
}

HWindow EngineFacade::GetWindowHandleById(HWindowId hwndId) const{
	return mImpl->GetWindowHandleById(hwndId);
}

bool EngineFacade::InitRenderer(const char* pluginName) {
	return mImpl->InitRenderer(pluginName);
}

bool EngineFacade::InitCanvas(HWindowId id, int width, int height) {
	return mImpl->InitCanvas(id, width, height);
}

bool EngineFacade::InitCanvas(HWindow hwnd){
	return mImpl->InitCanvas(hwnd);
}

void EngineFacade::SetClearColor(const Color& color){
	Renderer::GetInstance().SetClearColor(GetMainWindowHandleId(), color);
}

ScenePtr EngineFacade::GetMainScene() const {
	return mImpl->GetMainScene();
}

ScenePtr EngineFacade::GetCurrentScene() const{
	return mImpl->GetCurrentScene();
}

void EngineFacade::AddSceneObserver(ISceneObserver::Type type, ISceneObserverPtr observer){
	mImpl->mMainScene->AddSceneObserver(type, observer);
}

void EngineFacade::RemoveSceneObserver(ISceneObserver::Type type, ISceneObserverPtr observer){
	mImpl->mMainScene->RemoveObserver(type, observer);
}

ScenePtr EngineFacade::CreateScene(const char* uniquename){
	return SceneManager::GetInstance().CreateScene(uniquename);
}

void EngineFacade::OverrideMainScene(IScenePtr scene){
	mImpl->OverrideMainScene(scene);
}

void EngineFacade::OverrideMainScene(){
	mImpl->OverrideMainScene();
}

void EngineFacade::LockSceneOverriding(bool lock){
	mImpl->mLockSceneOverriding = lock;
}

void EngineFacade::SetDrawClouds(bool draw){
	mImpl->SetDrawClouds(draw);
}

void EngineFacade::UpdateFileMonitor(){
	if (mImpl->mFileMonitor)
		mImpl->mFileMonitor->Check();
}

void EngineFacade::UpdateInput(){
	mImpl->UpdateInput();
}

void EngineFacade::Update(TIME_PRECISION dt) {
	mImpl->Update(dt);
}

void EngineFacade::Render() {
	mImpl->Render();
}

EngineOptionsPtr EngineFacade::GetEngineOptions() const{
	return mImpl->mEngineOptions;
}

bool EngineFacade::MainCameraExists() const{
	return mImpl->mMainCamera != 0;
}

CameraPtr EngineFacade::GetMainCamera() const{
	return mImpl->mMainCamera;
}

Real EngineFacade::GetMainCameraAspectRatio() const{
	return mImpl->mMainCamera->GetAspectRatio();
}

Real EngineFacade::GetMainCameraFov() const{
	return mImpl->mMainCamera->GetFOV();
}

const Vec3& EngineFacade::GetMainCameraPos() const{
	return mImpl->mMainCamera->GetPosition();
}

void EngineFacade::SetMainCameraPos(const Vec3& pos){
	mImpl->mMainCamera->SetPosition(pos);
}

const Vec3& EngineFacade::GetMainCameraDirection() const{
	return mImpl->mMainCamera->GetDirection();
}

const Mat44& EngineFacade::GetCameraMatrix(ICamera::MatrixType type) const{
	return mImpl->mMainCamera->GetMatrix(type);
}

void EngineFacade::SetMainCameraTarget(ISpatialObjectPtr spatialObject){
	mImpl->mMainCamera->SetTarget(spatialObject);
}

void EngineFacade::EnableCameraInput(bool enable){
	mImpl->mMainCamera->SetEnalbeInput(enable);
}

const Ray3& EngineFacade::GetWorldRayFromCursor(){
	return mImpl->GetWorldRayFromCursor();
}

IInputInjectorPtr EngineFacade::GetInputInjector(){
	return InputManager::GetInstance().GetInputInjector();
}

void EngineFacade::AddDirectionalLightCoordinates(DirectionalLightIndex::Enum idx, Real phi, Real theta){
	mImpl->AddDirectionalLightCoordinates(idx, phi, theta);
}

const Vec3& EngineFacade::GetLightDirection(DirectionalLightIndex::Enum idx){
	return mImpl->GetLightDirection(idx);
}

const Vec3& EngineFacade::GetLightDiffuse(DirectionalLightIndex::Enum idx){
	return mImpl->GetLightDiffuse(idx);
}
const Real EngineFacade::GetLightIntensity(DirectionalLightIndex::Enum idx){
	return mImpl->GetLightIntensity(idx);
}

void EngineFacade::SetLightIntensity(IScenePtr scene, DirectionalLightIndex::Enum idx, Real intensity){
	mImpl->SetLightIntensity(scene, idx, intensity);
}

DirectionalLightPtr EngineFacade::GetMainSceneLight(DirectionalLightIndex::Enum idx){
	return mImpl->mMainScene->GetDirectionalLight(idx);
}


void EngineFacade::DetachBlendingSky(IScenePtr iscene){
	auto scene = std::static_pointer_cast<Scene>(iscene);
	auto sky = std::dynamic_pointer_cast<SkySphere>(scene->GetSky());
	if (sky){
		sky->DetachBlendingSky();
	}
}

void EngineFacade::DrawProfileResult(const ProfilerSimple& profiler, const char* posVarName){
	DrawProfileResult(profiler, posVarName, 0);
}

void EngineFacade::DrawProfileResult(const ProfilerSimple& profiler, const char* posVarName, int tab){
	char buf[256];
	std::string tapString;
	while (tab--)
	{
		tapString.push_back('\t');
	}
	sprintf_s(buf, "%s%s : %f", tapString.c_str(), profiler.GetName(), profiler.GetDT());
	Vec2I pos = LuaUtils::GetLuaVarAsVec2I(posVarName);
	Renderer::GetInstance().QueueDrawText(pos, buf, Color::White);
}

void EngineFacade::DrawProfileResult(wchar_t* buf, const char* posVarName){
	DrawProfileResult(buf, posVarName, 0);
}

void EngineFacade::DrawProfileResult(wchar_t* buf, const char* posVarName, int tab){
	Vec2I pos = LuaUtils::GetLuaVarAsVec2I(posVarName);
	Renderer::GetInstance().QueueDrawText(pos, buf, Color::White);
}

void* EngineFacade::MapMaterialParameterBuffer(){
	return Renderer::GetInstance().MapMaterialParameterBuffer();
}

void EngineFacade::UnmapMaterialParameterBuffer(){
	Renderer::GetInstance().UnmapMaterialParameterBuffer();
}

void* EngineFacade::MapBigBuffer(){
	return Renderer::GetInstance().MapBigBuffer();
}

void EngineFacade::UnmapBigBuffer(){
	return Renderer::GetInstance().UnmapBigBuffer();
}

void EngineFacade::SetEnable3DUIs(bool enable){
	Logger::Log(FB_DEFAULT_LOG_ARG, "3D UI featrue is not available.");
}

void EngineFacade::SetRendererFadeAlpha(Real alpha){
	Renderer::GetInstance().SetFadeAlpha(alpha);
}

void EngineFacade::ClearDurationTexts(){
	Renderer::GetInstance().ClearDurationTexts();
}

bool EngineFacade::GetResolutionList(unsigned& outNum, Vec2I* list){
	return Renderer::GetInstance().GetResolutionList(outNum, list);
}

Vec2 EngineFacade::ToNdcPos(HWindowId id, const Vec2I& screenPos) const{
	return Renderer::GetInstance().ToNdcPos(id, screenPos);
}

void EngineFacade::SetFontTextureAtlas(const char* path){
	Renderer::GetInstance().SetFontTextureAtlas(path);
}

void EngineFacade::SetEnvironmentMap(const char* path){
	auto& renderer = Renderer::GetInstance();
	auto texture = renderer.CreateTexture(path);
	Renderer::GetInstance().SetEnvironmentTexture(texture);
}

intptr_t EngineFacade::WinProc(HWindow window, unsigned msg, uintptr_t wp, uintptr_t lp){
#if defined(_PLATFORM_WINDOWS_)
	static HCURSOR sArrowCursor = LoadCursor(0, IDC_ARROW);
	switch (msg)
	{
	case WM_PAINT:
	{
		auto hwndId = Renderer::GetInstance().GetWindowHandleId(window);
		auto rt = Renderer::GetInstance().GetRenderTarget(hwndId);
		if (rt)
		{
			rt->TriggerDrawEvent();
		}
	}
	break;


	case WM_INPUT:
	{
		UINT dwSize = 40;
		static BYTE lpb[40];

		GetRawInputData((HRAWINPUT)lp, RID_INPUT,
			lpb, &dwSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)lpb;
		if (InputManager::HasInstance()){
			switch (raw->header.dwType)
			{
			case RIM_TYPEMOUSE:
			{
				MouseEvent* evt = (MouseEvent*)&raw->data.mouse;	
				InputManager::GetInstance().PushMouseEvent(window, *(evt), gpTimer->GetTime());
			}
			return 0;
			case RIM_TYPEKEYBOARD:
			{
				InputManager::GetInstance().PushKeyEvent(window, *((KeyboardEvent*)&raw->data.keyboard));
			}
			return 0;
			}
		}
	}
	break;

	case WM_CHAR:
	{
		if (InputManager::HasInstance())
		{
			InputManager::GetInstance().PushChar(window, wp, gpTimer->GetTime());
		}
	}
	return 0; // processed

	case WM_SETFOCUS:
	{
		SetCursor(sArrowCursor);
		if (InputManager::HasInstance())
		{
			InputManager::GetInstance().OnSetFocus(window);
		}		
	}
	return 0;

	case WM_KILLFOCUS:
	{
		if (InputManager::HasInstance()){
			InputManager::GetInstance().OnKillFocus();
		}		
	}
	return 0;

	case WM_SIZE:
	{
		RECT rc;
		GetClientRect((HWND)window, &rc);
		auto width = rc.right - rc.left;
		auto height = rc.bottom - rc.top;
		auto hwndId = Renderer::GetInstance().GetWindowHandleId(window);
		if (hwndId != INVALID_HWND_ID && Renderer::HasInstance()){
			Renderer::GetInstance().OnWindowSizeChanged(window, Vec2I(width, height));
		}
		return 0;
	}

	case WM_SIZING:
	{
		if (Renderer::HasInstance()){
			RECT* pRect = (RECT*)lp;
			RECT prevRect = *pRect;
			auto hwndId = Renderer::GetInstance().GetWindowHandleId(window);
			Vec2I size = Renderer::GetInstance().FindClosestSize(hwndId, Vec2I(prevRect.right - prevRect.left, prevRect.bottom - prevRect.top));
			pRect->right = pRect->left + size.x;
			pRect->bottom = pRect->top + size.y;
			AdjustWindowRect(pRect, GetWindowLongPtr((HWND)window, GWL_STYLE), FALSE);
			RECT adjustedRect = *pRect;
			unsigned eWidth = adjustedRect.right - adjustedRect.left;
			unsigned eHeight = adjustedRect.bottom - adjustedRect.top;

			if (wp == WMSZ_RIGHT || wp == WMSZ_BOTTOMRIGHT || wp == WMSZ_TOPRIGHT || wp == WMSZ_TOP || wp == WMSZ_BOTTOM){
				pRect->left = prevRect.left;
				pRect->right = prevRect.left + eWidth;
			}
			else{
				pRect->right = prevRect.right;
				pRect->left = prevRect.right - eWidth;
			}


			if (wp == WMSZ_BOTTOM || wp == WMSZ_BOTTOMLEFT || wp == WMSZ_BOTTOMRIGHT || wp == WMSZ_RIGHT || wp == WMSZ_LEFT){
				pRect->top = prevRect.top;
				pRect->bottom = prevRect.top + eHeight;
			}
			else{
				pRect->bottom = prevRect.bottom;
				pRect->top = pRect->bottom - eHeight;
			}


			return 1;
		}
		break;
	}

	/*	case WM_ACTIVATE:
	{
	if (WA_ACTIVE==LOWORD(wp))
	{
	BringWindowToTop(mWindowHandles);
	}
	}
	return 0; // processed
	*/
	}


	return DefWindowProc((HWND)window, msg, wp, lp); // not processed
#endif
}

void EngineFacade::RegisterInputConsumer(IInputConsumerPtr consumer, int priority){
	InputManager::GetInstance().RegisterInputConsumer(consumer, priority);
}

void EngineFacade::AddRendererObserver(int rendererObserverType, IRendererObserverPtr observer){
	Renderer::GetInstance().AddObserver(rendererObserverType, observer);
}

void EngineFacade::AddFileChangeObserver(int fileChangeObserverType, IFileChangeObserverPtr observer){
	FileMonitor::GetInstance().AddObserver(fileChangeObserverType, observer);
}

RenderTargetPtr EngineFacade::GetMainRenderTarget() const{
	return Renderer::GetInstance().GetMainRenderTarget();
}

const Vec2I& EngineFacade::GetMainRenderTargetSize() const{
	return Renderer::GetInstance().GetMainRenderTargetSize();
}

IVideoPlayerPtr EngineFacade::CreateVideoPlayer(VideoPlayerType::Enum type){
	switch (type){
	case VideoPlayerType::OGG:
		return VideoPlayerOgg::Create();
	}
	Logger::Log(FB_ERROR_LOG_ARG, "Not implemented type.");
	return 0;
}

void EngineFacade::QueueDrawTextForDuration(float secs, const Vec2I& pos, const char* text, const Color& color){
	QueueDrawTextForDuration(secs, pos, text, color, 20.f);
}

void EngineFacade::QueueDrawTextForDuration(float secs, const Vec2I& pos, const char* text, const Color& color, float size){
	if (Renderer::HasInstance())
		Renderer::GetInstance().QueueDrawTextForDuration(secs, pos, text, color, size);
}

void EngineFacade::QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color){
	Renderer::GetInstance().QueueDrawText(pos, text, color);
}

void EngineFacade::QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size){
	Renderer::GetInstance().QueueDrawText(pos, text, color, size);
}

void EngineFacade::QueueDrawText(const Vec2I& pos, const char* text, const Color& color){
	Renderer::GetInstance().QueueDrawText(pos, text, color);
}

void EngineFacade::QueueDrawText(const Vec2I& pos, const char* text, const Color& color, Real size){
	Renderer::GetInstance().QueueDrawText(pos, text, color, size);
}

void EngineFacade::QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color){
	Renderer::GetInstance().QueueDraw3DText(worldpos, text, color);
}

void EngineFacade::QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, Real size){
	Renderer::GetInstance().QueueDraw3DText(worldpos, text, color, size);
}

void EngineFacade::QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color){
	Renderer::GetInstance().QueueDraw3DText(worldpos, text, color);
}

void EngineFacade::QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color, Real size){
	Renderer::GetInstance().QueueDraw3DText(worldpos, text, color, size);
}

void EngineFacade::QueueDrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
	const Color& color0, const Color& color1){
	Renderer::GetInstance().QueueDrawLineBeforeAlphaPass(start, end, color0, color1);
}

void EngineFacade::QueueDrawLine(const Vec3& start, const Vec3& end,
	const Color& color0, const Color& color1){
	Renderer::GetInstance().QueueDrawLine(start, end, color0, color1);
}

void EngineFacade::QueueDrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
	const char* texture, bool textureFlow) {
	Renderer::GetInstance().QueueDrawTexturedThickLine(start, end, color0, color1, thickness, texture, textureFlow);
}

void EngineFacade::QueueDrawSphere(const Vec3& pos, Real radius, const Color& color){
	Renderer::GetInstance().QueueDrawSphere(pos, radius, color);
}

void EngineFacade::QueueDrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha){
	Renderer::GetInstance().QueueDrawBox(boxMin, boxMax, color, alpha);
}

void EngineFacade::QueueDrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha){
	Renderer::GetInstance().QueueDrawTriangle(a, b, c, color, alpha);
}

FontPtr EngineFacade::GetFont(float fontHeight){
	if (Renderer::HasInstance())
		return Renderer::GetInstance().GetFont(fontHeight);
	Logger::Log(FB_ERROR_LOG_ARG, "Renderer is deleted.");
	return 0;
}

unsigned EngineFacade::GetNumLoadingTexture() const{
	if (Renderer::HasInstance())
		return Renderer::GetInstance().GetNumLoadingTexture();
	return 0;
}

RenderTargetPtr EngineFacade::CreateRenderTarget(const RenderTargetParamEx& param){
	if (Renderer::HasInstance()){
		RenderTargetParam renderParam;
		renderParam.mSize = param.mSize;
		renderParam.mPixelFormat = param.mPixelFormat;
		renderParam.mEveryFrame = param.mEveryFrame;
		renderParam.mShaderResourceView = param.mShaderResourceView;
		renderParam.mMipmap = param.mMipmap;
		renderParam.mCubemap = param.mCubemap;
		renderParam.mWillCreateDepth = param.mWillCreateDepth;
		renderParam.mUsePool = param.mUsePool;
		auto rt = Renderer::GetInstance().CreateRenderTarget(renderParam);
		if (!param.mSceneNameToCreateAndOwn.empty()){
			auto scene = SceneManager::GetInstance().CreateScene(param.mSceneNameToCreateAndOwn.c_str());
			rt->TakeOwnershipScene(scene);
		}
		if (!param.mEnvironmentTexture.empty()){
			rt->SetEnvTexture(Renderer::GetInstance().CreateTexture(param.mEnvironmentTexture.c_str()));
		}
		return rt;
	}
	return 0;
}

VoxelizerPtr EngineFacade::CreateVoxelizer(){
	return Voxelizer::Create();
}

void EngineFacade::AddTempMesh(MeshFacadePtr mesh){
	mImpl->AddTempMesh(mesh);
}

void EngineFacade::GetFractureMeshObjects(const char* daeFilePath, std::vector<MeshFacadePtr>& objects){
	mImpl->GetFractureMeshObjects(daeFilePath, objects);
}

std::wstring EngineFacade::StripTextTags(const char* text){
	auto font = GetFont(22.f);
	if (font){
		return font->StripTags(fb::AnsiToWide(text));		
	}
	else{
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("No font found.").c_str());
		return std::wstring(fb::AnsiToWide(text));
	}
}

void EngineFacade::QueueProcessConsoleCommand(const char* command, bool history){
	Console::GetInstance().QueueProcessCommand(command, history);
}

void EngineFacade::OnChangeDetected(){

}

bool EngineFacade::OnFileChanged(const char* watchDir, const char* filepath, const char* loweredExtension){
	return false;
}

void EngineFacade::StopAllParticles(){
	mImpl->mParticleSystem->StopParticles();
}

void EngineFacade::AddTask(TaskPtr NewTask){
	mImpl->mTaskSchedular->AddTask(NewTask);
}

AudioId EngineFacade::PlayAudio(const char* filepath){
	return mImpl->PlayAudio(filepath);
}

AudioId EngineFacade::PlayAudio(const char* filepath, const Vec3& pos){
	return mImpl->PlayAudio(filepath, pos);
}

AudioId EngineFacade::PlayAudio(const char* filepath, const AudioProperty& prop){
	return mImpl->PlayAudio(filepath, prop);
}

bool EngineFacade::SetAudioPosition(AudioId id, const Vec3& pos){
	return mImpl->SetAudioPosition(id, pos);
}

void EngineFacade::SetListenerPosition(const Vec3& pos){
	return mImpl->SetListenerPosition(pos);
}