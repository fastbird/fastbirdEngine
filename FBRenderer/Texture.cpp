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
#include "FBCommonHeaders/VectorMap.h"

namespace fb{
static std::vector<TextureWeakPtr> sAllTextures;
TexturePtr GetTextureFromExistings(IPlatformTexturePtr platformShader) {
	for (auto it = sAllTextures.begin(); it != sAllTextures.end(); /**/){
		IteratingWeakContainer(sAllTextures, it, texture);
		if (texture->GetPlatformTexture() == platformShader){
			return texture;
		}
	}
	return 0;
}

size_t Texture::sNextTextureID = 0;
static VectorMap< BINDING_SHADER, VectorMap<int, TextureWeakPtr> > sBindedTextures;
void SetBindedTexture(BINDING_SHADER shader, int startSlot, TexturePtr pTextures[], int num){
	auto& shaderCategory = sBindedTextures[shader];
	int count = 0;
	for (int i = startSlot; i < startSlot + num; ++i){
		shaderCategory[i] = pTextures[count++];
	}
}

std::vector< std::pair<BINDING_SHADER, int> > FindSlotInfo(TexturePtr texture){
	std::vector< std::pair<BINDING_SHADER, int> > result;
	static int bindingShaders[] = {
		BINDING_SHADER_VS,
		BINDING_SHADER_HS,
		BINDING_SHADER_DS,
		BINDING_SHADER_GS,
		BINDING_SHADER_PS,
		BINDING_SHADER_CS
	};
	for (int shader = 0; shader < ARRAYCOUNT(bindingShaders); ++shader){
		auto& shaderCategory = sBindedTextures[(BINDING_SHADER)bindingShaders[shader]];
		for (auto it = shaderCategory.begin(); it != shaderCategory.end(); ++it){
			if (it->second.lock() == texture){
				result.push_back(std::make_pair((BINDING_SHADER)bindingShaders[shader], it->first));
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
	TEXTURE_TYPE mType;
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

	void SetType(TEXTURE_TYPE type){
		mType = type;
	}

	TEXTURE_TYPE GetType() const{
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

	void Bind(BINDING_SHADER shader, int slot) const{
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
};

//---------------------------------------------------------------------------
TexturePtr Texture::Create(){
	auto p = TexturePtr(FB_NEW(Texture), [](Texture* obj){ FB_DELETE(obj); });
	sAllTextures.push_back(p);
	p->mImpl->mSelf = p;
	return p;
}

void Texture::ReloadTexture(const char* file){
	auto& renderer = Renderer::GetInstance();
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

void Texture::SetType(TEXTURE_TYPE type)
{
	mImpl->SetType(type);
}

TEXTURE_TYPE Texture::GetType() const{
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

void Texture::Bind(BINDING_SHADER shader, int slot){
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

}