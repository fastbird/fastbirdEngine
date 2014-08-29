#pragma once

#include <UI/Container.h>

namespace fastbird
{

class ImageBox : public Container
{
public:
	ImageBox();
	~ImageBox();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::ImageBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// ImageBox;
	virtual void SetTexture(const char* file);
	virtual void SetTexture(ITexture* pTexture);
	virtual void SetUseHighlight(bool use) { mUseHighlight = use; }
	// or
	virtual void SetTextureAtlasRegion(const char* atlas, const char* region);
	virtual void ChangeRegion(TextureAtlasRegion* region);
	virtual void ChangeRegion(const char* region);
	virtual TextureAtlasRegion* GetTextureAtlasRegion() const { return mAtlasRegion; }
	virtual void Highlight(bool enable);
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);

	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);


private:
	std::string mImageFile;
	TextureAtlas* mTextureAtlas;
	TextureAtlasRegion* mAtlasRegion;
	SmartPtr<ITexture> mTexture;
	bool mUseHighlight;

};
}