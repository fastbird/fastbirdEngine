#pragma once

#include <UI/Container.h>
#include <UI/ButtonImages.h>

namespace fastbird
{
class IUIObject;
class ImageBox;
class HorizontalGauge;
class Button : public Container
{
public:
	Button();
	virtual ~Button();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Button; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);
	virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
	virtual bool SetVisible(bool visible);
	virtual void SetVisibleInternal(bool visible);

	virtual void OnPosChanged(bool anim);
	virtual HorizontalGauge* GetProgressBar()  const { return mProgressBar; }
	virtual void StartProgress();
	virtual void SetPercentage(float p); // progress bar
	virtual void Blink(bool blink); // progress bar
	virtual void Blink(bool blink, float time); // progress bar
	virtual void EndProgress();
	virtual void Highlight(bool highlight);
	virtual void SetTexture(ButtonImages::Enum type, ITexture* pTexture, bool drawFixedSize);
	virtual void OnEnableChanged();
	const static float LEFT_GAP;

	void OnMouseIn(void* arg);
	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void OnMouseDown(void* arg);
	bool IsActivated() const { return mActivated; }
	void AlignIconText();

	void UpdateImageSize();
	virtual void SetEnable(bool enable);


protected:
	virtual void SetUseBorder(bool use);

private:

	ImageBox* CreateImageBox();
	void SetAlphaRegionTexture();
	void SetDefaultImageAtlasPathIfNotSet();

private:
	Color mBackColor;
	Color mBackColorOver;
	Color mBackColorDown;
	Color mEdgeColor;
	Color mEdgeColorOver;

	std::string mImageAtlas;
	ImageBox* mImages[ButtonImages::Num];
	std::string mRegionName;
	std::string mRegionNames;
	std::string mTextureFile;
	std::string mHorverImage;
	std::string mBackgroundImage;
	std::string mBackgroundImageDisabled;
	std::string mBackgoundImageHover;
	std::string mBackgroundImageNoAtlas;
	std::string mBackgroundImageHoverNoAtlas;
	std::string mFrameImage;
	std::string mFrameImageDisabled;
	std::string mActivatedImage;
	std::string mDeactivatedImage;

	std::string mAlphaRegion;
	Vec2I mImageSize;

	Color mImageColorOverlay;
	bool mActivatedRot;
	float mFps;
	HorizontalGauge* mProgressBar;
	int mButtonIconSize;
	bool mInProgress;
	bool mNoBackgroundBackup;
	bool mActivated;
	bool mChangeImageActivation;
	bool mIconText;
	bool mNoButton;
};

}