#pragma once
#ifndef _fastbird_RenderStructs_header_included_
#define _fastbird_RenderStructs_header_included_
#include <Engine/Renderer/RendererEnums.h>
#include <string>

namespace fastbird
{
	struct RASTERIZER_DESC
	{
		RASTERIZER_DESC()
			: FillMode(FILL_MODE_SOLID)
			, CullMode(CULL_MODE_BACK)
			, FrontCounterClockwise(false)
			, DepthBias(0)
			, DepthBiasClamp(0.f)
			, SlopeScaledDepthBias(0.f)
			, DepthClipEnable(true)
			, ScissorEnable(false)
			, MultisampleEnable(true)
			, AntialiasedLineEnable(true)
		{
		}

		bool operator<(const RASTERIZER_DESC& other) const
		{
			if (memcmp(this, &other, sizeof(RASTERIZER_DESC)) < 0)
				return true;
			
			return false;
		}

		FILL_MODE		FillMode;
		CULL_MODE		CullMode;
		bool            FrontCounterClockwise;
		int             DepthBias;
		float           DepthBiasClamp;
		float           SlopeScaledDepthBias;
		bool            DepthClipEnable;
		bool            ScissorEnable;
		bool            MultisampleEnable;
		bool            AntialiasedLineEnable;
	};

	struct SAMPLER_DESC
	{
		SAMPLER_DESC()
			: Filter(TEXTURE_FILTER_MIN_MAG_MIP_LINEAR)
			, AddressU(TEXTURE_ADDRESS_CLAMP)
			, AddressV(TEXTURE_ADDRESS_CLAMP)
			, AddressW(TEXTURE_ADDRESS_CLAMP)
			, MipLODBias(0.f)
			, MaxAnisotropy(16)
			, ComparisonFunc(COMPARISON_ALWAYS)
			, MinLOD(-FLT_MAX)
			, MaxLOD(FLT_MAX)

		{
			BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 0.f;
		}

		bool operator<(const SAMPLER_DESC& other) const
		{
			if (memcmp(this, &other, sizeof(SAMPLER_DESC)) < 0)
				return true;
			
			return false;
		}

		TEXTURE_FILTER       Filter;
		TEXTURE_ADDRESS_MODE AddressU;
		TEXTURE_ADDRESS_MODE AddressV;
		TEXTURE_ADDRESS_MODE AddressW;
		float                MipLODBias;
		unsigned int         MaxAnisotropy;
		COMPARISON_FUNC      ComparisonFunc;
		float                BorderColor[4];
		float                MinLOD;
		float                MaxLOD;
	};

	struct RENDER_TARGET_BLEND_DESC
	{
		bool			BlendEnable;
		BLEND			SrcBlend;
		BLEND			DestBlend;
		BLEND_OP		BlendOp;
		BLEND			SrcBlendAlpha;
		BLEND			DestBlendAlpha;
		BLEND_OP		BlendOpAlpha;
		unsigned char	RenderTargetWriteMask;
	};

	struct BLEND_DESC
	{
		BLEND_DESC()
			:AlphaToCoverageEnable(false)
			, IndependentBlendEnable(false)
		{
			for (int i=0; i<8; i++)
			{
				RenderTarget[i].BlendEnable = false;
				RenderTarget[i].SrcBlend = BLEND_ONE;
				RenderTarget[i].DestBlend = BLEND_ZERO;
				RenderTarget[i].BlendOp = BLEND_OP_ADD;
				RenderTarget[i].SrcBlendAlpha = BLEND_ONE;
				RenderTarget[i].DestBlendAlpha = BLEND_ZERO;
				RenderTarget[i].BlendOpAlpha = BLEND_OP_ADD;
				RenderTarget[i].RenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
			}
		}

		bool operator<(const BLEND_DESC& other) const
		{
			if (memcmp(this, &other, sizeof(BLEND_DESC)) < 0)
				return true;
			
			return false;
		}

		bool						AlphaToCoverageEnable;
		bool						IndependentBlendEnable;
		RENDER_TARGET_BLEND_DESC	RenderTarget[8];
	};

	//------------------------------------------------------------------------
	struct DEPTH_STENCILOP_DESC
	{
		DEPTH_STENCILOP_DESC()
			: StencilFailOp(STENCIL_OP_KEEP)
			, StencilDepthFailOp(STENCIL_OP_KEEP)
			, StencilPassOp(STENCIL_OP_KEEP)
			, StencilFunc(COMPARISON_ALWAYS)
		{
			
		}

		STENCIL_OP StencilFailOp;
		STENCIL_OP StencilDepthFailOp;
		STENCIL_OP StencilPassOp;
		COMPARISON_FUNC StencilFunc;
	};

	//------------------------------------------------------------------------
	struct DEPTH_STENCIL_DESC
	{
		DEPTH_STENCIL_DESC()
			: DepthEnable(true)
			, DepthWriteMask(DEPTH_WRITE_MASK_ALL)
			, DepthFunc(COMPARISON_LESS)
			, StencilEnable(false)
			, StencilReadMask(0xff)
			, StencilWriteMask(0xff)
		{
		}

		bool operator<(const DEPTH_STENCIL_DESC& other) const
		{
			if (memcmp(this, &other, sizeof(DEPTH_STENCIL_DESC)) < 0)
				return true;
			
			return false;
		}

		bool DepthEnable;
		DEPTH_WRITE_MASK DepthWriteMask;
		COMPARISON_FUNC DepthFunc;
		bool StencilEnable;
		unsigned char StencilReadMask;
		unsigned char StencilWriteMask;
		DEPTH_STENCILOP_DESC FrontFace;
		DEPTH_STENCILOP_DESC BackFace;
	};

	struct RENDERER_FRAME_PROFILER
	{
		RENDERER_FRAME_PROFILER()
		{
			FrameRate = 30.0f;
			FrameRateDisplay = 30.0f;
			FrameRateDisplayUpdateTime = 0.0f;
		}
		void Clear()
		{
			NumDrawCall = 0;
			NumVertexCount = 0;

			NumDrawIndexedCall = 0;
			NumIndexCount = 0;
		}

		void UpdateFrameRate(float dt)
		{
			FrameRateDisplayUpdateTime += dt;
			dt = std::max(0.0000001f, dt);
			FrameRate = (FrameRate + 1.0f/dt)/2.0f;
			if (FrameRateDisplayUpdateTime>0.5f)
			{
				FrameRateDisplayUpdateTime = 0.f;
				FrameRateDisplay = FrameRate;
			}

		}

		unsigned int NumDrawCall;
		unsigned int NumVertexCount;
		
		unsigned int NumDrawIndexedCall;
		unsigned int NumIndexCount;

		float FrameRate;
		float FrameRateDisplay;
		float FrameRateDisplayUpdateTime;
	};

	struct INPUT_ELEMENT_DESC
	{
		INPUT_ELEMENT_DESC(const char* semantic, unsigned index,
			INPUT_ELEMENT_FORMAT format, unsigned inputSlot,
			unsigned alignedByteOffset, INPUT_CLASSIFICATION slotClass,
			unsigned instanceDataStepRate)
			: mSemanticName(semantic), mSemanticIndex(index)
			, mFormat(format), mInputSlot(inputSlot)
			, mAlignedByteOffset(alignedByteOffset), mInputSlotClass(slotClass)
			, mInstanceDataStepRate(instanceDataStepRate)
		{
		}
		INPUT_ELEMENT_DESC()
		{
		}
		bool operator==(const INPUT_ELEMENT_DESC& other) const
		{
			return mSemanticName == other.mSemanticName &&
				mSemanticIndex == other.mSemanticIndex &&
				mFormat == other.mFormat &&
				mInputSlot == other.mInputSlot &&
				mAlignedByteOffset == other.mAlignedByteOffset &&
				mInputSlotClass == other.mInputSlotClass &&
				mInstanceDataStepRate == other.mInstanceDataStepRate;
		}

		bool operator< (const INPUT_ELEMENT_DESC& other) const
		{
			int cmp = mSemanticName.compare(other.mSemanticName);

			if (cmp < 0)
				return true;
			else if (cmp==0)
			{
				if (mSemanticIndex < other.mSemanticIndex)
					return true;
				else if (mSemanticIndex == other.mSemanticIndex)
				{
					if (mFormat < other.mFormat)
						return true;
					else if (mFormat == other.mFormat)
					{
						if (mInputSlot < other.mInputSlot)
							return true;
						else if (mInputSlot == other.mInputSlot)
						{
							if (mAlignedByteOffset < other.mAlignedByteOffset)
								return true;
							else if (mAlignedByteOffset == other.mAlignedByteOffset)
							{
								if (mInputSlotClass < other.mInputSlotClass)
									return true;
								else if (mInputSlotClass == other.mInputSlotClass)
								{
									if (mInstanceDataStepRate < other.mInstanceDataStepRate)
										return true;
								}
							}
						}
					}
				}
			}
			return false;
		}

		std::string mSemanticName;
		unsigned mSemanticIndex;
		INPUT_ELEMENT_FORMAT mFormat;
		unsigned mInputSlot;
		unsigned mAlignedByteOffset;
		INPUT_CLASSIFICATION mInputSlotClass;
		unsigned mInstanceDataStepRate;
	};

	typedef std::vector<INPUT_ELEMENT_DESC> INPUT_ELEMENT_DESCS;

	struct MapData
	{
		MapData() : pData(0){}
		void* pData;
		unsigned RowPitch;
		unsigned DepthPitch;
	};

	struct Viewport
	{
		float mTopLeftX;
		float mTopLeftY;
		float mWidth;
		float mHeight;
		float mMinDepth;
		float mMaxDepth;
	};

	struct Box3D
	{
		UINT left;
		UINT top;
		UINT front;
		UINT right;
		UINT bottom;
		UINT back;
	};

	namespace DEFAULT_MATERIALS{ enum Enum {QUAD,BILLBOARDQUAD, COUNT}; }
	namespace DEFAULT_INPUTS{ enum Enum {
		POSITION,
		POSITION_COLOR,
		POSITION_HDR_COLOR,
		POSITION_NORMAL,
		POSITION_TEXCOORD,
		POSITION_COLOR_TEXCOORD_BLENDINDICES,
		POSITION_NORMAL_TEXCOORD,
		POSITION_VEC4,
		POSITION_VEC4_COLOR,
		COUNT,
	};

	struct POSITION_V
	{
		POSITION_V() {}
		POSITION_V(const fastbird::Vec3& _p) : p(_p) { }
		fastbird::Vec3 p;
	};
	typedef POSITION_V V_P;

	struct POSITION_COLOR_V
	{
		POSITION_COLOR_V() {}
		POSITION_COLOR_V(const fastbird::Vec3& _p, unsigned int _c)
			: p(_p), color(_c) {}
		fastbird::Vec3 p;
		unsigned int color;
	};
	typedef POSITION_COLOR_V V_PC;

	struct POSITION_HDRCOLOR_V
	{
		POSITION_HDRCOLOR_V() {}
		POSITION_HDRCOLOR_V(const fastbird::Vec3& _p, const fastbird::Color& _c)
			: p(_p), color(_c) {}
		fastbird::Vec3 p;
		fastbird::Color color;
	};
	typedef POSITION_HDRCOLOR_V V_PhC;

	struct POSITION_NORMAL_V
	{
		POSITION_NORMAL_V() {}
		POSITION_NORMAL_V(const fastbird::Vec3& _p, const fastbird::Vec3& _n)
			: p(_p), n(_n) {}
		fastbird::Vec3 p;
		fastbird::Vec3 n;
	};
	typedef POSITION_NORMAL_V V_PN;

	struct POSITION_TEXCOORD_V
	{
		POSITION_TEXCOORD_V(){}
		POSITION_TEXCOORD_V(const fastbird::Vec3& _p, const fastbird::Vec2& _uv)
			: p(_p), uv(_uv) {}
		fastbird::Vec3 p;
		fastbird::Vec2 uv;
	};
	typedef POSITION_TEXCOORD_V V_PT;

	struct POSITION_COLOR_TEXCOORD_BLENDINDICES_V
	{
		POSITION_COLOR_TEXCOORD_BLENDINDICES_V(){}
		POSITION_COLOR_TEXCOORD_BLENDINDICES_V(const fastbird::Vec3& _p, unsigned int _c,
			const fastbird::Vec2& _uv, unsigned int _bindex)
			: p(_p), color(_c), uv(_uv), bindex(_bindex){}
		fastbird::Vec3 p;		// 12
		unsigned int color;		// 4
		fastbird::Vec2 uv;		// 8
		unsigned int bindex;	// 4
	};
	typedef POSITION_COLOR_TEXCOORD_BLENDINDICES_V V_PCTB;

	struct POSITION_NORMAL_TEXCOORD_V
	{
		POSITION_NORMAL_TEXCOORD_V() {}
		POSITION_NORMAL_TEXCOORD_V(const fastbird::Vec3& _p, const fastbird::Vec3& _n, 
			const fastbird::Vec2 _uv)
			: p(_p), n(_n),  uv(_uv){}

		bool operator==(const POSITION_NORMAL_TEXCOORD_V& other) const
		{
			return p==other.p && n==other.n && uv==other.uv;
		}

		bool operator<(const POSITION_NORMAL_TEXCOORD_V& other) const
		{
			return memcmp(this, &other, sizeof(POSITION_NORMAL_TEXCOORD_V)) < 0;
		}
		fastbird::Vec3 p;	// 12
		fastbird::Vec3 n;	// 12
		fastbird::Vec2 uv;	// 8
	};
	typedef POSITION_NORMAL_TEXCOORD_V V_PNT;

	struct POSITION_VEC4_V
	{
		POSITION_VEC4_V(){}
		POSITION_VEC4_V(const fastbird::Vec3& _p, const fastbird::Vec4& _v4)
			: p(_p), v4(_v4){}
		fastbird::Vec3 p;		// 12
		fastbird::Vec4 v4;		// 4
	};
	typedef POSITION_VEC4_V V_PV4;

	struct POSITION_VEC4_COLOR_V
	{
		POSITION_VEC4_COLOR_V(){}
		POSITION_VEC4_COLOR_V(const fastbird::Vec3& _p, const fastbird::Vec4& _v4, DWORD _color)
			: p(_p), v4(_v4), color(_color){}
		fastbird::Vec3 p;		// 12
		fastbird::Vec4 v4;		// 28
		DWORD color;			// 32
	};
	typedef POSITION_VEC4_COLOR_V V_PV4C;


	}
}
#endif //_fastbird_RenderStructs_header_included_