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
#include "FBCommonHeaders/Observable.h"
#include "IRendererObserver.h"
#include "RendererEnums.h"
#include "RendererStructs.h"
#include "ShaderDefines.h"
#include "InputElementDesc.h"
#include "RenderTargetParam.h"
#include "InputLayout.h"
#include "IPlatformIndexBuffer.h"
#include "RenderStates.h"
#include "TextureBinding.h"
#include "SystemTextures.h"
#include "RenderEventMarker.h"
#include "FBSceneManager/ISceneObserver.h"
#include "FBSceneManager/IScene.h"
#include "FBInputManager/IInputConsumer.h"
#include "FBMathLib/Math.h"
struct lua_State;
namespace fb{	
	struct POINT_LIGHT_CONSTANTS;	
	typedef unsigned RenderTargetId;
	FB_DECLARE_SMART_PTR(ResourceProvider);
	FB_DECLARE_SMART_PTR(IVideoPlayer);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(PointLightManager);
	FB_DECLARE_SMART_PTR(Font);
	FB_DECLARE_SMART_PTR(DirectionalLight);
	FB_DECLARE_SMART_PTR(PointLight);	
	FB_DECLARE_SMART_PTR_STRUCT(TextureAtlasRegion);
	FB_DECLARE_SMART_PTR(TextureAtlas);	
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(Shader);
	FB_DECLARE_SMART_PTR(IndexBuffer);
	FB_DECLARE_SMART_PTR(VertexBuffer);
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(RenderTarget);
	FB_DECLARE_SMART_PTR(RendererOptions);
	FB_DECLARE_SMART_PTR(Renderer);
	/** Render vertices with a specified material	
	Rednerer handles vertex/index data, materials, textures, shaders,
	render states, lights and render targets.
	\ingroup FBRenderer
	*/
	class FB_DLL_RENDERER Renderer : public Observable<IRendererObserver>, public ISceneObserver, public IInputConsumer{
		FB_DECLARE_PIMPL_NON_COPYABLE(Renderer);
		Renderer();		

	public:
		/** Create an empty Renderer. 
		You have the owner ship of the pointer.
		*/
		static RendererPtr Create();

		/** Create a Renderer with the platform specific render engine you specified in \a rendererPlugInName 
		\param rendererPlugInName The renderer plug-in name. You can specify "FBRendererD3D11" for
		the default renderer on Windows, and "FBRendererGL41" for Mac. \n
		\param L Renderer need the lua_State* to read the configuration file. If you pass NULL, all setting will be default.
		*/
		static RendererPtr Create(const char* rendererPlugInName);

		/** Returns the Renderer as a reference.
		This function does not check the validity whether the Renderer is created or not.
		It will cause a crash if you call this function without calling Renderer::Create()
		If you need to check the validity use Renderer::HasInstance() function.
		*/
		static Renderer& GetInstance();

		/** Checks whether the renderer does exists or not.
		*/
		static bool HasInstance();

		/** Used for only in the Renderer project.
		*/
		static RendererPtr GetInstancePtr();

		
	public:
		~Renderer();
		static const HWindow INVALID_HWND;		

		/** Prepare the platform specific render engine
		You don't need to call this function if you create Renderer with the following function.
		 \code 
		 static RendererPtr Create(const char* rendererPlugInName); 
		 \endcode
		 because the Renderer you got already have the render engine plugged in.
		*/
		bool PrepareRenderEngine(const char* rendererPlugInName);

		//-------------------------------------------------------------------
		// Canvas & System
		//-------------------------------------------------------------------
		bool InitCanvas(HWindowId id, HWindow window, int width, int height);
		void ReleaseCanvas(HWindowId id);
		void Render();
		void Update(TIME_PRECISION dt);

		//-------------------------------------------------------------------
		// Resource Creation
		//-------------------------------------------------------------------
		RenderTargetPtr CreateRenderTarget(const RenderTargetParam& param);
		/** After the render target is kept in pool, you should stop using it.
		If you need to get it again use Renderer::CreateRenderTarget() again with
		setting mUsePool in param as true.*/
		void KeepRenderTargetInPool(RenderTargetPtr rt);
		/** Load texture file asynchronously */
		TexturePtr CreateTexture(const char* file);
		TexturePtr CreateTexture(const char* file, bool async);		
		TexturePtr CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
			BUFFER_USAGE usage, int  buffer_cpu_access, int texture_type);
		VertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
			unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag);
		IndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
			INDEXBUFFER_FORMAT format);
		ShaderPtr CreateShader(const char* filepath, int shaders);
		ShaderPtr CreateShader(const char* filepath, int shaders, const SHADER_DEFINES& defines);
		bool ReapplyShaderDefines(Shader* shader);
		MaterialPtr CreateMaterial(const char* file);		
		// use this if you are sure there is instance of the descs.
		InputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs, ShaderPtr shader);
		InputLayoutPtr GetInputLayout(DEFAULT_INPUTS::Enum e, ShaderPtr shader);
		RasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc);
		BlendStatePtr CreateBlendState(const BLEND_DESC& desc);
		DepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		SamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc);		
		TextureAtlasPtr GetTextureAtlas(const char* path);		
		TextureAtlasRegionPtr GetTextureAtlasRegion(const char* path, const char* region);
		TexturePtr GetTemporalDepthBuffer(const Vec2I& size);
		PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime,
			bool manualDeletion);

		//-------------------------------------------------------------------
		// Hot reloading
		//-------------------------------------------------------------------
		/*bool ReloadShader(ShaderPtr shader);
		bool ReloadTexture(ShaderPtr shader);*/

		//-------------------------------------------------------------------
		// Resource Bindings
		//-------------------------------------------------------------------
		void SetRenderTarget(TexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
			TexturePtr pDepthStencil, size_t dsViewIndex);		
		void UnbindRenderTarget(TexturePtr renderTargetTexture);
		void SetViewports(const Viewport viewports[], int num);
		void SetScissorRects(Rect rects[], int num);
		void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
			VertexBufferPtr pVertexBuffers[], unsigned int strides[], unsigned int offsets[]);
		void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt);		
		/** Bind serveral texture at once.
		The texture order apears in the shader(BINDING_SHADER) should be sequential.
		*/
		void SetTextures(TexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot);
		void SetSystemTexture(SystemTextures::Enum type, TexturePtr texture);
		void UnbindTexture(BINDING_SHADER shader, int slot);
		void UnbindInputLayout();
		void UnbindVertexBuffers();
		void UnbindShader(BINDING_SHADER shader);
		// pre defined
		void BindDepthTexture(bool set);		
		void SetDepthWriteShader();
		void SetDepthWriteShaderCloud();		
		void SetPositionInputLayout();

		void SetSystemTextureBindings(SystemTextures::Enum type, const TextureBindings& bindings);
		const TextureBindings& GetSystemTextureBindings(SystemTextures::Enum type) const;				

		//-------------------------------------------------------------------
		// Device RenderStates
		//-------------------------------------------------------------------
		void RestoreRenderStates();
		void RestoreRasterizerState();
		void RestoreBlendState();
		void RestoreDepthStencilState();				
		// sampler
		void SetSamplerState(int ResourceTypes_SamplerStates, BINDING_SHADER shader, int slot);

		//-------------------------------------------------------------------
		// GPU constants
		//-------------------------------------------------------------------
		/// record == false
		void UpdateObjectConstantsBuffer(const void* pData);
		void UpdateObjectConstantsBuffer(const void* pData, bool record);
		void UpdatePointLightConstantsBuffer(const void* pData);
		void UpdateFrameConstantsBuffer();
		void UpdateMaterialConstantsBuffer(const void* pData);
		void UpdateCameraConstantsBuffer();
		void UpdateRenderTargetConstantsBuffer();
		void UpdateSceneConstantsBuffer();
		void UpdateRareConstantsBuffer();
		void UpdateRadConstantsBuffer(const void* pData);
		void* MapMaterialParameterBuffer();
		void UnmapMaterialParameterBuffer();
		void* MapBigBuffer();
		void UnmapBigBuffer();

		//-------------------------------------------------------------------
		// GPU Manipulation
		//-------------------------------------------------------------------
		void SetClearColor(HWindowId id, const Color& color);
		void SetClearDepthStencil(HWindowId id, Real z, UINT8 stencil);
		void Clear(Real r, Real g, Real b, Real a, Real z, UINT8 stencil);
		void Clear(Real r, Real g, Real b, Real a);
		// Avoid to use
		void ClearState();
		void BeginEvent(const char* name);
		void EndEvent();
		void TakeScreenshot();
		void ChangeFullscreenMode(int mode);
		void OnWindowSizeChanged(HWindow window, const Vec2I& clientSize);
		void ChangeResolution(const Vec2I& resol);
		void ChangeResolution(HWindow window, const Vec2I& resol);		
		void ChangeWindowSizeAndResolution(const Vec2I& resol);
		void ChangeWindowSizeAndResolution(HWindow window, const Vec2I& resol);
		

		//-------------------------------------------------------------------
		// FBRenderer State
		//-------------------------------------------------------------------
		ResourceProviderPtr GetResourceProvider() const;
		/// \param provider cannot be null
		void SetResourceProvider(ResourceProviderPtr provider);
		void SetForcedWireFrame(bool enable);
		bool GetForcedWireFrame() const;
		RenderTargetPtr GetMainRenderTarget() const;
		IScenePtr GetMainScene() const; // move to SceneManager
		const Vec2I& GetMainRenderTargetSize() const;
		void SetCurrentRenderTarget(RenderTargetPtr renderTarget);
		RenderTargetPtr GetCurrentRenderTarget() const;
		void SetCurrentScene(IScenePtr scene);
		bool IsMainRenderTarget() const;
		const Vec2I& GetRenderTargetSize(HWindowId id = INVALID_HWND_ID) const;
		const Vec2I& GetRenderTargetSize(HWindow hwnd = 0) const;
		void SetDirectionalLightInfo(int idx, const DirectionalLightInfo& info);		
		void InitFrameProfiler(Real dt);
		const RENDERER_FRAME_PROFILER& GetFrameProfiler() const;
		inline FontPtr GetFont(Real fontHeight) const;
		const INPUT_ELEMENT_DESCS& GetInputElementDesc(
			DEFAULT_INPUTS::Enum e);
		void SetEnvironmentTexture(TexturePtr pTexture);
		void SetEnvironmentTextureOverride(TexturePtr texture);		
		void SetDebugRenderTarget(unsigned idx, const char* textureName);
		void SetFadeAlpha(Real alpha);
		PointLightManagerPtr GetPointLightMan() const;
		void RegisterVideoPlayer(IVideoPlayerPtr player);
		void UnregisterVideoPlayer(IVideoPlayerPtr player);
		bool GetSampleOffsets_Bloom(DWORD dwTexSize,
			float afTexCoordOffset[15],
			Vec4* avColorWeight,
			float fDeviation, float fMultiplier);
		void GetSampleOffsets_GaussBlur5x5(DWORD texWidth, DWORD texHeight, Vec4f** avTexCoordOffset, Vec4f** avSampleWeight, float fMultiplier);
		void GetSampleOffsets_DownScale2x2(DWORD texWidth, DWORD texHeight, Vec4f* avSampleOffsets);
		bool IsLuminanceOnCpu() const;
		void SetLockDepthStencilState(bool lock);
		void SetLockBlendState(bool lock);
		void SetFontTextureAtlas(const char* path);
		

		//-------------------------------------------------------------------
		// Queries
		//-------------------------------------------------------------------
		unsigned GetMultiSampleCount() const;
		bool GetFilmicToneMapping() const;
		void SetFilmicToneMapping(bool use);
		bool GetLuminanaceOnCPU() const;
		void SetLuminanaceOnCPU(bool oncpu);
		RenderTargetPtr GetRenderTarget(HWindowId id) const;
		void SetCamera(CameraPtr pCamera);
		CameraPtr GetCamera() const; // this is for current carmera.
		CameraPtr GetMainCamera() const;
		HWindow GetMainWindowHandle() const;
		HWindowId GetMainWindowHandleId();		
		HWindow GetWindowHandle(HWindowId windowId);
		HWindowId GetWindowHandleId(HWindow window);
		Vec2I ToSreenPos(HWindowId id, const Vec3& ndcPos) const;
		Vec2 ToNdcPos(HWindowId id, const Vec2I& screenPos) const;
		unsigned GetNumLoadingTexture() const;
		Vec2I FindClosestSize(HWindowId id, const Vec2I& input);
		bool GetResolutionList(unsigned& outNum, Vec2I* list);
		RendererOptionsPtr GetRendererOptions() const;
		void SetMainWindowStyle(unsigned style);

		//-------------------------------------------------------------------
		// Drawing
		//-------------------------------------------------------------------
		void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation);
		void Draw(unsigned int vertexCount, unsigned int startVertexLocation);				
		void DrawFullscreenQuad(ShaderPtr pixelShader, bool farside);
		void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec4& color, MaterialPtr mat);
		void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color);
		void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color, bool updateRs);
		void DrawQuadWithTexture(const Vec2I& pos, const Vec2I& size, const Color& color, TexturePtr texture, MaterialPtr materialOverride = 0);
		void DrawQuadWithTextureUV(const Vec2I& pos, const Vec2I& size, const Vec2& uvStart, const Vec2& uvEnd,
			const Color& color, TexturePtr texture, MaterialPtr materialOverride = 0);
		void DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset,
			DWORD color, MaterialPtr pMat);
		void QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color);
		void QueueDrawText(const Vec2I& pos, WCHAR* text, const Color& color, Real size);
		void QueueDrawText(const Vec2I& pos, const char* text, const Color& color);
		void QueueDrawText(const Vec2I& pos, const char* text, const Color& color, Real size);
		void QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color);
		void QueueDraw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, Real size);
		void QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color);
		void QueueDraw3DText(const Vec3& worldpos, const char* text, const Color& color, Real size);
		void QueueDrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text, const Color& color);
		void QueueDrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text,
			const Color& color, Real size);
		void QueueDrawTextForDuration(Real secs, const Vec2I& pos, const char* text, const Color& color);
		void QueueDrawTextForDuration(Real secs, const Vec2I& pos, const char* text,
			const Color& color, Real size);
		void ClearDurationTexts();
		void QueueDrawLine(const Vec3& start, const Vec3& end,
			const Color& color0, const Color& color1);		
		void QueueDrawLine(const Vec2I& start, const Vec2I& end,
			const Color& color0, const Color& color1);
		void QueueDrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
			const Color& color0, const Color& color1);
		void QueueDrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color);
		/**Rendered before the transparent object.*/
		void QueueDrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
			const char* texture, bool textureFlow);
		/**Rendered before the transparent object.*/
		void QueueDrawSphere(const Vec3& pos, Real radius, const Color& color);
		/**Rendered before the transparent object.*/
		void QueueDrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha);
		/**Rendered before the transparent object.*/
		void QueueDrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha);		
		void QueueDrawQuadLine(const Vec2I& pos, const Vec2I& size, const Color& color);

		//-------------------------------------------------------------------
		// Internal
		//-------------------------------------------------------------------
		void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst);
		void RefreshPointLight();
		bool NeedToRefreshPointLight() const;
		void RenderDebugHud();

		//-------------------------------------------------------------------
		// ISceneObserver
		//-------------------------------------------------------------------
		void OnAfterMakeVisibleSet(IScene* scene);
		void OnBeforeRenderingOpaques(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut);
		void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut);

		//-------------------------------------------------------------------
		// IInputConsumer
		//-------------------------------------------------------------------
		void ConsumeInput(IInputInjectorPtr injector); /// inject to main camera
	};

}