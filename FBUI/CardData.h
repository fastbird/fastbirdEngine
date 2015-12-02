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


namespace fb{
	FB_DECLARE_SMART_PTR(Texture);
	class FB_DLL_UI CardData{
	public:
		struct TextureData{
			TextureData(TexturePtr t, const char* cname);
			TexturePtr texture;
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
		void SetTexture(unsigned key, const char* comp, TexturePtr texture);
		void GetTextures(unsigned key, std::vector<TextureData>& textures);

		//---------------------------------------------------------------------------
		unsigned GetNumData() const;
		LuaObject GetData(unsigned key) const;
		LuaObject GetDataWithIndex(unsigned index) const;

		unsigned GetKey(unsigned index) const;
		unsigned GetIndex(unsigned key) const;
	};
}