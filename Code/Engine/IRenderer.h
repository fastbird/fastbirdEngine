#pragma once
#ifndef _IRenderer_header_included_
#define _IRenderer_header_included_
#pragma warning( disable : 4275 ) // Because ReferenceCounter is not exported class.

#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/RendererStructs.h>
#include <Engine/RendererEnums.h>
#include <Engine/IInputLayout.h>
#include <Engine/IScene.h>
#include <Engine/IShader.h>
#include <Engine/ITexture.h>
#include <Engine/IObject.h>
#include <Engine/IMaterial.h>
#include <Engine/IEngine.h>
#include <CommonLib/Math/Vec2I.h>
#include <CommonLib/Math/Vec2.h>

#undef DrawText

#ifdef _DEBUG
#define FB_SET_DEVICE_DEBUG_NAME(pObj, name) \
	assert(pObj); \
	assert(name); \
	assert(strlen(name) > 0); \
	pObj->SetDebugName((name))

#else
#define FB_SET_DEVICE_DEBUG_NAME(pObj, name)
#endif

namespace fastbird
{	
struct POINT_LIGHT_CONSTANTS;
class IPointLight;
class ICamera;
class Vec2I;
class Vec3;
class IRasterizerState;
class IBlendState;
class IDepthStencilState;
class ISamplerState;
class IFont;
class IRenderTarget;
class ILight;
class PointLightMan;
class IScene;
class IRenderListener;

struct CloudProperties
{
	float		fLength;
	float		fWidth;
	float		fHigh;
	float		fCellSize;
	float		fEvolvingSpeed;
	Vec3		vCloudPos;
	unsigned	particleID;
};
struct RenderTargetParam
{
	bool mEveryFrame;
	Vec2I mSize;
	PIXEL_FORMAT mPixelFormat;
	bool mShaderResourceView;
	bool mMipmap;
	bool mCubemap;
	bool mHasDepth; // set true, and call IRenderTarget::SetDepthStencilDesc().
	bool mUsePool;
};

class IRenderer : public ReferenceCounter
{
public:
	static IRenderer* CreateD3D9Instance();
	static IRenderer* CreateD3D11Instance();
	static IRenderer* CreateOpenGLInstance();

	virtual ~IRenderer() {}

	// threadPool 0 : no thread pool
	virtual bool Init(int threadPool) = 0;
	virtual void Deinit() = 0;
	virtual void ProcessRenderTarget() = 0;
	virtual IRenderTarget* CreateRenderTarget(const RenderTargetParam& param) = 0;
	virtual void DeleteRenderTarget(IRenderTarget*) = 0;
	virtual IRenderTarget* GetRenderTarget(HWND_ID id) const = 0;
	virtual void SetClearColor(HWND_ID id, const Color& color) = 0;
	virtual void SetClearDepthStencil(HWND_ID id, float z, UINT8 stencil) = 0;
	virtual void Clear(float r, float g, float b, float a, float z, UINT8 stencil) = 0;
	virtual void Clear(float r, float g, float b, float a) = 0;// only color
	// do not use if possible.
	virtual void ClearState() = 0;
	virtual void SetCamera(ICamera* pCamera) = 0;
	virtual ICamera* GetCamera() const = 0; // this is for current carmera.
	virtual ICamera* GetMainCamera() const = 0;
	virtual void UpdateObjectConstantsBuffer(void* pData) = 0;
	virtual void UpdatePointLightConstantsBuffer(void* pData) = 0;
	virtual void* MapMaterialParameterBuffer() = 0;
	virtual void UnmapMaterialParameterBuffer() = 0;
	virtual void* MapBigBuffer() = 0;
	virtual void UnmapBigBuffer() = 0;
	virtual void Present() = 0;
	virtual void SetVertexBuffer(unsigned int startSlot, unsigned int numBuffers,
		IVertexBuffer* pVertexBuffers[], unsigned int strides[], unsigned int offsets[]) = 0;
	virtual void SetIndexBuffer(IIndexBuffer* pIndexBuffer) = 0;
	virtual void SetShaders(IShader* pShader) = 0;
	virtual void SetVSShader(IShader* pShader) = 0;
	virtual void SetPSShader(IShader* pShader) = 0;
	virtual void SetGSShader(IShader* pShader) = 0;
	virtual void SetDSShader(IShader* pShader) = 0;
	virtual void SetHSShader(IShader* pShader) = 0;
	virtual void SetInputLayout(IInputLayout* pInputLayout) = 0;
	virtual void SetTexture(ITexture* pTexture, BINDING_SHADER shaderType, unsigned int slot) = 0;
	virtual void SetTextures(ITexture* pTextures[], int num, BINDING_SHADER shaderType, int startSlot) = 0;
	virtual void GenerateMips(ITexture* pTexture) = 0;
	virtual void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt) = 0;
	virtual void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) = 0;
	virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation) = 0;
	virtual unsigned GetWidth(HWND hWnd) const = 0;
	virtual unsigned GetHeight(HWND hWnd) const = 0;
	virtual unsigned GetWidth(HWND_ID hWnd) const = 0;
	virtual unsigned GetHeight(HWND_ID hWnd) const = 0;
	virtual void SetWireframe(bool enable) = 0;
	virtual bool GetWireframe() const = 0;
	virtual unsigned GetMultiSampleCount() const = 0;

	virtual MapData MapVertexBuffer(IVertexBuffer* pBuffer, UINT subResource, 
		MAP_TYPE type, MAP_FLAG flag) = 0;
	virtual void UnmapVertexBuffer(IVertexBuffer* pBuffer, unsigned int subResource) = 0;
	virtual MapData MapTexture(ITexture* pTexture, UINT subResource, 
		MAP_TYPE type, MAP_FLAG flag) = 0;
	virtual void UnmapTexture(ITexture* pTexture, UINT subResource) = 0;
	virtual void CopyToStaging(ITexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
		ITexture* src, UINT srcSubresource, Box3D* pBox) = 0;
	virtual void SaveTextureToFile(ITexture* texture, const char* filename) = 0;
		
	virtual IVertexBuffer* CreateVertexBuffer(void* data, unsigned stride, 
		unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) = 0;
	// when you don't use SmartPtr
	virtual void DeleteVertexBuffer(IVertexBuffer* buffer) = 0;
	virtual IIndexBuffer* CreateIndexBuffer(void* data, unsigned int numIndices, 
		INDEXBUFFER_FORMAT format) = 0;
	virtual IShader* CreateShader(const char* filepath, int shaders,
		const IMaterial::SHADER_DEFINES& defines = IMaterial::SHADER_DEFINES(), IShader* pReloadingShader = 0) = 0;
	virtual ITexture* CreateTexture(const Vec2I& size, int mipLevels, int arraySize) = 0;
	virtual ITexture* CreateTexture(const char* file, ITexture* pReloadingTexture=0) = 0;
	virtual ITexture* CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
		BUFFER_USAGE usage, int  buffer_cpu_access, int texture_type) = 0;

	virtual IRenderTarget* GetMainRenderTarget() const = 0;
	virtual IScene* GetMainScene() const = 0;	
	virtual const Vec2I& GetMainRTSize() const = 0;

	virtual void SetCurRenderTarget(IRenderTarget* renderTarget) = 0;
	virtual IRenderTarget* GetCurRendrTarget() const = 0;
	virtual bool IsMainRenderTarget() const = 0;
	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtViewIndex[], int num,
		ITexture* pDepthStencil, size_t dsViewIndex) = 0;
	virtual const Vec2I& GetRenderTargetSize() const = 0;
	virtual void SetViewports(Viewport viewports[], int num) = 0;
	virtual void SetScissorRects(RECT rects[], int num) = 0;
	virtual void RestoreScissorRects() = 0;
	// to restore directionalLight call this function with null light.
	virtual void SetDirectionalLight(ILight* pLight, int idx) = 0;
	virtual ILight* GetDirectionalLight(int idx) const = 0;
	virtual void DrawFullscreenQuad(IShader* pixelShader, bool farside) = 0;

	virtual IMaterial* CreateMaterial(const char* file) = 0;
	virtual IMaterial* GetMissingMaterial() = 0;

	// use this if you are sure there is instance of the descs.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs) = 0;

	// use these if you are not sure.
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IMaterial* material) = 0;
	virtual IInputLayout* GetInputLayout(const INPUT_ELEMENT_DESCS& descs,
		IShader* shader) = 0;
	// auxiliary
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IMaterial* material) = 0;
	virtual IInputLayout* GetInputLayout(DEFAULT_INPUTS::Enum e,
		IShader* shader) = 0;

	virtual IRasterizerState* CreateRasterizerState(const RASTERIZER_DESC& desc) =0;
	virtual IBlendState* CreateBlendState(const BLEND_DESC& desc) = 0;
	virtual IDepthStencilState* CreateDepthStencilState( const DEPTH_STENCIL_DESC& desc ) = 0;
	virtual ISamplerState* CreateSamplerState(const SAMPLER_DESC& desc) = 0;

	// internal
	virtual void SetRasterizerState(IRasterizerState* pRasterizerState) = 0;
	virtual void SetBlendState(IBlendState* pBlendState) = 0;
	virtual void SetDepthStencilState(IDepthStencilState* pDepthStencilState, unsigned stencilRef) = 0;
	virtual void SetSamplerState(ISamplerState* pSamplerState, BINDING_SHADER shader, int slot) = 0;

	virtual void InitFrameProfiler(float dt) = 0;
	virtual const RENDERER_FRAME_PROFILER& GetFrameProfiler() const = 0;

	virtual Vec2I ToSreenPos(HWND_ID id, const Vec3& ndcPos) const = 0;
	virtual Vec2 ToNdcPos(HWND_ID id, const Vec2I& screenPos) const = 0;

	virtual void DrawText(const Vec2I& pos, WCHAR* text, const Color& color, float size = 24) = 0;
	virtual void DrawText(const Vec2I& pos, const char* text, const Color& color, float size = 24) = 0;
	virtual void Draw3DText(const Vec3& worldpos, WCHAR* text, const Color& color, float size = 24) = 0;
	virtual void Draw3DText(const Vec3& worldpos, const char* text, const Color& color, float size = 24) = 0;
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color, float size = 24) = 0;
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
		const Color& color, float size = 24) = 0;

	// no depth culling
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1) = 0;
	virtual void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end,
		const Color& color0, const Color& color1) = 0;
	virtual void DrawLine(const Vec2I& start, const Vec2I& end, 
		const Color& color, const Color& color1) = 0;

	// with depth culling
	virtual void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
		const char* texture, bool textureFlow) = 0;


	virtual void DrawSphere(const Vec3& pos, float radius, const Color& color) = 0;
	virtual void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha) = 0;
	virtual void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha) = 0;
	virtual void RenderGeoms() = 0;
	virtual void RenderDebugHud() = 0; 
	virtual inline IFont* GetFont() const = 0;
	virtual void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color) = 0;
	virtual void DrawQuadLine(const Vec2I& pos, const Vec2I& size, const Color& color) = 0;
	virtual void DrawQuadWithTexture(const Vec2I& pos, const Vec2I& size, const Color& color, ITexture* texture, IMaterial* materialOverride = 0) = 0;
	virtual void DrawQuadWithTextureUV(const Vec2I& pos, const Vec2I& size, const Vec2& uvStart, const Vec2& uvEnd,
		const Color& color, ITexture* texture, IMaterial* materialOverride = 0) = 0;
	virtual void DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset, 
		DWORD color, IMaterial* pMat) = 0;
	virtual void DrawTriangleNow(const Vec3& a, const Vec3& b, const Vec3& c, const Vec4& color, IMaterial* mat) = 0;

	virtual const INPUT_ELEMENT_DESCS& GetInputElementDesc(
		DEFAULT_INPUTS::Enum e) = 0;

	// do not delete the returned point
	virtual TextureAtlas* GetTextureAtlas(const char* path) = 0;
	virtual TextureAtlasRegion* GetTextureAtlasRegion(const char* path, const char* region) = 0;	

	// will hold reference.
	virtual void SetEnvironmentTexture(ITexture* pTexture) = 0;
	virtual void BindDepthTexture(bool set) = 0;
	//-------------------------------------------------------------------------
	// Render States
	virtual void RestoreRenderStates() = 0;
	virtual void RestoreRasterizerState() = 0;
	virtual void RestoreBlendState() = 0;
	virtual void RestoreDepthStencilState() = 0;
	virtual void LockDepthStencilState() = 0;
	virtual void UnlockDepthStencilState() = 0;

	// blend
	virtual void SetAlphaBlendState() = 0;
	virtual void SetRedAlphaMask() = 0;
	virtual void SetGreenAlphaMask() = 0;
	virtual void SetGreenMask() = 0;
	virtual void SetBlueMask() = 0;
	virtual void SetGreenAlphaMaskMax() = 0;
	
	virtual void SetGreenAlphaMaskAddAddBlend() = 0;
	virtual void SetRedAlphaMaskAddMinusBlend() = 0;

	virtual void SetGreenAlphaMaskAddMinusBlend() = 0;
	virtual void SetRedAlphaMaskAddAddBlend() = 0;

	// depth
	virtual void SetNoDepthWriteLessEqual() = 0;
	virtual void SetLessEqualDepth() = 0;
	virtual void SetNoDepthStencil() = 0;

	// raster
	virtual void SetFrontFaceCullRS() = 0;

	// sampler
	virtual void SetSamplerState(SAMPLERS::Enum s, BINDING_SHADER shader, int slot) = 0;

	virtual ITexture* GetTemporalDepthBuffer(const Vec2I& size) = 0;

	virtual void SetDepthWriteShader() = 0;
	virtual void SetDepthWriteShaderCloud() = 0;
	virtual void SetOccPreShader() = 0;
	virtual void SetOccPreGSShader() = 0;
	virtual void SetPositionInputLayout() = 0;

	virtual void UpdateEnvMapInNextFrame(ISkySphere* sky) = 0;

	virtual void InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds) = 0;
	virtual void CleanCloud() = 0;

	virtual void SetDebugRenderTarget(unsigned idx, const char* textureName) = 0;

	virtual unsigned GetNumLoadingTexture() const = 0;

	virtual void SetEnvironmentTextureOverride(ITexture* texture) = 0;
	
	// internal only
	virtual void GatherPointLightData(BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst) = 0;
	virtual void RefreshPointLight() = 0;
	virtual bool NeedToRefreshPointLight() const = 0;

	virtual IPointLight* CreatePointLight(const Vec3& pos, float range, const Vec3& color, float intensity, float lifeTime, 
		bool manualDeletion) = 0;
	virtual void DeletePointLight(IPointLight* pointLight) = 0;

	virtual void SetFadeAlpha(float alpha) = 0;
	virtual PointLightMan* GetPointLightMan() const = 0;

	virtual IMaterial* GetMaterial(DEFAULT_MATERIALS::Enum type) = 0;

	virtual int CropSize8(int size) const = 0;
	
	virtual void RegisterUIs(HWND_ID hwndId, std::vector<IUIObject*>& uiobj) = 0;
	virtual void UnregisterUIs(HWND_ID hwndId) = 0;
	virtual void Register3DUIs(HWND_ID hwndId, const char* name, std::vector<IUIObject*>& objects) = 0;
	virtual void Unregister3DUIs(const char* name) = 0;
	virtual void Set3DUIPosSize(const char* name, const Vec3& pos, const Vec2& sizeInWorld) = 0;
	virtual void Reset3DUI(const char* name) = 0;
	virtual void SetEnable3DUIs(bool enable) = 0;

	virtual void AddMarkObject(IObject* mark) = 0;
	virtual void RemoveMarkObject(IObject* mark) = 0;
	virtual void AddHPBarObject(IObject* hpBar) = 0;
	virtual void RemoveHPBarObject(IObject* hpBar) = 0;

	virtual void AddRenderListener(IRenderListener* listener) = 0;
	virtual void RemoveRenderListener(IRenderListener* listener) = 0;
};

}

#endif //_IRenderer_header_included_