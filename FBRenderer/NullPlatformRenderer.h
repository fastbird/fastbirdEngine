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
#include "FBCommonHeaders/Types.h"
#include "IPlatformRenderer.h"
namespace fb{
	FB_DECLARE_SMART_PTR(NullPlatformRenderer);
	class NullPlatformRenderer : public IPlatformRenderer {
		NullPlatformRenderer();
		~NullPlatformRenderer();
	public:
		static NullPlatformRendererPtr Create();

		void RegisterThreadIdConsideredMainThread(std::thread::id threadId) OVERRIDE;
		void PrepareQuit() OVERRIDE;
		//-------------------------------------------------------------------
		// Device features
		//-------------------------------------------------------------------		
		Vec2ITuple FindClosestSize(HWindowId id, const Vec2ITuple& input);
		/** Returns resolution list.
		First, call this function with \a list = 0. Then you will get how many resolutions are supported.
		Allocated Vec2ITuple array with that count, and call this function again with valid \a list.
		*/
		bool GetResolutionList(unsigned& outNum, Vec2ITuple* list);
		/** Initializes canvas(swap-chain)
		\param fullscreen 0: window mode, 1: full-screen, 2: faked full-screen
		*/
		bool InitCanvas(const CanvasInitInfo& info,
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture);
		void DeinitCanvas(HWindowId id, HWindow window);
		bool ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& resol,
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture);
		bool ChangeFullscreenMode(HWindowId id, HWindow window, int mode);
		unsigned GetMultiSampleCount() const;
		bool IsDeviceRemoved() const OVERRIDE;
		bool IsFullscreen() const OVERRIDE;


		//-------------------------------------------------------------------
		// Resource creation
		//-------------------------------------------------------------------
		void SetShaderCacheOption(bool useShaderCache, bool generateCache);
		IPlatformTexturePtr CreateTexture(const char* path, const TextureCreationOption& option);
		IPlatformTexturePtr CreateTexture(void* data, int width, int height,
			PIXEL_FORMAT format, int mipLevels, BUFFER_USAGE usage, int  buffer_cpu_access,
			int texture_type);
		IPlatformVertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
			unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag);
		IPlatformIndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
			INDEXBUFFER_FORMAT format);
		IPlatformShaderPtr CreateVertexShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) OVERRIDE;
		IPlatformShaderPtr CreateGeometryShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) OVERRIDE;
		IPlatformShaderPtr CreatePixelShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) OVERRIDE;
		IPlatformShaderPtr CreateComputeShader(const char* path,
			const SHADER_DEFINES& defines, bool ignoreCache) OVERRIDE;
		IPlatformShaderPtr CompileComputeShader(const char* code, const char* entry,
			const SHADER_DEFINES& defines) OVERRIDE;
		IPlatformInputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
			void* shaderByteCode, unsigned size);
		IPlatformBlendStatePtr CreateBlendState(const BLEND_DESC& desc);
		IPlatformDepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		IPlatformRasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc);
		IPlatformSamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc);
		unsigned GetNumLoadingTexture() const;

		//-------------------------------------------------------------------
		// Resource Bindings
		//-------------------------------------------------------------------
		void SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
			IPlatformTexturePtr pDepthStencil, size_t dsViewIndex);
		void SetDepthTarget(IPlatformTexturePtr pDepthStencil, size_t dsViewIndex);
		void SetViewports(const Viewport viewports[], int num);
		void SetScissorRects(const Rect rects[], int num);
		void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
			IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]);
		void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt);
		void SetTextures(IPlatformTexturePtr pTextures[], int num, SHADER_TYPE shaderType, int startSlot);
		void UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size);
		virtual void* MapShaderConstantsBuffer() const;
		virtual void UnmapShaderConstantsBuffer() const;
		virtual void* MapBigBuffer() const;
		virtual void UnmapBigBuffer() const;
		void UnbindInputLayout();
		void UnbindShader(SHADER_TYPE shader);
		void UnbindTexture(SHADER_TYPE shader, int slot);
		void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
			IPlatformTexture* src, UINT srcSubresource, Box3D* pBox);


		//-------------------------------------------------------------------
		// Drawing
		//-------------------------------------------------------------------
		void Draw(unsigned int vertexCount, unsigned int startVertexLocation);
		void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation);
		void Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil);
		void Clear(Real r, Real g, Real b, Real a);
		void ClearDepthStencil(Real z, UINT8 stencil);
		void ClearState();
		void Present();

		//-------------------------------------------------------------------
		// Debugging & Profiling
		//-------------------------------------------------------------------
		void BeginEvent(const char* name);
		void EndEvent();
		void TakeScreenshot(const char* filename);
	};
}