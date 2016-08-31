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
#include "FBRenderer/RendererStructs.h"
#include "ConvertEnumD3D11.h"

namespace fb
{
	void ConvertStructD3D11(D3D11_RASTERIZER_DESC& dest, const RASTERIZER_DESC& src)
	{
		dest.FillMode = ConvertEnumD3D11(src.GetFillMode());
		dest.CullMode = ConvertEnumD3D11(src.GetCullMode());
		dest.FrontCounterClockwise = src.GetFrontCounterClockwise();
		dest.DepthBias = src.GetDepthBias();
		dest.DepthBiasClamp = src.GetDepthBiasClamp();
		dest.SlopeScaledDepthBias = src.GetSlopeScaledDepthBias();
		dest.DepthClipEnable = src.GetDepthClipEnable();
		dest.ScissorEnable = src.GetScissorEnable();
		dest.MultisampleEnable = src.GetMultisampleEnable();
		dest.AntialiasedLineEnable = src.GetAntialiasedLineEnable();	
	}

	void ConvertStructD3D11(D3D11_SAMPLER_DESC& dest, const SAMPLER_DESC& src)
	{
		dest.Filter = ConvertEnumD3D11(src.GetFilter());
		dest.AddressU = ConvertEnumD3D11(src.GetAddressU());
		dest.AddressV = ConvertEnumD3D11(src.GetAddressV());
		dest.AddressW = ConvertEnumD3D11(src.GetAddressW());
		dest.MipLODBias = src.GetMipLODBias();
		dest.MaxAnisotropy = src.GetMaxAnisotropy();
		dest.ComparisonFunc = ConvertEnumD3D11(src.GetComparisonFunc());
		auto borderColor = src.GetBorderColor();
		dest.BorderColor[0] = borderColor[0];
		dest.BorderColor[1] = borderColor[1];
		dest.BorderColor[2] = borderColor[2];
		dest.BorderColor[3] = borderColor[3];
		dest.MinLOD = src.GetMinLOD();
		dest.MaxLOD = src.GetMaxLOD();
	}

	void ConvertStructD3D11(D3D11_RENDER_TARGET_BLEND_DESC& dest, const RENDER_TARGET_BLEND_DESC& src)
	{
		dest.BlendEnable = src.BlendEnable;
		dest.SrcBlend = ConvertEnumD3D11(src.SrcBlend);
		dest.DestBlend = ConvertEnumD3D11(src.DestBlend);
		dest.BlendOp = ConvertEnumD3D11(src.BlendOp);
		dest.SrcBlendAlpha = ConvertEnumD3D11(src.SrcBlendAlpha);
		dest.DestBlendAlpha = ConvertEnumD3D11(src.DestBlendAlpha);
		dest.BlendOpAlpha = ConvertEnumD3D11(src.BlendOpAlpha);
		dest.RenderTargetWriteMask = ConvertEnumD3D11((COLOR_WRITE_MASK)src.RenderTargetWriteMask);
	}

	void ConvertStructD3D11(D3D11_BLEND_DESC& dest, const BLEND_DESC& src)
	{
		dest.AlphaToCoverageEnable = src.AlphaToCoverageEnable;
		dest.IndependentBlendEnable = src.IndependentBlendEnable;
		for (int i=0; i<8; i++)
		{
			ConvertStructD3D11(dest.RenderTarget[i], src.RenderTarget[i]);
		}
	}

	void ConvertStructD3D11(D3D11_DEPTH_STENCILOP_DESC& desc, const DEPTH_STENCILOP_DESC& src)
	{
		desc.StencilFailOp = ConvertEnumD3D11(src.GetStencilFailOp());
		desc.StencilDepthFailOp = ConvertEnumD3D11(src.GetStencilDepthFailOp());
		desc.StencilPassOp = ConvertEnumD3D11(src.GetStencilPassOp());
		desc.StencilFunc = ConvertEnumD3D11(src.GetStencilFunc());
	}

	void ConvertStructD3D11(D3D11_DEPTH_STENCIL_DESC& desc, const DEPTH_STENCIL_DESC& src)
	{
		desc.DepthEnable = src.GetDepthEnable();
		desc.DepthWriteMask = ConvertEnumD3D11(src.GetDepthWriteMask());
		desc.DepthFunc = ConvertEnumD3D11(src.GetDepthFunc());
		desc.StencilEnable = src.GetStencilEnable();
		desc.StencilReadMask = src.GetStencilReadMask();
		desc.StencilWriteMask = src.GetStencilWriteMask();
		ConvertStructD3D11(desc.FrontFace, src.GetFrontFace());
		ConvertStructD3D11(desc.BackFace, src.GetBackFace());

	}

	//------------------------------------------------------------------------
	inline D3D11_INPUT_ELEMENT_DESC ConvertStructD3D11(const INPUT_ELEMENT_DESC& srcDesc)
	{
		D3D11_INPUT_ELEMENT_DESC destDesc;
		destDesc.SemanticName = srcDesc.mSemanticName;
		destDesc.SemanticIndex = srcDesc.mSemanticIndex;
		switch(srcDesc.mFormat)
		{
		case INPUT_ELEMENT_FORMAT_FLOAT4:
			destDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case INPUT_ELEMENT_FORMAT_FLOAT3:
			destDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case INPUT_ELEMENT_FORMAT_UBYTE4:
			destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case INPUT_ELEMENT_FORMAT_FLOAT2:
			destDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case INPUT_ELEMET_FORMAT_INT4:
			destDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Undefined input format found!");
			assert(0);
		}
		destDesc.InputSlot = srcDesc.mInputSlot;
		destDesc.AlignedByteOffset = srcDesc.mAlignedByteOffset;
		switch(srcDesc.mInputSlotClass)
		{
		case INPUT_CLASSIFICATION_PER_VERTEX_DATA:
			destDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			break;
		case INPUT_CLASSIFICATION_PER_INSTANCE_DATA:
			destDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			break;
		default:
			Logger::Log(FB_ERROR_LOG_ARG, "Undefined input slot class found!");
			assert(0);
		}
		destDesc.InstanceDataStepRate = srcDesc.mInstanceDataStepRate;
		return destDesc;
	}

	inline D3D11_VIEWPORT ConvertStructD3D11(const Viewport& src)
	{
		D3D11_VIEWPORT vp = {
			src.mTopLeftX, src.mTopLeftY,
			src.mWidth, src.mHeight,
			src.mMinDepth, src.mMaxDepth
		};
		return vp;
	}
}