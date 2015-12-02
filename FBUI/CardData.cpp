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

#include "StdAfx.h"
#include "CardData.h"


using namespace fb;

CardData::TextureData::TextureData(TexturePtr t, const char* cname)
	: texture(t)
	, compName(cname)
{
}

unsigned CardData::AddData(unsigned key, LuaObject& data){
	unsigned index = -1;
	auto found = std::find(mKeys.begin(), mKeys.end(), key);
	if (found == mKeys.end()){
		mKeys.push_back(key);
		index = mKeys.size() - 1;
	}
	else{
		index = std::distance(mKeys.begin(), found);
	}
	mData[key] = data;
	return index;
}

unsigned CardData::DeleteData(unsigned key){
	auto it = mData.Find(key);
	unsigned distance = -1;
	if (it != mData.end()){
		mData.erase(it);
		auto keyIt = std::find(mKeys.begin(), mKeys.end(), key);
		assert(keyIt != mKeys.end());
		distance = std::distance(mKeys.begin(), keyIt);
		mKeys.erase(keyIt);		
	}
	auto textureIt = mTextures.Find(key);
	if (textureIt != mTextures.end()){
		mTextures.erase(textureIt);
	}
	return distance;
}

void CardData::Clear(){
	mData.clear();
	mTextures.clear();
	mKeys.clear();
}

void CardData::DeleteDataWithIndex(unsigned index){
	assert(index < mKeys.size());
	unsigned key = mKeys[index];
	DeleteData(key);
}

void CardData::SetTexture(unsigned key, const char* comp, TexturePtr texture){
	auto it = mTextures.Find(key);
	if (it == mTextures.end()){
		mTextures.Insert(std::make_pair(key, std::vector<TextureData>()));
	}
	it = mTextures.Find(key);
	assert(it != mTextures.end());
	auto& v = it->second;
	bool found = false;
	for (auto i = v.begin(); i != v.end(); ++i){
		if (strcmp(i->compName.c_str(), comp) == 0){
			found = true;
			assert(i->texture == texture);
			break;
		}
	}
	if (!found)
		it->second.push_back(TextureData(texture, comp));	

}

void CardData::GetTextures(unsigned key, std::vector<TextureData>& textures){
	auto it = mTextures.Find(key);
	if (it != mTextures.end()){
		const auto& v = it->second;
		for (auto i = v.begin(); i != v.end(); ++i){
			textures.push_back(*i);
		}
	}
}

//---------------------------------------------------------------------------
unsigned CardData::GetNumData() const{
	return mKeys.size();
}

LuaObject CardData::GetData(unsigned key) const{
	auto it = mData.Find(key);
	if (it == mData.end()){
		return LuaObject();
	}
	return mData[key];
}

LuaObject CardData::GetDataWithIndex(unsigned index) const{
	assert(index < mKeys.size());
	return mData[mKeys[index]];
}

unsigned CardData::GetKey(unsigned index) const{
	assert(index < mKeys.size());
	return mKeys[index];
}

unsigned CardData::GetIndex(unsigned key) const{
	auto it = std::find(mKeys.begin(), mKeys.end(), key);
	if (it != mKeys.end())
		return std::distance(mKeys.begin(), it);
	return -1;
}