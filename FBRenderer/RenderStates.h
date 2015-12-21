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
#include "IPlatformRenderStates.h"
#include "RendererStructs.h"
#include "FBCommonHeaders/Types.h"
namespace fb{	
	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(RasterizerState);
	class FB_DLL_RENDERER RasterizerState{
		FB_DECLARE_PIMPL_NON_COPYABLE(RasterizerState);
		RasterizerState();
		~RasterizerState();

	public:
		static RasterizerStatePtr Create();		

		void SetPlatformState(IPlatformRasterizerStatePtr state);
		void Bind();
		void SetDebugName(const char* name);
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(BlendState);
	class FB_DLL_RENDERER BlendState{
		FB_DECLARE_PIMPL_NON_COPYABLE(BlendState);
		BlendState();
		~BlendState();

	public:
		static BlendStatePtr Create();
		static void SetLock(bool lock);

		void SetPlatformState(IPlatformBlendStatePtr state);
		void Bind();
		void SetDebugName(const char* name);
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(DepthStencilState);
	class FB_DLL_RENDERER DepthStencilState{
		FB_DECLARE_PIMPL_NON_COPYABLE(DepthStencilState);
		DepthStencilState();
		~DepthStencilState();

	public:
		static DepthStencilStatePtr Create();
		static void SetLock(bool lock);

		void SetPlatformState(IPlatformDepthStencilStatePtr state);
		void Bind();
		void Bind(unsigned stencilRef);
		void SetDebugName(const char* name);
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(SamplerState);
	class FB_DLL_RENDERER SamplerState{
		FB_DECLARE_PIMPL_NON_COPYABLE(SamplerState);
		SamplerState();
		~SamplerState();

	public:
		static SamplerStatePtr Create();

		void SetPlatformState(IPlatformSamplerStatePtr state);
		void Bind(BINDING_SHADER shader, int slot);
		void SetDebugName(const char* name);
	};

	//---------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(RenderStates);
	/** represents RasterizerState, BlendState and DepthStencilState as one integrated object.
	RenderStates usually resides in Material. Internal data will be efficiently shared among
	RenderStates objects indicating the same states.
	*/
	class FB_DLL_RENDERER RenderStates
	{
		FB_DECLARE_PIMPL(RenderStates);		

	public:
		static RenderStatesPtr Create();
		static RenderStatesPtr Create(const RenderStates& other);		

		RenderStates();
		RenderStates(const RenderStates& other);
		~RenderStates();
		RenderStates& operator==(const RenderStates& other) = delete;

		void Reset();
		void ResetRasterizerState();
		void ResetBlendState();
		void ResetDepthStencilState();
		void CreateRasterizerState(const RASTERIZER_DESC& desc);
		void CreateBlendState(const BLEND_DESC& desc);
		void CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc);
		void Bind() const;
		void Bind(unsigned stencilRef) const;
		void DebugPrint() const;
	};
}