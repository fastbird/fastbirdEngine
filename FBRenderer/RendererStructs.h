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
#ifndef _fastbird_RenderStructs_header_included_
#define _fastbird_RenderStructs_header_included_
#include "RendererEnums.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBMathLib/Color.h"
#include "FBMathLib/Math.h"
#include "FBStringLib/MurmurHash.h"

namespace fb
{
	struct RASTERIZER_DESC
	{
	private:
		mutable size_t	HashValue;
		FILL_MODE		FillMode;
		CULL_MODE		CullMode;
		int             DepthBias;
		float           DepthBiasClamp;
		float           SlopeScaledDepthBias;
		bool            FrontCounterClockwise;
		bool            DepthClipEnable;
		bool            ScissorEnable;
		bool            MultisampleEnable;
		bool            AntialiasedLineEnable;
		mutable bool		HashDirty;
		char			padding[2];

	public:
		RASTERIZER_DESC()
			: HashValue(false)
			, FillMode(FILL_MODE_SOLID)
			, CullMode(CULL_MODE_BACK)			
			, DepthBias(0)
			, DepthBiasClamp(0.f)
			, SlopeScaledDepthBias(0.f)
			, FrontCounterClockwise(false)
			, DepthClipEnable(true)
			, ScissorEnable(false)
			, MultisampleEnable(true)
			, AntialiasedLineEnable(true)
			, HashDirty(true)
		{
			memset(padding, 0, 2);
		}

		void SetFillMode(FILL_MODE fillmode) {
			FillMode = fillmode;
			HashDirty = true;
		}
		FILL_MODE GetFillMode() const { 
			return FillMode; }

		void SetCullMode(CULL_MODE cullmode) {
			CullMode = cullmode;
			HashDirty = true;
		}
		CULL_MODE GetCullMode() const {
			return CullMode;
		}

		void SetDepthBias(int depthBias) {
			DepthBias = depthBias;
			HashDirty = true;
		}
		int GetDepthBias() const {
			return DepthBias;
		}

		void SetDepthBiasClamp(float clamp) {
			DepthBiasClamp = clamp;
			HashDirty = true;
		}
		float GetDepthBiasClamp() const {
			return DepthBiasClamp;
		}

		void SetSlopeScaledDepthBias(float slope) {
			SlopeScaledDepthBias = slope;
			HashDirty = true;
		}
		float GetSlopeScaledDepthBias() const {
			return SlopeScaledDepthBias;
		}

		void SetFrontCounterClockwise(bool counter) {
			FrontCounterClockwise = counter;
			HashDirty = true;
		}
		bool GetFrontCounterClockwise() const {
			return FrontCounterClockwise;
		}

		void SetDepthClipEnable(bool depthclip) {
			DepthClipEnable = depthclip;
			HashDirty = true;
		}
		bool GetDepthClipEnable() const {
			return DepthClipEnable;
		}

		void SetScissorEnable(bool scissor) {
			ScissorEnable = scissor;
			HashDirty = true;
		}
		bool GetScissorEnable() const {
			return ScissorEnable;
		}

		void SetMultisampleEnable(bool multisample) {
			MultisampleEnable = multisample;
			HashDirty = true;
		}
		bool GetMultisampleEnable() const {
			return MultisampleEnable;
		}

		void SetAntialiasedLineEnable(bool antializedLine) {
			AntialiasedLineEnable = antializedLine;
			HashDirty = true;
		}
		bool GetAntialiasedLineEnable() const {
			return AntialiasedLineEnable;
		}

		bool operator<(const RASTERIZER_DESC& other) const
		{
			return Hash() < other.Hash();
		}

		bool operator==(const RASTERIZER_DESC& other) const
		{
			return Hash() == other.Hash();
		}

		bool operator!=(const RASTERIZER_DESC& other) const
		{
			return !operator==(other);
		}

		size_t Hash() const {
			if (HashDirty) {
				HashDirty = false;
				HashValue = 0;
				HashValue = murmur3_32((const char*)this, sizeof(RASTERIZER_DESC));
			}
			return HashValue;
		}
	};

	struct SAMPLER_DESC
	{
	private:
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
		mutable size_t				HashValue;
		mutable bool					HashDirty;

	public:
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
			, HashDirty(true)

		{
			BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 0.f;
		}

		void SetFilter(TEXTURE_FILTER filter) {
			Filter = filter;
			HashDirty = true;
		}
		TEXTURE_FILTER GetFilter() const {
			return Filter;
		}

		void SetAddressU(TEXTURE_ADDRESS_MODE addressMode) {
			AddressU = addressMode;
			HashDirty = true;
		}
		TEXTURE_ADDRESS_MODE GetAddressU() const {
			return AddressU;
		}

		void SetAddressV(TEXTURE_ADDRESS_MODE addressMode) {
			AddressV = addressMode;
			HashDirty = true;
		}
		TEXTURE_ADDRESS_MODE GetAddressV() const {
			return AddressV;
		}

		void SetAddressW(TEXTURE_ADDRESS_MODE addressMode) {
			AddressW = addressMode;
			HashDirty = true;
		}
		TEXTURE_ADDRESS_MODE GetAddressW() const {
			return AddressW;
		}

		void SetMipLODBias(float lodBias) {
			MipLODBias = lodBias;
			HashDirty = true;
		}
		float GetMipLODBias() const {
			return MipLODBias;
		}

		void SetMaxAnisotropy(unsigned int maxAni) {
			MaxAnisotropy = maxAni;
			HashDirty = true;
		}
		unsigned int GetMaxAnisotropy() const {
			return MaxAnisotropy;
		}

		void SetComparisonFunc(COMPARISON_FUNC func) {
			ComparisonFunc = func;
			HashDirty = true;
		}
		COMPARISON_FUNC GetComparisonFunc() const {
			return ComparisonFunc;
		}

		void SetBorderColor(float color[4]) {
			memcpy(BorderColor, color, sizeof(float) * 4);
			HashDirty = true;
		}
		const float* GetBorderColor() const {
			return BorderColor;
		}

		void SetMinLOD(float minLod) {
			MinLOD = minLod;
			HashDirty = true;
		}
		float GetMinLOD() const {
			return MinLOD;
		}

		void SetMaxLOD(float maxLod) {
			MaxLOD = maxLod;
			HashDirty = true;
		}
		float GetMaxLOD() const {
			return MaxLOD;
		}

		bool operator<(const SAMPLER_DESC& other) const
		{
			return Hash() < other.Hash();
		}

		bool operator == (const SAMPLER_DESC& other) const {
			return Hash() == other.Hash();
		}

		size_t Hash() const {
			if (HashDirty) {
				HashDirty = false;
				std::hash<int> intHasher;
				std::hash<float> floatHasher;
				HashValue = intHasher(Filter);
				hash_combine(HashValue, intHasher(AddressU));
				hash_combine(HashValue, intHasher(AddressV));
				hash_combine(HashValue, intHasher(AddressW));
				hash_combine(HashValue, floatHasher(MipLODBias));
				hash_combine(HashValue, std::hash<unsigned int>()(MaxAnisotropy));
				hash_combine(HashValue, intHasher(ComparisonFunc));
				hash_combine(HashValue, floatHasher(BorderColor[0]));
				hash_combine(HashValue, floatHasher(BorderColor[1]));
				hash_combine(HashValue, floatHasher(BorderColor[2]));
				hash_combine(HashValue, floatHasher(BorderColor[3]));
				hash_combine(HashValue, floatHasher(MinLOD));
				hash_combine(HashValue, floatHasher(MaxLOD));				
			}
			return HashValue;
		}
	};

	struct RENDER_TARGET_BLEND_DESC
	{
		RENDER_TARGET_BLEND_DESC()
			: SrcBlend(BLEND_ONE)
			, DestBlend(BLEND_ZERO)
			, BlendOp(BLEND_OP_ADD)
			, SrcBlendAlpha(BLEND_ONE)
			, DestBlendAlpha(BLEND_ZERO)
			, BlendOpAlpha(BLEND_OP_ADD)
			, RenderTargetWriteMask(COLOR_WRITE_MASK_ALL)
			, BlendEnable(false)
		{
			memset(padding, 0, 2);
		}
		BLEND			SrcBlend;
		BLEND			DestBlend;
		BLEND_OP		BlendOp;
		BLEND			SrcBlendAlpha;
		BLEND			DestBlendAlpha;
		BLEND_OP		BlendOpAlpha;
		unsigned char	RenderTargetWriteMask;
		bool			BlendEnable;
		char			padding[2];
	};

	struct BLEND_DESC
	{
		BLEND_DESC()
			:AlphaToCoverageEnable(false)
			, IndependentBlendEnable(false)
		{
			memset(padding, 0, 2);
			for (int i = 0; i < 8; i++)
			{
				RenderTarget[i].BlendEnable = false;
				RenderTarget[i].SrcBlend = BLEND_ONE;
				RenderTarget[i].DestBlend = BLEND_ZERO;
				RenderTarget[i].BlendOp = BLEND_OP_ADD;
				RenderTarget[i].SrcBlendAlpha = BLEND_ONE;
				RenderTarget[i].DestBlendAlpha = BLEND_ZERO;
				RenderTarget[i].BlendOpAlpha = BLEND_OP_ADD;
				RenderTarget[i].RenderTargetWriteMask = COLOR_WRITE_MASK_ALL;
			}
		}

		bool operator<(const BLEND_DESC& other) const
		{
			return memcmp(this, &other, sizeof(BLEND_DESC)) < 0;
		}

		bool operator==(const BLEND_DESC& other) const {
			return memcmp(this, &other, sizeof(BLEND_DESC)) == 0;
		}

		bool operator!=(const BLEND_DESC& other) const {
			return !operator==(other);
		}

		bool						AlphaToCoverageEnable;
		bool						IndependentBlendEnable;
		char						padding[2];
		RENDER_TARGET_BLEND_DESC	RenderTarget[8];
	};
}

namespace std {
	template<>
	struct hash<fb::BLEND_DESC>
		: public _Bitwise_hash<fb::BLEND_DESC>
	{
		typedef fb::BLEND_DESC _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return fb::murmur3_32((const char*)&_Keyval, sizeof(fb::BLEND_DESC));
		}
	};
}

namespace fb
{
	//------------------------------------------------------------------------
	struct DEPTH_STENCILOP_DESC
	{
	private:
		STENCIL_OP			StencilFailOp;
		STENCIL_OP			StencilDepthFailOp;
		STENCIL_OP			StencilPassOp;
		COMPARISON_FUNC StencilFunc;
		mutable size_t HashValue;
		mutable bool HashDirty;

	public:

		DEPTH_STENCILOP_DESC()
			: StencilFailOp(STENCIL_OP_KEEP)
			, StencilDepthFailOp(STENCIL_OP_KEEP)
			, StencilPassOp(STENCIL_OP_KEEP)
			, StencilFunc(COMPARISON_ALWAYS)
			, HashDirty(true)
		{
			
		}

		void SetStencilFailOp(STENCIL_OP op) {
			StencilFailOp = op;
			HashDirty = true;
		}	
		STENCIL_OP GetStencilFailOp() const {
			return StencilFailOp;
		}

		void SetStencilDepthFailOp(STENCIL_OP op) {
			StencilDepthFailOp = op;
			HashDirty = true;
		}
		STENCIL_OP GetStencilDepthFailOp() const {
			return StencilDepthFailOp;
		}

		void SetStencilPassOp(STENCIL_OP op) {
			StencilPassOp = op;
			HashDirty = true;
		}
		STENCIL_OP GetStencilPassOp() const {
			return StencilPassOp;
		}

		void SetStencilFunc(COMPARISON_FUNC func) {
			StencilFunc = func;
			HashDirty = true;
		}
		COMPARISON_FUNC GetStencilFunc() const {
			return StencilFunc;
		}

		size_t Hash() const {
			if (HashDirty) {
				HashDirty = false;
				HashValue = std::hash<int>()(StencilFailOp);
				hash_combine(HashValue, std::hash<int>()(StencilDepthFailOp));
				hash_combine(HashValue, std::hash<int>()(StencilPassOp));
				hash_combine(HashValue, std::hash<int>()(StencilFunc));
			}
			return HashValue;
		}		
	};

	//------------------------------------------------------------------------
	// Don't forget to call ComputeHash when you change any member variables.
	// However when you pass DESC to the Renderer to create a DepthStencilSTate
	// do not need to call ComputeHash(). The Renderer calls it.
	struct DEPTH_STENCIL_DESC
	{
	private:
		DEPTH_WRITE_MASK			DepthWriteMask;
		COMPARISON_FUNC				DepthFunc;
		DEPTH_STENCILOP_DESC	FrontFace;
		DEPTH_STENCILOP_DESC	BackFace;
		unsigned char					StencilReadMask;
		unsigned char					StencilWriteMask;
		bool									DepthEnable;
		bool									StencilEnable;
		mutable bool HashDirty;
		mutable size_t HashValue;

	public:
		DEPTH_STENCIL_DESC()
			: DepthEnable(true)
			, DepthWriteMask(DEPTH_WRITE_MASK_ALL)
			, DepthFunc(COMPARISON_LESS)
			, StencilEnable(false)
			, StencilReadMask(0xff)
			, StencilWriteMask(0xff)
			, HashDirty(true)
		{
		}

		void SetDepthWriteMask(DEPTH_WRITE_MASK writemask) {
			DepthWriteMask = writemask;
			HashDirty = true;
		}
		DEPTH_WRITE_MASK GetDepthWriteMask() const {
			return DepthWriteMask;
		}

		void SetDepthFunc(COMPARISON_FUNC func) {
			DepthFunc = func;
			HashDirty = true;
		}
		COMPARISON_FUNC GetDepthFunc() const {
			return DepthFunc;
		}

		void SetFrontFace(DEPTH_STENCILOP_DESC stencilop) {
			FrontFace = stencilop;
		}
		DEPTH_STENCILOP_DESC GetFrontFace() const {
			return FrontFace;
		}
		DEPTH_STENCILOP_DESC& GetFrontFace() {
			HashDirty = true;
			return FrontFace;
		}

		void SetBackFace(DEPTH_STENCILOP_DESC stencilop) {
			BackFace = stencilop;
			HashDirty = true;
		}
		DEPTH_STENCILOP_DESC GetBackFace() const {
			return BackFace;
		}
		DEPTH_STENCILOP_DESC& GetBackFace() {
			HashDirty = true;
			return BackFace;
		}

		void SetStencilReadMask(unsigned char mask) {
			StencilReadMask = mask;
			HashDirty = true;
		}
		unsigned char GetStencilReadMask() const {
			return StencilReadMask;
		}

		void SetStencilWriteMask(unsigned char mask) {
			StencilWriteMask = mask;
			HashDirty = true;
		}
		unsigned char GetStencilWriteMask() const {
			return StencilWriteMask;
		}

		void SetDepthEnable(bool depthEnable) {
			DepthEnable = depthEnable;
			HashDirty = true;
		}
		bool GetDepthEnable() const {
			return DepthEnable;
		}

		void SetStencilEnable(bool stencilEnable) {
			StencilEnable = stencilEnable;
			HashDirty = true;
		}
		bool GetStencilEnable() const {
			return StencilEnable;
		}

		bool operator<(const DEPTH_STENCIL_DESC& other) const
		{
			return Hash() < other.Hash();
		}

		bool operator==(const DEPTH_STENCIL_DESC& other) const
		{
			return Hash() == other.Hash();
		}

		bool operator!=(const DEPTH_STENCIL_DESC& other) const
		{
			return !operator==(other);
		}

		size_t Hash() const {
			if (HashDirty) {
				HashDirty = false;
				HashValue = std::hash<int>()(DepthWriteMask);
				hash_combine(HashValue, std::hash<int>()(DepthFunc));
				hash_combine(HashValue, std::hash<int>()(DepthFunc));
				hash_combine(HashValue, FrontFace.Hash());
				hash_combine(HashValue, BackFace.Hash());
				hash_combine(HashValue, std::hash<unsigned char>()(StencilReadMask));
				hash_combine(HashValue, std::hash<unsigned char>()(StencilWriteMask));
				hash_combine(HashValue, std::hash<bool>()(DepthEnable));
				hash_combine(HashValue, std::hash<bool>()(StencilEnable));
			}
			return HashValue;
		}	
	};

	struct RENDERER_FRAME_PROFILER
	{
		RENDERER_FRAME_PROFILER()
		{
			FrameRate = 30.0f;
			FrameRateDisplay = 30.0f;
			FrameRateDisplayUpdateTime = 0.0f;
			mLastDrawTakes = 0.f;
		}
		void Clear()
		{
			NumDrawCall = 0;
			NumVertexCount = 0;

			NumIndexedDrawCall = 0;
			NumIndexCount = 0;
			NumUpdateObjectConst = 0;
		}

		void UpdateFrameRate(TIME_PRECISION rendererDt, TIME_PRECISION totalDt)
		{
			mLastDrawTakes = rendererDt;
			FrameRateDisplayUpdateTime += totalDt;
			totalDt = std::max((TIME_PRECISION)0.0000001f, totalDt);
			FrameRate = (FrameRate + 1.0f / (Real)totalDt) / 2.0f;
			if (FrameRateDisplayUpdateTime>0.5f)
			{
				FrameRateDisplayUpdateTime = 0.f;
				FrameRateDisplay = FrameRate;
			}

		}

		unsigned int NumDrawCall;
		unsigned int NumVertexCount;
		
		unsigned int NumIndexedDrawCall;
		unsigned int NumIndexCount;

		Real FrameRate;
		Real FrameRateDisplay;
		TIME_PRECISION FrameRateDisplayUpdateTime;
		Real mLastDrawTakes;

		unsigned int NumUpdateObjectConst;
	};

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

	namespace DEFAULT_MATERIALS{ enum Enum {QUAD, QUAD_TEXTURE, BILLBOARDQUAD, COUNT}; }
	namespace DEFAULT_INPUTS{ enum Enum {
		POSITION,
		POSITION_COLOR,
		POSITION_COLOR_TEXCOORD,
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
		POSITION_V(const Vec3& _p) : p(_p) { }
		Vec3f p;
	};
	typedef POSITION_V V_P;

	struct POSITION_COLOR_V
	{
		POSITION_COLOR_V() {}
		POSITION_COLOR_V(const Vec3& _p, unsigned int _c)
			: p(_p), color(_c) {}
		Vec3f p;
		unsigned int color;
	};
	typedef POSITION_COLOR_V V_PC;

	struct POSITION_COLOR_TEXCOORD_V
	{
		POSITION_COLOR_TEXCOORD_V() {}
		POSITION_COLOR_TEXCOORD_V(const Vec3& _p, unsigned int _c, Vec2f _tex)
			: p(_p), color(_c), t(_tex) {}
		Vec3f p;
		unsigned int color;
		Vec2f t;
	};
	typedef POSITION_COLOR_TEXCOORD_V V_PCT;

	struct POSITION_HDRCOLOR_V
	{
		POSITION_HDRCOLOR_V() {}
		POSITION_HDRCOLOR_V(const Vec3& _p, const Color& _c)
			: p(_p), color(_c) {}
		Vec3f p;
		Color color;
	};
	typedef POSITION_HDRCOLOR_V V_PhC;

	struct POSITION_NORMAL_V
	{
		POSITION_NORMAL_V() {}
		POSITION_NORMAL_V(const Vec3& _p, const Vec3& _n)
			: p(_p), n(_n) {}
		Vec3f p;
		Vec3f n;
	};
	typedef POSITION_NORMAL_V V_PN;

	struct POSITION_TEXCOORD_V
	{
		POSITION_TEXCOORD_V(){}
		POSITION_TEXCOORD_V(const Vec3& _p, const Vec2& _uv)
			: p(_p), uv(_uv) {}
		Vec3f p;
		Vec2f uv;
	};
	typedef POSITION_TEXCOORD_V V_PT;

	struct POSITION_COLOR_TEXCOORD_BLENDINDICES_V
	{
		POSITION_COLOR_TEXCOORD_BLENDINDICES_V(){}
		POSITION_COLOR_TEXCOORD_BLENDINDICES_V(const Vec3& _p, unsigned int _c,
			const Vec2& _uv, unsigned int _bindex)
			: p(_p), color(_c), uv(_uv), bindex(_bindex){}
		Vec3f p;		// 12
		unsigned int color;		// 4
		Vec2f uv;		// 8
		unsigned int bindex;	// 4
	};
	typedef POSITION_COLOR_TEXCOORD_BLENDINDICES_V V_PCTB;

	struct POSITION_NORMAL_TEXCOORD_V
	{
		POSITION_NORMAL_TEXCOORD_V() {}
		POSITION_NORMAL_TEXCOORD_V(const Vec3& _p, const Vec3& _n, 
			const Vec2f _uv)
			: p(_p), n(_n),  uv(_uv){}

		bool operator==(const POSITION_NORMAL_TEXCOORD_V& other) const
		{
			return p==other.p && n==other.n && uv==other.uv;
		}

		bool operator<(const POSITION_NORMAL_TEXCOORD_V& other) const
		{
			return memcmp(this, &other, sizeof(POSITION_NORMAL_TEXCOORD_V)) < 0;
		}
		Vec3f p;	// 12
		Vec3f n;	// 12
		Vec2f uv;	// 8
	};
	typedef POSITION_NORMAL_TEXCOORD_V V_PNT;

	struct POSITION_VEC4_V
	{
		POSITION_VEC4_V(){}
		POSITION_VEC4_V(const Vec3& _p, const Vec4& _v4)
			: p(_p), v4(_v4){}
		Vec3f p;		// 12
		Vec4 v4;		// 4
	};
	typedef POSITION_VEC4_V V_PV4;

	struct POSITION_VEC4_COLOR_V
	{
		POSITION_VEC4_COLOR_V(){}
		POSITION_VEC4_COLOR_V(const Vec3& _p, const Vec4& _v4, DWORD _color)
			: p(_p), v4(_v4), color(_color){}
		Vec3f p;		// 12
		Vec4 v4;		// 28
		DWORD color;			// 32
	};
	typedef POSITION_VEC4_COLOR_V V_PV4C;


	}

	struct TextureCreationOption {
		TextureCreationOption() 
			: async(true)
			, generateMip(true)
			, textureType(TEXTURE_TYPE_DEFAULT)
		{
		}

		TextureCreationOption(bool async, bool generateMip)
			: async(async)
			, generateMip(generateMip)
		{
		}

		TextureCreationOption(bool async, bool generateMip, int textureType)
			: async(async)
			, generateMip(generateMip)
			, textureType(textureType)
		{
		}

		bool async;
		// if the texture source doesn't have mip data and this flag is true,
		// mipmap will be generated.
		// Textures for ui doesn't need to have mipmap.
		bool generateMip;
		int textureType;
	};
}

namespace std {
	template<>
	struct hash<fb::RASTERIZER_DESC>
		: public _Bitwise_hash<fb::RASTERIZER_DESC>
	{
		typedef fb::RASTERIZER_DESC _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};

	template<>
	struct hash<fb::DEPTH_STENCIL_DESC>
		: public _Bitwise_hash<fb::DEPTH_STENCIL_DESC>
	{
		typedef fb::DEPTH_STENCIL_DESC _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};	

	template<>
	struct hash<fb::SAMPLER_DESC>
		: public _Bitwise_hash<fb::SAMPLER_DESC>
	{
		typedef fb::SAMPLER_DESC _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};
}

#endif //_fastbird_RenderStructs_header_included_