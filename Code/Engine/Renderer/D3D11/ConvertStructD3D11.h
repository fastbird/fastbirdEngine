#pragma once
#include <Engine/Renderer/RendererStructs.h>
#include <Engine/Renderer/D3D11/ConvertEnumD3D11.h>
#include <d3d11.h>
namespace fastbird
{
	void ConvertStructD3D11(D3D11_RASTERIZER_DESC& dest, const RASTERIZER_DESC& src)
	{
		dest.FillMode = ConvertEnumD3D11(src.FillMode);
		dest.CullMode = ConvertEnumD3D11(src.CullMode);
		dest.FrontCounterClockwise = src.FrontCounterClockwise;
		dest.DepthBias = src.DepthBias;
		dest.DepthBiasClamp = src.DepthBiasClamp;
		dest.SlopeScaledDepthBias = src.SlopeScaledDepthBias;
		dest.DepthClipEnable = src.DepthClipEnable;
		dest.ScissorEnable = src.ScissorEnable;
		dest.MultisampleEnable = src.MultisampleEnable;
		dest.AntialiasedLineEnable = src.AntialiasedLineEnable;	
	}

	void ConvertStructD3D11(D3D11_SAMPLER_DESC& dest, const SAMPLER_DESC& src)
	{
		dest.Filter = ConvertEnumD3D11(src.Filter);
		dest.AddressU = ConvertEnumD3D11(src.AddressU);
		dest.AddressV = ConvertEnumD3D11(src.AddressV);
		dest.AddressW = ConvertEnumD3D11(src.AddressW);
		dest.MipLODBias = src.MipLODBias;
		dest.MaxAnisotropy = src.MaxAnisotropy;
		dest.ComparisonFunc = ConvertEnumD3D11(src.ComparisonFunc);
		dest.BorderColor[0] = src.BorderColor[0];
		dest.BorderColor[1] = src.BorderColor[1];
		dest.BorderColor[2] = src.BorderColor[2];
		dest.BorderColor[3] = src.BorderColor[3];
		dest.MinLOD = src.MinLOD;
		dest.MaxLOD = src.MaxLOD;
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
		dest.RenderTargetWriteMask = ConvertEnumD3D11((COLOR_WRITE_ENABLE)src.RenderTargetWriteMask);
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
		desc.StencilFailOp = ConvertEnumD3D11(src.StencilFailOp);
		desc.StencilDepthFailOp = ConvertEnumD3D11(src.StencilDepthFailOp);
		desc.StencilPassOp = ConvertEnumD3D11(src.StencilPassOp);
		desc.StencilFunc = ConvertEnumD3D11(src.StencilFunc);
	}

	void ConvertStructD3D11(D3D11_DEPTH_STENCIL_DESC& desc, const DEPTH_STENCIL_DESC& src)
	{
		desc.DepthEnable = src.DepthEnable;
		desc.DepthWriteMask = ConvertEnumD3D11(src.DepthWriteMask);
		desc.DepthFunc = ConvertEnumD3D11(src.DepthFunc);
		desc.StencilEnable = src.StencilEnable;
		desc.StencilReadMask = src.StencilReadMask;
		desc.StencilWriteMask = src.StencilWriteMask;
		ConvertStructD3D11(desc.FrontFace, src.FrontFace);
		ConvertStructD3D11(desc.BackFace, src.BackFace);

	}

	//------------------------------------------------------------------------
	inline D3D11_INPUT_ELEMENT_DESC ConvertStructD3D11(const INPUT_ELEMENT_DESC& srcDesc)
	{
		D3D11_INPUT_ELEMENT_DESC destDesc;
		destDesc.SemanticName = srcDesc.mSemanticName.c_str();
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
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Undefined input format found!");
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
			IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Undefined input slot class found!");			
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