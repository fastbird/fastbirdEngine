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
	virtual void OnSizeChanged();
	virtual void OnStartUpdate(float elapsedTime);

	// ImageBox;
	virtual void SetTexture(const char* file);
	virtual void SetTexture(ITexture* pTexture);
	virtual void SetUseHighlight(bool use) { mUseHighlight = use; }
	// or
	virtual void SetTextureAtlasRegion(const char* atlas, const char* region);
	virtual void SetTextureAtlasRegions(const char* atlas, const std::vector<std::string>& data);
	virtual void ChangeRegion(TextureAtlasRegion* region);
	virtual void ChangeRegion(const char* region);
	virtual TextureAtlasRegion* GetTextureAtlasRegion() const { return mAtlasRegion; }
	virtual bool IsAnimated() const;
	virtual void Highlight(bool enable);
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);

	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void SetKeepImageRatio(bool keep);
	void SetUVRot(bool set);
	void SetCenterUVMatParam();
	void DrawAsFixedSizeCenteredAt(const Vec2& wnpos);
	void DrawAsFixedSizeAtCenter();
	void SetDesaturate(bool desat);
	void SetAmbientColor(const Vec4& color);

private:

	ImageBox* ImageBox::CreateImageBox();
	void CalcUV(const Vec2I& textureSize);


private:
	std::string mTextureAtlasFile;
	std::string mImageFile; 
	TextureAtlas* mTextureAtlas;
	TextureAtlasRegion* mAtlasRegion;
	std::vector<TextureAtlasRegion*> mAtlasRegions;
	SmartPtr<ITexture> mTexture;
	bool mUseHighlight;
	bool mKeepImageRatio;
	ImageBox* mFrameImage;
	bool mAnimation;
	float mSecPerFrame;
	float mPlayingTime;
	unsigned mCurFrame;
};
}