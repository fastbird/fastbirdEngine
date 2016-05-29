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
#include "FBRenderer/IPlatformRenderStates.h"
#include "FBRenderer/RendererEnums.h"
#include "D3D11Types.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(RasterizerStateD3D11);
	class RasterizerStateD3D11 : public IPlatformRasterizerState
	{
		FB_DECLARE_NON_COPYABLE(RasterizerStateD3D11);

		ID3D11RasterizerStatePtr mRasterizerState;

		explicit RasterizerStateD3D11(ID3D11RasterizerState* rasterizerState);

	public:
		static RasterizerStateD3D11Ptr Create(ID3D11RasterizerState* rasterizerState);		

		//--------------------------------------------------------------------
		// IPlatformRasterizerState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind();
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------		
		ID3D11RasterizerState* GetHardwareRasterizerState() const;
	};

	//-------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(BlendStateD3D11);
	class BlendStateD3D11 : public IPlatformBlendState
	{
		FB_DECLARE_NON_COPYABLE(BlendStateD3D11);

		ID3D11BlendStatePtr mBlendState;

		explicit BlendStateD3D11(ID3D11BlendState* blendState);

	public:
		static BlendStateD3D11Ptr Create(ID3D11BlendState* blendState);

		//--------------------------------------------------------------------
		// IPlatformBlendState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind();
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------		
		ID3D11BlendState* GetHardwareBlendState() const;
		// Array of blend factors, one for each RGBA component.This requires 
		// a blend state object that specifies the D3D11_BLEND_BLEND_FACTOR option.
		float* GetBlendFactor() const { return 0; }
		// 32-bit sample coverage. The default value is 0xffffffff.
		// A sample mask determines which samples get updated in all the active render targets.
		// The mapping of bits in a sample mask to samples in a multisample render target is 
		// the responsibility of an individual application.
		// A sample mask is always applied; it is independent of whether multisampling is enabled, 
		// and does not depend on whether an application uses multisample render targets.
		DWORD GetSampleMask() const { return 0xffffffff;}
	};

	FB_DECLARE_SMART_PTR(DepthStencilStateD3D11);
	class DepthStencilStateD3D11 : public IPlatformDepthStencilState
	{
		FB_DECLARE_NON_COPYABLE(DepthStencilStateD3D11);

		ID3D11DepthStencilStatePtr mDepthStencilState;

		explicit DepthStencilStateD3D11(ID3D11DepthStencilState* depthStencilState);

	public:
		static DepthStencilStateD3D11Ptr Create(ID3D11DepthStencilState* depthStencilState);		

		//--------------------------------------------------------------------
		// IPlatformDepthStencilState Interfacec
		//--------------------------------------------------------------------
		virtual void Bind(int stencilRef);
		virtual void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------		
		ID3D11DepthStencilState* GetHardwareDSState() const;
	};

	FB_DECLARE_SMART_PTR(SamplerStateD3D11);
	class SamplerStateD3D11 : public IPlatformSamplerState
	{
		FB_DECLARE_NON_COPYABLE(SamplerStateD3D11);

		ID3D11SamplerStatePtr mSamplerState;

		explicit SamplerStateD3D11(ID3D11SamplerState* samplerState);

	public:
		
		static SamplerStateD3D11Ptr Create(ID3D11SamplerState* samplerState);

		//--------------------------------------------------------------------
		// IPlatformSamplerState Interfacec
		//--------------------------------------------------------------------
		void Bind(SHADER_TYPE shader, int slot);
		void SetDebugName(const char* name);

		//--------------------------------------------------------------------
		// OWN Interfacec
		//--------------------------------------------------------------------		
		ID3D11SamplerState* GetHardwareSamplerState() const;
	};
}