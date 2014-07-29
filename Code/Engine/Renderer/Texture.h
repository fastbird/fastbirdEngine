#pragma once

#include <Engine/ITexture.h>

namespace fastbird
{
	class Texture : public ITexture
	{
	public:
		Texture();
		virtual ~Texture();

		
		virtual void SetName(const char* filepath);
		virtual const std::string& GetName() const { return mName; }

		virtual void SetType(TEXTURE_TYPE type);
		virtual TEXTURE_TYPE GetType() const { return mType; }

		virtual void SetSamplerDesc(const SAMPLER_DESC& samplerDesc);
		virtual const SAMPLER_DESC& GetSamplerDesc() const { return mSamplerDesc; }

		virtual MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag);
		virtual void Unmap(UINT subResource);

	protected:
		std::string mName;
		TEXTURE_TYPE mType;

	protected:
		static std::vector<Texture*> mTextures;
		SAMPLER_DESC mSamplerDesc;
	};
}