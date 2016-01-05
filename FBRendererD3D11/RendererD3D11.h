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
#ifndef __Rendererd3d11_header_included__
#define __Rendererd3d11_header_included__

#include "FBCommonHeaders/Types.h"
#include "FBRenderer/IPlatformRenderer.h"
#include "FBRenderer/RendererStructs.h"

#include "EssentialEngineData/shaders/Constants.h"
#include "RenderStatesD3D11.h"

namespace fb
{
	class IObject;
	class ICamera;
	class VertexBufferD3D11;
	class IndexBufferD3D11;
	class TextureD3D11;
	class Color;
	class RendererD3D11;
	class IRenderTarget;
	class ShaderD3D11;
	class InputLayoutD3D11;
	
	FB_DECLARE_SMART_PTR(RendererD3D11);
	class RendererD3D11 : public IPlatformRenderer
	{	
		FB_DECLARE_PIMPL_NON_COPYABLE(RendererD3D11);
		RendererD3D11();
		virtual ~RendererD3D11();

	public:
		static IPlatformRenderer* Create();
		static void Destroy();
		static RendererD3D11& GetInstance();			

		//-------------------------------------------------------------------
		// IPlatformRenderer interface
		//-------------------------------------------------------------------		
		// Device features
		Vec2ITuple FindClosestSize(HWindowId id, const Vec2ITuple& input);
		bool GetResolutionList(unsigned& outNum, Vec2ITuple* list);
		bool InitCanvas(HWindowId id, HWindow window, int width, int height, int fullscreen,
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture);
		void DeinitCanvas(HWindowId id, HWindow window);
		bool ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& resol,
			IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture);
		bool ChangeFullscreenMode(HWindowId id, HWindow window, int mode);
		unsigned GetMultiSampleCount() const;

		// Resource creation
		void SetShaderCacheOption(bool useShaderCache, bool generateCache);
		IPlatformTexturePtr CreateTexture(const char* path, bool async);
		IPlatformTexturePtr CreateTexture(void* data, int width, int height,
			PIXEL_FORMAT format, BUFFER_USAGE usage, int  buffer_cpu_access,
			int texture_type);
		IPlatformVertexBufferPtr CreateVertexBuffer(void* data, unsigned stride,
			unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag);
		IPlatformIndexBufferPtr CreateIndexBuffer(void* data, unsigned int numIndices,
			INDEXBUFFER_FORMAT format);
		IPlatformShaderPtr CreateShader(const char* path, int shaders,
			const SHADER_DEFINES& defines, bool ignoreCache);
		IPlatformInputLayoutPtr CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,
			void* shaderByteCode, unsigned size);
		IPlatformBlendStatePtr CreateBlendState(const BLEND_DESC& desc);
		IPlatformDepthStencilStatePtr CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		IPlatformRasterizerStatePtr CreateRasterizerState(const RASTERIZER_DESC& desc);
		IPlatformSamplerStatePtr CreateSamplerState(const SAMPLER_DESC& desc);
		unsigned GetNumLoadingTexture() const;
		
		// Resource Binding
		void SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,
			IPlatformTexturePtr pDepthStencil, size_t dsViewIndex);
		void SetViewports(const Viewport viewports[], int num);
		void SetScissorRects(const Rect rects[], int num);
		void SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,
			IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]);
		void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt);		
		void SetTextures(IPlatformTexturePtr pTextures[], int num, BINDING_SHADER shaderType, int startSlot);		
		void UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size);
		void* MapMaterialParameterBuffer() const;
		void UnmapMaterialParameterBuffer() const;
		void* MapBigBuffer() const;
		void UnmapBigBuffer() const;
		void UnbindInputLayout();
		void UnbindShader(BINDING_SHADER shader);
		void UnbindTexture(BINDING_SHADER shader, int slot);
		void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,
			IPlatformTexture* src, UINT srcSubresource, Box3D* pBox);

		// Drawing
		void Draw(unsigned int vertexCount, unsigned int startVertexLocation);
		void DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation);				
		void Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil);
		void Clear(Real r, Real g, Real b, Real a);
		void ClearState();
		void Present();

		// Debugging & Profiling
		void BeginEvent(const char* name);
		void EndEvent();
		void TakeScreenshot(const char* filename);


		//-------------------------------------------------------------------
		// Platform Specific
		//-------------------------------------------------------------------
		// Resource Manipulations
		MapData MapBuffer(ID3D11Resource* pResource, UINT subResource, MAP_TYPE type, MAP_FLAG flag) const;
		void UnmapBuffer(ID3D11Resource* pResource, UINT subResource) const;
		void SaveTextureToFile(TextureD3D11* texture, const char* filename);		
		void GenerateMips(TextureD3D11* pTexture);
		
		// Resource Bindings
		void SetIndexBuffer(IndexBufferD3D11* pIndexBuffer, unsigned offset);
		void SetTexture(TextureD3D11* pTexture, BINDING_SHADER shaderType, unsigned int slot);
		void SetShaders(ShaderD3D11* pShader);
		void SetVSShader(ShaderD3D11* pShader);
		void SetHSShader(ShaderD3D11* pShader);
		void SetDSShader(ShaderD3D11* pShader);		
		void SetGSShader(ShaderD3D11* pShader);
		void SetPSShader(ShaderD3D11* pShader);	
		
		void SetInputLayout(InputLayoutD3D11* pInputLayout);		
		void SetRasterizerState(RasterizerStateD3D11* pRasterizerState);
		void SetBlendState(BlendStateD3D11* pBlendState);
		void SetDepthStencilState(DepthStencilStateD3D11* pDepthStencilState, unsigned stencilRef);
		void SetSamplerState(SamplerStateD3D11* pSamplerState, BINDING_SHADER shader, int slot);
	};
}



#endif //__Rendererd3d11_header_included__