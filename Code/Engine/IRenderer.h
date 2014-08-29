#pragma once
#ifndef _IRenderer_header_included_
#define _IRenderer_header_included_
#pragma warning( disable : 4275 ) // Because ReferenceCounter is not exported class.

#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/Renderer/RendererStructs.h>
#include <Engine/Renderer/RendererEnums.h>
#include <Engine/IInputLayout.h>
#include <Engine/IScene.h>
#include <Engine/IShader.h>
#include <Engine/ITexture.h>
#include <Engine/IObject.h>
#include <Engine/IMaterial.h>
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

class ICamera;
class Vec2I;
class Vec3;
class IRasterizerState;
class IBlendState;
class IDepthStencilState;
class IFont;
class IRenderToTexture;
class ILight;
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

class IRenderer : public ReferenceCounter
{
public:
	static IRenderer* CreateD3D9Instance();
	static IRenderer* CreateD3D11Instance();
	static IRenderer* CreateOpenGLInstance();

	virtual ~IRenderer() {}

	// threadPool 0 : no thread pool
	virtual bool Init(int threadPool) = 0;
	virtual int InitSwapChain(HWND handle, int width, int height) = 0;
	virtual void Deinit() = 0;
	virtual void ProcessRenderToTexture() = 0;
	virtual IRenderToTexture* CreateRenderToTexture(bool everyframe) = 0;
	virtual void DeleteRenderToTexture(IRenderToTexture*) = 0;
	virtual void SetClearColor(float r, float g, float b, float a=1.f) = 0;
	virtual void SetClearDepthStencil(float z, UINT8 stencil) = 0;
	virtual void Clear(float r, float g, float b, float a, float z, UINT8 stencil) = 0;
	virtual void Clear() = 0;
	virtual void Clear(float r, float g, float b, float a) = 0;// only color
	virtual void ClearState() = 0;
	virtual void SetCamera(ICamera* pCamera) = 0;
	virtual ICamera* GetCamera() const = 0;
	virtual void UpdateFrameConstantsBuffer() = 0;
	virtual void UpdateObjectConstantsBuffer(void* pData) = 0;
	virtual void UpdateMaterialConstantsBuffer(void* pData) = 0;
	virtual void UpdateRareConstantsBuffer() = 0;
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
	virtual unsigned GetWidth() const = 0;
	virtual unsigned GetHeight() const=0;
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
	virtual IIndexBuffer* CreateIndexBuffer(void* data, unsigned int numIndices, 
		INDEXBUFFER_FORMAT format) = 0;
	virtual IShader* CreateShader(const char* filepath, int shaders,
		const IMaterial::SHADER_DEFINES& defines, IShader* pReloadingShader=0) = 0;
	virtual ITexture* CreateTexture(const Vec2I& size, int mipLevels, int arraySize) = 0;
	virtual ITexture* CreateTexture(const char* file, ITexture* pReloadingTexture=0) = 0;
	virtual ITexture* CreateTexture(void* data, int width, int height, PIXEL_FORMAT format,
		BUFFER_USAGE usage, int  buffer_cpu_access, int texture_type) = 0;

	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num, 
		ITexture* pDepthStencil, size_t dsIndex) = 0;
	virtual void SetRenderTarget(ITexture* pRenderTargets[], size_t rtIndex[], int num) = 0;
	virtual void RestoreRenderTarget() = 0;
	// internal use
	// after called:
	// 0 : current rendertarget
	// 1 : glow rendertarget
	// 2 : not changed
	// 3 : not changed
	virtual void SetGlowRenderTarget() = 0;
	virtual void UnSetGlowRenderTarget() = 0;
	virtual void SetViewports(Viewport viewports[], int num) = 0;
	virtual void RestoreViewports() = 0;
	virtual void SetScissorRects(RECT rects[], int num) = 0;
	virtual void RestoreScissorRects() = 0;
	// to restore directionalLight call this function with null light.
	virtual void SetDirectionalLight(ILight* pLight) = 0;
	virtual ILight* GetDirectionalLight() const = 0;
	// returning number of vertices.
	virtual int BindFullscreenQuadUV_VB(bool farSide) = 0;
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

	virtual void SetTextureSamplerState(ITexture* pTexture, const SAMPLER_DESC& desc) = 0;

	virtual IRasterizerState* CreateRasterizerState(const RASTERIZER_DESC& desc) =0;
	virtual IBlendState* CreateBlendState(const BLEND_DESC& desc) = 0;
	virtual IDepthStencilState* CreateDepthStencilState( const DEPTH_STENCIL_DESC& desc ) = 0;

	// internal
	virtual void SetRasterizerState(IRasterizerState* pRasterizerState) = 0;
	virtual void SetBlendState(IBlendState* pBlendState) = 0;
	virtual void SetDepthStencilState(IDepthStencilState* pDepthStencilState, unsigned stencilRef) = 0;

	virtual void InitFrameProfiler(float dt) = 0;
	virtual const RENDERER_FRAME_PROFILER& GetFrameProfiler() const = 0;

	virtual Vec2I ToSreenPos(const Vec3& ndcPos) const = 0;
	virtual Vec2 ToNdcPos(const Vec2I& screenPos) const = 0;

	virtual void DrawText(const Vec2I& pos, WCHAR* text, const Color& color) = 0;
	virtual void DrawText(const Vec2I& pos, const char* text, const Color& color) = 0;
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color) = 0;
	virtual void DrawTextForDuration(float secs, const Vec2I& pos, const char* text, 
		const Color& color) = 0;
	virtual void DrawLine(const Vec3& start, const Vec3& end, 
		const Color& color0, const Color& color1) = 0;
	virtual void DrawLine(const Vec2I& start, const Vec2I& end, 
		const Color& color, const Color& color1) = 0;
	virtual void RenderDebugHud() = 0; 
	virtual inline IFont* GetFont() const = 0;
	virtual void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color) = 0;
	virtual void DrawBillboardWorldQuad(const Vec3& pos, const Vec2& size, const Vec2& offset, 
		DWORD color, IMaterial* pMat) = 0;

	virtual const INPUT_ELEMENT_DESCS& GetInputElementDesc(
		DEFAULT_INPUTS::Enum e) = 0;

	// do not delete the returned point
	virtual TextureAtlas* GetTextureAtlas(const char* path) = 0;
	virtual TextureAtlasRegion* GetTextureAtlasRegion(const char* path, const char* region) = 0;	

	// will hold reference.
	virtual void SetEnvironmentTexture(ITexture* pTexture) = 0;

	//-------------------------------------------------------------------------
	// Render States
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

	// raster
	virtual void SetFrontFaceCullRS() = 0;

	virtual void SetDepthWriteShader() = 0;
	virtual void SetOccPreShader() = 0;
	virtual void SetOccPreGSShader() = 0;
	virtual void SetPositionInputLayout() = 0;

	virtual void UpdateEnvMapInNextFrame(ISkySphere* sky) = 0;

	virtual void InitCloud(unsigned numThreads, unsigned numCloud, CloudProperties* clouds) = 0;
	virtual void CleanCloud() = 0;

	virtual void SetCloudRendering(bool rendering) = 0;
	
};

}

#endif //_IRenderer_header_included_