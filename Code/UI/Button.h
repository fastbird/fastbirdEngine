#pragma once

#include <UI/Container.h>

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
	virtual void OnSizeChanged();
	virtual void OnPosChanged();
	virtual void SetNPosOffset(const Vec2& offset);
	virtual HorizontalGauge* GetProgressBar()  const { return mProgressBar; }
	virtual void StartProgress();
	virtual void SetPercentage(float p); // progress bar
	virtual void Blink(bool blink); // progress bar
	virtual void OnStartUpdate(float elapsedTime);
	virtual void EndProgress();
	virtual void SetEnable(bool enable);
	virtual void Highlight(bool highlight);
	virtual void SetBackgroundTexture(ITexture* pTexture);
	const static float LEFT_GAP;

	void OnMouseIn(void* arg);
	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void OnMouseDown(void* arg);

private:

	ImageBox* CreateImageBox();

private:
	Color mBackColor;
	Color mBackColorOver;
	Color mBackColorDown;
	Color mEdgeColor;
	Color mEdgeColorOver;

	std::string mImageAtlas;
	ImageBox* mImage;
	ImageBox* mImageOver;
	ImageBox* mFrameImage;
	HorizontalGauge* mProgressBar;
	bool mInProgress;
	bool mNoBackgroundBackup;
	bool mEnable;
};

}