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
#define FB_RENDERER_VERSION "1"
#include "FBCommonHeaders/Types.h"
#include "IPlatformIndexBuffer.h"
#include "ShaderDefines.h"
#include "InputElementDesc.h"
#include "ShaderConstants.h"
#include <memory>

namespace fb{
	FB_DECLARE_SMART_PTR(IPlatformSamplerState);
	FB_DECLARE_SMART_PTR(IPlatformRasterizerState);
	FB_DECLARE_SMART_PTR(IPlatformDepthStencilState);
	FB_DECLARE_SMART_PTR(IPlatformBlendState);
	FB_DECLARE_SMART_PTR(IPlatformTexture);
	FB_DECLARE_SMART_PTR(IPlatformShader);		
	FB_DECLARE_SMART_PTR(IPlatformInputLayout);
	FB_DECLARE_SMART_PTR(IPlatformIndexBuffer);
	FB_DECLARE_SMART_PTR(IPlatformVertexBuffer);		
	FB_DECLARE_SMART_PTR(IPlatformRenderer);
	
	struct CanvasInitInfo
	{
		CanvasInitInfo(HWindowId windowId, HWindow window, int width, int height, int fullscreen,
			PIXEL_FORMAT colorFormat, PIXEL_FORMAT depthFormat)
			: mId(windowId)
			, mWindow(window)
			, mWidth(width)
			, mHeight(height)
			, mColorFormat(colorFormat)
			, mDepthFormat(depthFormat)
		{
		}

		HWindowId mId;
		HWindow mWindow;
		int mWidth, mHeight, mFullscreen;
		PIXEL_FORMAT mColorFormat, mDepthFormat;
	};

	/** Plug-in interface for render engine like D3D11, OpenGL.	
	*/
	class IPlatformRenderer{
	public:		
		virtual void PrepareQuit() = 0;
		//-------------------------------------------------------------------
		// Device features
		//-------------------------------------------------------------------		
		virtual Vec2ITuple FindClosestSize(HWindowId id, const Vec2ITuple& input) = 0;
		/** Returns resolution list.
		First, call this function with \a list = 0. Then you will get how many resolutions are supported.
		Allocated Vec2ITuple array with that count, and call this function again with valid \a list.
		*/
		virtual bool GetResolutionList(unsigned& outNum, Vec2ITuple* list) = 0;
		/** Initializes canvas(swap-chain)
		\param fullscreen 0: window mode, 1: full-screen, 2: faked full-screen
		*/
		virtual bool InitCanvas(const CanvasInitInfo& info, 
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) = 0;		
		virtual void DeinitCanvas(HWindowId id, HWindow window) = 0;
		/** Changes the resolution.
		if the new outColorTexture and outDepthTexture is prepared, return true.
		*/
		virtual bool ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& resol,
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) = 0;
		virtual bool ChangeFullscreenMode(HWindowId id, HWindow window, int mode) = 0;
		virtual unsigned GetMultiSampleCount() const = 0;
		

		//-------------------------------------------------------------------
		// Resource creation
		//-------------------------------------------------------------------
		virtual void SetShaderCacheOption(bool useShaderCache, bool generateCache) = 0;		
		virtual IPlatformTexturePtr CreateTexture(const char* path, const TextureCreationOption& option) = 0;
		// mipLevels 0 for full generated mips.
		virtual IPlatformTexturePtr CreateTexture(void* data, int width, int height,
			PIXEL_FORMAT format, int mipLevels, BUFFER_USAGE usage, int  buffer_cpu_access,
			int texture_type) = 0;		
		virtual IPlatformVertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
			unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) = 0;
		virtual IPlatformIndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
			INDEXBUFFER_FORMAT format) = 0;
		virtual IPlatformShaderPtr CreateVertexShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) = 0;
		virtual IPlatformShaderPtr CreateGeometryShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) = 0;
		virtual IPlatformShaderPtr CreatePixelShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) = 0;
		virtual IPlatformShaderPtr CreateComputeShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) = 0;
		virtual IPlatformShaderPtr CompileComputeShader(const char* code, const char* entry,
			const SHADER_DEFINES& defines) = 0;
		virtual IPlatformInputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
			void* shaderByteCode, unsigned size) = 0;
		virtual IPlatformBlendStatePtr CreateBlendState(const BLEND_DESC& desc) = 0;
		virtual IPlatformDepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc) = 0;
		virtual IPlatformRasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc) = 0;
		virtual IPlatformSamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc) = 0;
		virtual unsigned GetNumLoadingTexture() const = 0;

		//-------------------------------------------------------------------
		// Resource Bindings
		//-------------------------------------------------------------------
		virtual void SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
			IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) = 0;
		virtual void SetDepthTarget(IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) = 0;
		virtual void SetViewports(const Viewport viewports[], int num) = 0;
		virtual void SetScissorRects(const Rect rects[], int num) = 0;
		virtual void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
			IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]) = 0;
		virtual void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt) = 0;
		virtual void SetTextures(IPlatformTexturePtr pTextures[], int num, SHADER_TYPE shaderType, int startSlot) = 0;
		virtual void UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size) = 0;		
		virtual void* MapShaderConstantsBuffer() const = 0;
		virtual void UnmapShaderConstantsBuffer() const = 0;
		virtual void* MapBigBuffer() const = 0;
		virtual void UnmapBigBuffer() const = 0;
		virtual void UnbindInputLayout() = 0;
		virtual void UnbindShader(SHADER_TYPE shader) = 0;
		virtual void UnbindTexture(SHADER_TYPE shader, int slot) = 0;
		virtual void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz, 
			IPlatformTexture* src, UINT srcSubresource, Box3D* pBox) = 0;


		//-------------------------------------------------------------------
		// Drawing
		//-------------------------------------------------------------------
		virtual void Draw(unsigned int vertexCount, unsigned int startVertexLocation) = 0;
		virtual void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) = 0;		
		virtual void Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil) = 0;
		virtual void Clear(Real r, Real g, Real b, Real a) = 0;
		virtual void ClearDepthStencil(Real z, UINT8 stencil) = 0;
		virtual void ClearState() = 0;		
		virtual void Present() = 0;

		//-------------------------------------------------------------------
		// Debugging & Profiling
		//-------------------------------------------------------------------
		virtual void BeginEvent(const char* name) = 0;
		virtual void EndEvent() = 0;
		virtual void TakeScreenshot(const char* filename) = 0;

	protected:
		~IPlatformRenderer() {}
	};
}