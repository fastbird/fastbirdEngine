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
#include "Texture.h"
#include "Renderer.h"
#include "IPlatformTexture.h"
#include "FBThread/AsyncObjects.h"
#include "FBMathLib/ColorRamp.h"
#include "FBCommonHeaders/VectorMap.h"

namespace fb{
static std::vector<TextureWeakPtr> sAllTextures;
FB_READ_WRITE_CS sAllTexturesLock;
TexturePtr GetTextureFromExistings(IPlatformTexturePtr platformShader) {
	READ_LOCK lock(sAllTexturesLock);
	for (auto it = sAllTextures.begin(); it != sAllTextures.end(); /**/){
		IteratingWeakContainer(sAllTextures, it, texture);
		if (texture->GetPlatformTexture() == platformShader){
			return texture;
		}
	}
	return 0;
}

size_t Texture::sNextTextureID = 0;
static VectorMap< SHADER_TYPE, VectorMap<int, TextureWeakPtr> > sBindedTextures;
void SetBindedTexture(SHADER_TYPE shader, int startSlot, TexturePtr pTextures[], int num){
	auto& shaderCategory = sBindedTextures[shader];
	int count = 0;
	for (int i = startSlot; i < startSlot + num; ++i){
		shaderCategory[i] = pTextures[count++];
	}
}

std::vector< std::pair<SHADER_TYPE, int> > FindSlotInfo(TexturePtr texture){
	std::vector< std::pair<SHADER_TYPE, int> > result;
	static int bindingShaders[] = {
		SHADER_TYPE_VS,
		SHADER_TYPE_HS,
		SHADER_TYPE_DS,
		SHADER_TYPE_GS,
		SHADER_TYPE_PS,
		SHADER_TYPE_CS
	};
	for (int shader = 0; shader < ARRAYCOUNT(bindingShaders); ++shader){
		auto& shaderCategory = sBindedTextures[(SHADER_TYPE)bindingShaders[shader]];
		for (auto it = shaderCategory.begin(); it != shaderCategory.end(); ++it){
			if (it->second.lock() == texture){
				result.push_back(std::make_pair((SHADER_TYPE)bindingShaders[shader], it->first));
			}
		}
	}
	return result;
}

class Texture::Impl{
public:
	TextureWeakPtr mSelf;
	unsigned mTextureID;
	IPlatformTexturePtr mPlatformTexture;	
	std::string mFilePath;
	int mType;
	//---------------------------------------------------------------------------
	Impl()
		: mTextureID(sNextTextureID++)
		, mType(TEXTURE_TYPE_DEFAULT)
	{
	}

	const char* GetFilePath() const{
		return mFilePath.c_str();
	}

	void SetFilePath(const char* path){
		if (path)
			mFilePath = path;
		else
			mFilePath.clear();
	}

	void SetType(int texture_type){
		mType = texture_type;
	}

	int GetType() const{
		return mType;
	}

	int GetWidth() const{
		return std::get<0>(mPlatformTexture->GetSize());
	}

	int GetHeight() const{
		return std::get<1>(mPlatformTexture->GetSize());
	}

	Vec2I GetSize() const{
		return mPlatformTexture->GetSize();
	}

	PIXEL_FORMAT GetFormat() const{
		return mPlatformTexture->GetPixelFormat();		
	}

	void SetDebugName(const char* name){
		mPlatformTexture->SetDebugName(name);
	}

	bool IsReady() const{
		return mPlatformTexture->IsReady();
	}

	void Bind(SHADER_TYPE shader, int slot) const{
		sBindedTextures[shader][slot] = mSelf.lock();
		mPlatformTexture->Bind(shader, slot);
	}

	void Unbind(){
		auto slots = FindSlotInfo(mSelf.lock());
		for (auto& it : slots){
			Renderer::GetInstance().UnbindTexture(it.first, it.second);
		}
	}

	MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const{
		return mPlatformTexture->Map(subResource, type, flag);
	}

	void Unmap(UINT subResource) const{
		return mPlatformTexture->Unmap(subResource);
	}

	TexturePtr CopyToStaging() {
		auto staging = Renderer::GetInstance().CreateTexture(0, GetWidth(), GetHeight(), GetFormat(),
			1, BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);
		if (!staging)
			return nullptr;
		mPlatformTexture->CopyToStaging(staging->GetPlatformTexture().get());
		return staging;
	}

	void CopyToStaging(TexturePtr dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ, 
		UINT srcSubresource, Box3D* srcBox) const{
		mPlatformTexture->CopyToStaging(dst->mImpl->mPlatformTexture.get(), 
			dstSubresource, dstX, dstY, dstZ, srcSubresource, srcBox);
	}

	void SaveToFile(const char* filename) const{
		mPlatformTexture->SaveToFile(filename);
	}

	void GenerateMips() {
		mPlatformTexture->GenerateMips();
	}

	void SetPlatformTexture(IPlatformTexturePtr platformTexture){
		mPlatformTexture = platformTexture;
	}

	IPlatformTexturePtr GetPlatformTexture() const{
		return mPlatformTexture;
	}

	size_t GetSizeInBytes() const {
		return mPlatformTexture->GetSizeInBytes();
	}

	bool UpdateColorRamp(ColorRamp& data, float noiseStrength) {
		if (!(mType & TEXTURE_TYPE_COLOR_RAMP)) {
			Logger::Log(FB_ERROR_LOG_ARG, "This texture is not a color ramp texture.");
			return false;
		}
		MapData dest= Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
		if (dest.pData)
		{
			auto width = GetWidth();
			// bar position is already updated. generate ramp texture data.
			data.GenerateColorRampTextureData(width, noiseStrength);
			unsigned int *pixels = (unsigned int*)dest.pData;
			for (int x = 0; x < width; x++)
			{
				pixels[x] = data[x].Get4Byte();
			}
			Unmap(0);
		}		
		return true;
	}
};

//---------------------------------------------------------------------------
TexturePtr Texture::Create(){
	auto p = TexturePtr(FB_NEW(Texture), [](Texture* obj){ FB_DELETE(obj); });
	
	{
		WRITE_LOCK lock(sAllTexturesLock);
		sAllTextures.push_back(p);
	}
	p->mImpl->mSelf = p;
	return p;
}

void Texture::ReloadTexture(const char* file){
	auto& renderer = Renderer::GetInstance();
	WRITE_LOCK lock(sAllTexturesLock);
	for (auto it = sAllTextures.begin(); it != sAllTextures.end(); /**/){
		IteratingWeakContainer(sAllTextures, it, texture);
		if (strcmp(texture->GetFilePath(), file)==0){
			renderer.ReloadTexture(texture, file);
		}
	}
}

Texture::Texture()
	: mImpl(new Impl){
}

Texture::~Texture(){
	WRITE_LOCK lock(sAllTexturesLock);
	auto itEnd = sAllTextures.end();
	for (auto it = sAllTextures.begin(); it != itEnd; it++){
		if (it->expired()){
			sAllTextures.erase(it);
			return;
		}
	}
}

//Texture& Texture::operator = (const Texture& other){
//	assert(0 && "not implemented");
//	return *this;
//}

size_t Texture::GetTextureID() const{
	return mImpl->mTextureID;
}

const char* Texture::GetFilePath() const{
	return mImpl->GetFilePath();
}

void Texture::SetFilePath(const char* filepath){
	mImpl->SetFilePath(filepath);
}

void Texture::SetType(int texture_type)
{
	mImpl->SetType(texture_type);
}

int Texture::GetType() const{
	return mImpl->GetType();
}

int Texture::GetWidth() const{
	return mImpl->GetWidth();
}

int Texture::GetHeight() const{
	return mImpl->GetHeight();
}

PIXEL_FORMAT Texture::GetFormat() const{
	return mImpl->GetFormat();
}

Vec2I Texture::GetSize(){
	return mImpl->GetSize();
}

void Texture::SetDebugName(const char* name){
	mImpl->SetDebugName(name);
}

bool Texture::IsReady() const{
	return mImpl->IsReady();
}

void Texture::Bind(SHADER_TYPE shader, int slot){
	mImpl->Bind(shader, slot);
}

void Texture::Unbind(){
	mImpl->Unbind();
}

MapData Texture::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag)
{
	return mImpl->Map(subResource, type, flag);	
}
void Texture::Unmap(UINT subResource)
{
	mImpl->Unmap(subResource);
}

TexturePtr Texture::CopyToStaging() {
	return mImpl->CopyToStaging();
}

void Texture::CopyToStaging(TexturePtr dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ,
	UINT srcSubresource, Box3D* srcBox){
	mImpl->CopyToStaging(dst, dstSubresource, dstX, dstY, dstZ, srcSubresource, srcBox);
}

void Texture::SaveToFile(const char* filename){
	mImpl->SaveToFile(filename);
}

void Texture::GenerateMips(){
	mImpl->GenerateMips();
}

void Texture::SetPlatformTexture(IPlatformTexturePtr platformTexture){
	mImpl->SetPlatformTexture(platformTexture);
}

IPlatformTexturePtr Texture::GetPlatformTexture() const{
	return mImpl->GetPlatformTexture();
}

bool Texture::GetMipGenerated() const {
	return mImpl->mPlatformTexture->GetMipGenerated();
}

size_t Texture::GetSizeInBytes() const {
	return mImpl->GetSizeInBytes();
}

bool Texture::UpdateColorRamp(ColorRamp& data, float noiseStrength) {
	return mImpl->UpdateColorRamp(data, noiseStrength);
}

}
