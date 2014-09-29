#include <Engine/StdAfx.h>
#include "Texture.h"

using namespace fastbird;

std::vector<Texture*> Texture::mTextures;

size_t Texture::NextTextureID = 0;

//----------------------------------------------------------------------------
Texture::Texture()
: mAdamTexture(0)
{
	mTextures.push_back(this);
	mType = TEXTURE_TYPE_DEFAULT;
	mTextureID = NextTextureID++;
}

//----------------------------------------------------------------------------
Texture::~Texture()
{
	mTextures.erase(std::remove(mTextures.begin(), mTextures.end(), this), 
		mTextures.end());
}

//static 
void ITexture::ReloadTexture(const char* unifiedPath)
{
	Texture::ReloadTexture(unifiedPath);
}

void Texture::ReloadTexture(const char* unifiedPath)
{
	std::string lower = unifiedPath;
	ToLowerCase(lower);
	FB_FOREACH(it, Texture::mTextures)
	{
		Texture* p = *it;
		if (!p->GetAdamTexture())
		{
			if (p->GetName() == lower)
			{
				gFBEnv->pRenderer->CreateTexture(lower.c_str(), p);
			}
		}
	}

}

//----------------------------------------------------------------------------
void Texture::SetName(const char* filepath)
{
	mName = filepath;
}

//----------------------------------------------------------------------------
void Texture::SetType(TEXTURE_TYPE type)
{
	mType = type;
}

////----------------------------------------------------------------------------
//void Texture::SetSamplerDesc(const SAMPLER_DESC& samplerDesc)
//{
//	mSamplerDesc = samplerDesc;
//	mSamplerState = gFBEnv->pRenderer->CreateSamplerState(samplerDesc);
//}

//----------------------------------------------------------------------------
MapData Texture::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag)
{
	return gFBEnv->pRenderer->MapTexture(this, subResource, type, flag);
}
void Texture::Unmap(UINT subResource)
{
	gFBEnv->pRenderer->UnmapTexture(this, subResource);
}