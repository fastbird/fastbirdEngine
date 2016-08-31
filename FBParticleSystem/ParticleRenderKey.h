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
#include "FBCommonHeaders/Helpers.h"
#include "FBRenderer/RendererStructs.h"
namespace fb{
	class IScene;
	struct ParticleRenderKey{
		IScene* mScene;
		int mScreenspace;
		char mTexturePath[256];
		BLEND_DESC mBDesc;
		bool mGlow;
		bool mDepthFade;		
		char padding[2];

		ParticleRenderKey();
		ParticleRenderKey(IScene* scene, const char* texturePath, 
			const BLEND_DESC& desc, bool glow, bool depthFade, int screenspace);
		bool operator==(const ParticleRenderKey& other) const;
		bool operator<(const ParticleRenderKey& other) const;
	};
}

namespace std {
	template<>
	struct hash<fb::ParticleRenderKey>
		: public _Bitwise_hash<fb::ParticleRenderKey>
	{
		typedef fb::ParticleRenderKey _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			auto h = std::hash<void*>()(_Keyval.mScene);
			fb::hash_combine(h, std::hash<int>()(_Keyval.mScreenspace));
			fb::hash_combine(h, std::hash<std::string>()(std::string(_Keyval.mTexturePath)));
			fb::hash_combine(h, std::hash<fb::BLEND_DESC>()(_Keyval.mBDesc));
			fb::hash_combine(h, std::hash<bool>()(_Keyval.mGlow));
			fb::hash_combine(h, std::hash<bool>()(_Keyval.mDepthFade));		
			return h;
		}
	};
}