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

#include "stdafx.h"
#include "TextureAtlas.h"
#include "Renderer.h"
#include "Texture.h"
#include "TinyXmlLib/tinyxml2.h"
#include "FBStringLib/StringLib.h"

namespace fb{
	void TextureAtlasRegion::GetQuadUV(Vec2 uv[])
	{
		uv[0] = Vec2(mUVStart.x, mUVEnd.y);
		uv[1] = mUVStart;
		uv[2] = mUVEnd;
		uv[3] = Vec2(mUVEnd.x, mUVStart.y);
	}

	const Vec2& TextureAtlasRegion::GetStartUV() const { 
		return mUVStart; 
	}

	const Vec2 TextureAtlasRegion::GetUVSize() const { 
		return mUVEnd - mUVStart; 
	}
	const Vec2I& TextureAtlasRegion::GetSize() const {
		return mSize;
	}

	//-----------------------------------------------------------------------
	FB_IMPLEMENT_STATIC_CREATE(TextureAtlas);
	TextureAtlas::TextureAtlas(){

	}

	TextureAtlas::~TextureAtlas(){

	}

	TextureAtlasRegionPtr TextureAtlas::AddRegion(const char* name)
	{
		if (!name || strlen(name) == 0){
			Logger::Log(FB_DEFAULT_LOG_ARG, "invalid arg");
			return 0;
		}
		auto it = mRegions.Find(name);
		if (it != mRegions.end()){
			Logger::Log(FB_DEFAULT_LOG_ARG, "already existing. returning it.");
			return it->second;
		}

		TextureAtlasRegionPtr region(FB_NEW(TextureAtlasRegion), [](TextureAtlasRegion* obj){ FB_DELETE(obj); });
		region->mName = name;
		mRegions.Insert(std::make_pair(region->mName, region));
		return region;
	}

	TextureAtlasRegionPtr TextureAtlas::GetRegion(const char* name)
	{
		if (!name || strlen(name) == 0){
			Logger::Log(FB_DEFAULT_LOG_ARG, "invalid arg");
			return 0;
		}

		REGION_MAP::iterator it = mRegions.Find(name);
		if (it != mRegions.end())
		{
			return it->second;
		}
		else
		{
			Logger::Log(FB_DEFAULT_LOG_ARG, 
				FormatString("region name(%s) is not found", name).c_str());
			return 0;
		}
	}

	TextureAtlasRegionPtr TextureAtlas::GetRegion(size_t index) {
		return (mRegions.begin() + index)->second;
	}

	size_t TextureAtlas::GetNumRegions() const {
		return mRegions.size();
	}
	
	void TextureAtlas::SetTexture(TexturePtr texture){
		mTexture = texture;
	}

	TexturePtr TextureAtlas::GetTexture() const{
		return mTexture;
	}

	void TextureAtlas::SetPath(const char* path){
		if (!ValidCString(path)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
			return;
		}
		mPath = path;
	}

	const char* TextureAtlas::GetPath() const{
		return mPath.c_str();
	}

	bool TextureAtlas::ReloadTextureAtlas()
	{
		auto& renderer = Renderer::GetInstance();		
		tinyxml2::XMLDocument doc;
		doc.LoadFile(mPath.c_str());
		if (doc.Error()) {
			const char* errMsg = doc.GetErrorStr1();
			if (errMsg)
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("texture atlas error : \t%s", errMsg).c_str());
			else
				Logger::Log(FB_ERROR_LOG_ARG, "ReloadTextureAtlas Failed");
			return false;
		}
		tinyxml2::XMLElement* pRoot = doc.FirstChildElement("TextureAtlas");
		if (!pRoot) {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid TextureAtlas format!");
			return false;
		}

		const char* szBuffer = pRoot->Attribute("file");
		TexturePtr texture;
		if (szBuffer) {
			TextureCreationOption option;
			option.generateMip = false;
			mTexture = renderer.CreateTexture(szBuffer, option);
			if (!mTexture)
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Texture %s not found.", szBuffer).c_str());
			}
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid TextureAtlas format! No Texture Defined.");
			return false;
		}

		Vec2I textureSize = mTexture->GetSize();
		if (textureSize.x != 0 && textureSize.y != 0) {
			tinyxml2::XMLElement* pRegionElem = pRoot->FirstChildElement("region");
			while (pRegionElem) {
				szBuffer = pRegionElem->Attribute("name");
				if (!szBuffer)
				{
					Logger::Log(FB_ERROR_LOG_ARG, "No name for texture atlas region");
					continue;
				}
				TextureAtlasRegionPtr region = AddRegion(szBuffer);
				if (!region)
				{
					continue;					
				}
				region->mID = pRegionElem->UnsignedAttribute("id");
				region->mStart.x = pRegionElem->IntAttribute("x");
				region->mStart.y = pRegionElem->IntAttribute("y");
				region->mSize.x = pRegionElem->IntAttribute("width");
				region->mSize.y = pRegionElem->IntAttribute("height");
				Vec2 start((float)region->mStart.x, (float)region->mStart.y);
				Vec2 end(start.x + region->mSize.x, start.y + region->mSize.y);
				region->mUVStart = start / textureSize;
				region->mUVEnd = end / textureSize;
				pRegionElem = pRegionElem->NextSiblingElement();
			}
			return true;
		}
		else {
			Logger::Log(FB_ERROR_LOG_ARG, "Texture size is 0,0");
			return false;
		}
	}
}