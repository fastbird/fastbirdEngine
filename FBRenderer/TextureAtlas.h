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
#include "FBCommonHeaders/IteratorWrapper.h"
#include "FBMathLib/Math.h"
namespace fb{
	FB_DECLARE_SMART_PTR_STRUCT(TextureAtlasRegion);
	struct FB_DLL_RENDERER TextureAtlasRegion
	{
		DWORD mID;
		std::string mName;
		Vec2 mUVStart;
		Vec2 mUVEnd;
		Vec2I mStart;
		Vec2I mSize;

		void GetQuadUV(Vec2 uv[]);
		const Vec2& GetStartUV() const;
		const Vec2 GetUVSize() const;
		const Vec2I& GetSize() const;
	};

	//------------------------------------------------------------------------
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(TextureAtlas);
	class FB_DLL_RENDERER TextureAtlas
	{
		std::string mPath;
		TexturePtr mTexture;
		typedef std::unordered_map<std::string, TextureAtlasRegionPtr> REGION_MAP;
		REGION_MAP mRegions;
		
		TextureAtlas();

	public:
		static TextureAtlasPtr Create();
		~TextureAtlas();

		TextureAtlasRegionPtr AddRegion(const char* name);
		TextureAtlasRegionPtr GetRegion(const char* name);
		IteratorWrapper<REGION_MAP> GetIterator();
		void SetTexture(TexturePtr texture);
		TexturePtr GetTexture() const;
		void SetPath(const char* path);		
		const char* GetPath() const;
		bool ReloadTextureAtlas();		
	};
}