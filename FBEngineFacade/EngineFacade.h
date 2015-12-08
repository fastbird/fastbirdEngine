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

#pragma once
#include "FBCommonHeaders/platform.h"
#include "FBCommonHeaders/Types.h"
#include "FBFileMonitor/IFileChangeObserver.h"
#include "FBSceneManager/DirectionalLightIndex.h"
#include "FBSceneManager/ISceneObserver.h"
#include "FBVideoPlayer/VideoPlayerType.h"
#include "FBAudioPlayer/AudioProperty.h"
#include "FBRenderer/ICamera.h"
#include "RenderTargetParamEx.h"
namespace fb{
	class ProfilerSimple;
	class LuaObject;
	class Color;
	class Ray3;
	FB_DECLARE_SMART_PTR(ISpatialObject);
	FB_DECLARE_SMART_PTR(MeshFacade);
	FB_DECLARE_SMART_PTR(DirectionalLight);
	FB_DECLARE_SMART_PTR(Task);
	FB_DECLARE_SMART_PTR(Voxelizer);
	FB_DECLARE_SMART_PTR(Font);
	FB_DECLARE_SMART_PTR(IVideoPlayer);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(IFileChangeObserver);
	FB_DECLARE_SMART_PTR(IRendererObserver);
	FB_DECLARE_SMART_PTR(IInputInjector);
	FB_DECLARE_SMART_PTR(IInputConsumer);
	FB_DECLARE_SMART_PTR(EngineOptions);
	FB_DECLARE_SMART_PTR(Scene);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(EngineFacade);
	FB_DECLARE_SMART_PTR(RenderTarget);
	class FB_DLL_ENGINEFACADE EngineFacade : public IFileChangeObserver{
		FB_DECLARE_PIMPL_NON_COPYABLE(EngineFacade);
		EngineFacade();
		~EngineFacade();

	public:		
		static EngineFacadePtr Create();
		static EngineFacade& GetInstance();
		static bool HasInstance();

		static const HWindowId INVALID_HWND_ID = (HWindowId)-1;
		
		//---------------------------------------------------------------------------
		// Engine Facade
		//---------------------------------------------------------------------------
		HWindowId CreateEngineWindow(int x, int y, int width, int height,
			const char* wndClass, const char* title, unsigned style, unsigned exStyle,
			WNDPROC winProc);
		void DestroyEngineWindow(HWindowId windowId);
		HWindowId GetMainWindowHandleId() const;
		HWindow GetMainWindowHandle() const;
		HWindow GetWindowHandleById(HWindowId hwndId) const;
		/// for windows;
		intptr_t WinProc(HWindow window, unsigned msg, uintptr_t wp, uintptr_t lp);
		EngineOptionsPtr GetEngineOptions() const;
		void UpdateFileMonitor();
		void UpdateInput();
		void Update(TIME_PRECISION dt);
		void Render();
		IInputInjectorPtr GetInputInjector();
		/** Register an input consumer.
		You need unregister when the consumer is destroyed or does not
		need to getinput information any more.
		\param consumer
		\param priority number one priority is handled first.
		i.e. the lowest value is handled first. Check the default
		priority at IInputConsumer::Priority
		*/
		void RegisterInputConsumer(IInputConsumerPtr consumer, int priority);
		void AddFileChangeObserver(int fileChangeObserverType, IFileChangeObserverPtr observer);
		IVideoPlayerPtr CreateVideoPlayer(VideoPlayerType::Enum type);
		VoxelizerPtr CreateVoxelizer();
		/// Keep the strong ptr.
		void AddTempMesh(MeshFacadePtr mesh);
		void GetFractureMeshObjects(const char* daeFilePath, std::vector<MeshFacadePtr>& objects);		
		std::wstring StripTextTags(const char* text);		
		void QueueProcessConsoleCommand(const char* command, bool history = true);
		// IFileChangeObserver
		void OnChangeDetected();
		bool OnFileChanged(const char* watchDir, const char* filepath, const char* loweredExtension);

		//---------------------------------------------------------------------------
		// Renderer Interfaces
		//---------------------------------------------------------------------------
		/// \param pluginName "FBRendererD3D11" is provided.
		bool InitRenderer(const char* pluginName);
		bool InitCanvas(HWindowId id, int width, int height);
		/// Use this function if you didn't create a window with EngienFacade.
		bool InitCanvas(HWindow hwnd);
		void SetClearColor(const Color& color);
		void AddRendererObserver(int rendererObserverType, IRendererObserverPtr observer);
		RenderTargetPtr GetMainRenderTarget() const;
		const Vec2I& GetMainRenderTargetSize() const;
		bool MainCameraExists() const;
		void QueueDrawTextForDuration(float secs, const Vec2I& pos, const char* text, const Color& color);
		void QueueDrawTextForDuration(float secs, const Vec2I& pos, const char* text, const Color& color, float size);
		void QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color);
		void QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size);
		void QueueDrawText(const Vec2I& pos, const char* text, const Color& color);
		void QueueDrawText(const Vec2I& pos, const char* text, const Color& color, Real size);
		void QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color);
		void QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, Real size);
		void QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color);
		void QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color, Real size);
		void QueueDrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
			const Color& color0, const Color& color1);
		void QueueDrawLine(const Vec3& start, const Vec3& end,
			const Color& color0, const Color& color1);
		/**Rendered before the transparent object.*/
		void QueueDrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
			const char* texture, bool textureFlow);
		/**Rendered before the transparent object.*/
		void QueueDrawSphere(const Vec3& pos, Real radius, const Color& color);
		/**Rendered before the transparent object.*/
		void QueueDrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha);
		/**Rendered before the transparent object.*/
		void QueueDrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha);

		FontPtr GetFont(float fontHeight);
		unsigned GetNumLoadingTexture() const;
		RenderTargetPtr CreateRenderTarget(const RenderTargetParamEx& param);
		CameraPtr GetMainCamera() const;
		Real GetMainCameraAspectRatio() const;
		Real GetMainCameraFov() const;
		const Vec3& GetMainCameraPos() const;
		void SetMainCameraPos(const Vec3& pos);
		const Vec3& GetMainCameraDirection() const;
		/// Get matrices of the main camera
		const Mat44& GetCameraMatrix(ICamera::MatrixType type) const;
		void SetMainCameraTarget(ISpatialObjectPtr spatialObject);
		// enable internal input mechanism for the main camera
		void EnableCameraInput(bool enable);
		const Ray3& GetWorldRayFromCursor();
		void DrawProfileResult(const ProfilerSimple& profiler, const char* posVarName);
		void DrawProfileResult(const ProfilerSimple& profiler, const char* posVarName, int tab);
		void DrawProfileResult(wchar_t* buf, const char* posVarName);
		void DrawProfileResult(wchar_t* buf, const char* posVarName, int tab);
		void* MapMaterialParameterBuffer();
		void UnmapMaterialParameterBuffer();
		void* MapBigBuffer();
		void UnmapBigBuffer();
		void SetEnable3DUIs(bool enable);
		void SetRendererFadeAlpha(Real alpha);
		void ClearDurationTexts();
		bool GetResolutionList(unsigned& outNum, Vec2I* list);
		Vec2 ToNdcPos(HWindowId id, const Vec2I& screenPos) const;
		void SetFontTextureAtlas(const char* path);
		void SetEnvironmentMap(const char* path);

		//---------------------------------------------------------------------------
		// Scene Manipulations
		//---------------------------------------------------------------------------
		/// returning scene attache to the main render target.
		ScenePtr GetMainScene() const;
		ScenePtr GetCurrentScene() const;
		/// Add a scene observer to the main scene
		/// \param type ISceneObserver::
		void AddSceneObserver(ISceneObserver::Type type, ISceneObserverPtr observer);
		/// Remove a scene observer from the main scene
		void RemoveSceneObserver(ISceneObserver::Type type, ISceneObserverPtr observer);
		ScenePtr CreateScene(const char* uniquename);
		void OverrideMainScene(IScenePtr scene);
		/// Create temporary scene, lasting until OverrideMainScene(0) called.
		void OverrideMainScene();
		void LockSceneOverriding(bool lock);
		void SetDrawClouds(bool draw);				
		void AddDirectionalLightCoordinates(DirectionalLightIndex::Enum idx, Real phi, Real theta);
		const Vec3& GetLightDirection(DirectionalLightIndex::Enum idx);
		const Vec3& GetLightDiffuse(DirectionalLightIndex::Enum idx);
		const Real GetLightIntensity(DirectionalLightIndex::Enum idx);
		void SetLightIntensity(IScenePtr scene, DirectionalLightIndex::Enum idx, Real intensity);
		DirectionalLightPtr GetMainSceneLight(DirectionalLightIndex::Enum idx);
		void DetachBlendingSky(IScenePtr scene);
		void StopAllParticles();
		void AddTask(TaskPtr NewTask);

		//---------------------------------------------------------------------------
		// Audio
		//---------------------------------------------------------------------------
		AudioId PlayAudio(const char* filepath);
		AudioId PlayAudio(const char* filepath, const Vec3& pos);
		AudioId PlayAudio(const char* filepath, const AudioProperty& prop);
		bool SetAudioPosition(AudioId id, const Vec3& pos);
		void SetListenerPosition(const Vec3& pos);
	};
}