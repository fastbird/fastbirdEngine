#pragma once
#include <CommonLib/LuaObject.h>

namespace fastbird{
	class CardData{
	public:
		struct TextureData{
			TextureData(ITexture* t, const char* cname)
				: texture(t)
				, compName(cname)
			{

			}
			SmartPtr<ITexture> texture;
			std::string compName;
		};

	private:
		std::vector<unsigned> mKeys;
		VectorMap<unsigned, LuaObject> mData;

		
		VectorMap<unsigned, std::vector<TextureData> > mTextures;
	public:
		unsigned AddData(unsigned key, LuaObject& data);
		unsigned DeleteData(unsigned key);
		void Clear();
		void DeleteDataWithIndex(unsigned index);
		void SetTexture(unsigned key, const char* comp, ITexture* texture);
		void GetTextures(unsigned key, std::vector<TextureData>& textures);

		//---------------------------------------------------------------------------
		unsigned GetNumData() const;
		LuaObject GetData(unsigned key) const;
		LuaObject GetDataWithIndex(unsigned index) const;

		unsigned GetKey(unsigned index) const;
		unsigned GetIndex(unsigned key) const;
	};
}