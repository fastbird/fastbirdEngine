#include <UI/StdAfx.h>
#include <UI/CardData.h>


using namespace fastbird;

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
	if (it != mData.end()){
		mData.erase(it);
		auto keyIt = std::find(mKeys.begin(), mKeys.end(), key);
		assert(keyIt != mKeys.end());
		unsigned distance = std::distance(mKeys.begin(), keyIt);
		mKeys.erase(keyIt);
		return distance;
	}
	return -1;
}

void CardData::DeleteDataWithIndex(unsigned index){
	assert(index < mKeys.size());
	unsigned key = mKeys[index];
	DeleteData(key);
}

void CardData::SetTexture(unsigned key, const char* comp, ITexture* texture){
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