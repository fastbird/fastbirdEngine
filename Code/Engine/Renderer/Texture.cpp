#include <Engine/StdAfx.h>
#include "Texture.h"

using namespace fastbird;

std::vector<Texture*> Texture::mTextures;

//----------------------------------------------------------------------------
Texture::Texture()
{
	mTextures.push_back(this);
	mType = TEXTURE_TYPE_DEFAULT;
}

//----------------------------------------------------------------------------
Texture::~Texture()
{
	mTextures.erase(std::remove(mTextures.begin(), mTextures.end(), this), 
		mTextures.end());
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

//----------------------------------------------------------------------------
void Texture::SetSamplerDesc(const SAMPLER_DESC& samplerDesc)
{
	mSamplerDesc = samplerDesc;
}

MapData Texture::Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag)
{
	return gFBEnv->pRenderer->MapTexture(this, subResource, type, flag);
}
void Texture::Unmap(UINT subResource)
{
	gFBEnv->pRenderer->UnmapTexture(this, subResource);
}