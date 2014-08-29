#pragma once

#include <Engine/ITexture.h>

namespace fastbird
{
	class Texture : public ITexture
	{
	public:
		Texture();
		virtual ~Texture();

		static size_t NextTextureID;
		
		static void ReloadTexture(const char* unifiedPath);

		virtual void SetName(const char* filepath);
		virtual const std::string& GetName() const { return mName; }

		virtual void SetType(TEXTURE_TYPE type);
		virtual TEXTURE_TYPE GetType() const { return mType; }

		virtual void SetSamplerDesc(const SAMPLER_DESC& samplerDesc);
		virtual const SAMPLER_DESC& GetSamplerDesc() const { return mSamplerDesc; }

		virtual MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag);
		virtual void Unmap(UINT subResource);

		Texture* GetAdamTexture() const { return mAdamTexture; }
		void SetAdamTexture(Texture* adam) { mAdamTexture = adam; }

		size_t GetTextureID() const { return mTextureID; }

	protected:
		std::string mName;
		TEXTURE_TYPE mType;

	protected:
		static std::vector<Texture*> mTextures;
		SAMPLER_DESC mSamplerDesc;
		SmartPtr<Texture> mAdamTexture;
		size_t mTextureID;
	};
}