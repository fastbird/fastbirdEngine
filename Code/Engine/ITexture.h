#pragma once

#include <CommonLib/SmartPtr.h>
#include <CommonLib/VectorMap.h>
#include <Engine/Renderer/RendererEnums.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class Vec2I;
	class ITexture : public ReferenceCounter
	{
	public:
		virtual ~ITexture(){}

		virtual bool IsReady() const = 0;
		virtual Vec2I GetSize() const = 0;
		virtual PIXEL_FORMAT GetFormat() const = 0;
		virtual void SetSamplerDesc(const SAMPLER_DESC& desc) = 0;
		virtual const SAMPLER_DESC& GetSamplerDesc() const = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void SetSlot(int slot) = 0;
		virtual int GetSlot() const = 0;

		virtual void SetName(const char* filepath) = 0;
		virtual const std::string& GetName() const = 0;

		virtual void SetType(TEXTURE_TYPE type) = 0;
		virtual TEXTURE_TYPE GetType() const = 0;

		virtual void SetShaderStage(BINDING_SHADER shader) = 0;
		virtual BINDING_SHADER GetShaderStage() const = 0;

		virtual ITexture* Clone() const = 0;

		virtual MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) = 0;
		virtual void Unmap(UINT subResource) = 0;

		virtual void CopyToStaging(ITexture* dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ,
			UINT srcSubresource, Box3D* srcBox) = 0;

		virtual void SaveToFile(const char* filename) = 0;

		virtual void GenerateMips() = 0;
	};

	struct TextureAtlasRegion
	{
		DWORD mID;
		std::string mName;		
		Vec2 mUVStart;
		Vec2 mUVEnd;

		void GetQuadUV(Vec2 uv[])
		{
			uv[0] = Vec2(mUVStart.x, mUVEnd.y);
			uv[1] = mUVStart;
			uv[2] = mUVEnd;
			uv[3] = Vec2(mUVEnd.x, mUVStart.y);
		}
	};
	struct TextureAtlas : public ReferenceCounter
	{
		std::string mPath;
		SmartPtr<ITexture> mTexture;
		typedef VectorMap<std::string, TextureAtlasRegion*> REGION_MAP;
		REGION_MAP mRegions;

		~TextureAtlas()
		{
			for each ( auto c in mRegions)
			{
				delete c.second;
			}
			mRegions.clear();
		}

		void AddRegion(TextureAtlasRegion* pRegion)
		{
			mRegions.Insert(std::make_pair(pRegion->mName, pRegion));
		}

		TextureAtlasRegion* GetRegion(const char* name)
		{
			REGION_MAP::iterator it = mRegions.Find(name);
			if (it != mRegions.end())
			{
				return it->second;
			}
			else
			{
				return 0;
			}
		}
	};
}