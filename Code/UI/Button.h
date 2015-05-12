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
	virtual void OnSizeChanged();
	virtual void OnPosChanged();
	virtual HorizontalGauge* GetProgressBar()  const { return mProgressBar; }
	virtual void StartProgress();
	virtual void SetPercentage(float p); // progress bar
	virtual void Blink(bool blink); // progress bar
	virtual void OnStartUpdate(float elapsedTime);
	virtual void EndProgress();
	virtual void Highlight(bool highlight);
	virtual void SetTexture(ButtonImages::Enum type, ITexture* pTexture);
	virtual void OnEnableChanged();
	const static float LEFT_GAP;

	void OnMouseIn(void* arg);
	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void OnMouseDown(void* arg);
	bool IsActivated() const { return mActivated; }
	void AlignIconText();

	virtual void SetHwndId(HWND_ID hwndId);

private:

	ImageBox* CreateImageBox();

private:
	Color mBackColor;
	Color mBackColorOver;
	Color mBackColorDown;
	Color mEdgeColor;
	Color mEdgeColorOver;

	std::string mImageAtlas;
	ImageBox* mImages[ButtonImages::Num];

	HorizontalGauge* mProgressBar;
	unsigned mButtonIconSize;
	bool mInProgress;
	bool mNoBackgroundBackup;
	bool mActivated;
	bool mChangeImageActivation;
	bool mIconText;
	bool mNoButton;
};

}