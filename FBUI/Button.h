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

#include "Container.h"
#include "ButtonImages.h"

namespace fb
{
FB_DECLARE_SMART_PTR(HorizontalGauge);
FB_DECLARE_SMART_PTR(ImageBox);
FB_DECLARE_SMART_PTR(Button);
class FB_DLL_UI Button : public Container
{
protected:
	Button();
	~Button();

public:
	static ButtonPtr Create();

	// IWinBase
	ComponentType::Enum GetType() const { return ComponentType::Button; }
	void GatherVisit(std::vector<UIObject*>& v);
	bool SetProperty(UIProperty::Enum prop, const char* val);
	bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
	bool SetVisible(bool visible);
	void SetVisibleInternal(bool visible);

	void OnPosChanged(bool anim);
	HorizontalGaugePtr GetProgressBar()  const { return mProgressBar.lock(); }
	void StartProgress();
	void SetPercentage(float p); // progress bar
	void Blink(bool blink); // progress bar
	void Blink(bool blink, float time); // progress bar
	void EndProgress();
	void Highlight(bool highlight);
	void SetTexture(ButtonImages::Enum type, TexturePtr pTexture, bool drawFixedSize);
	void OnEnableChanged();
	const static float LEFT_GAP;

	void OnMouseIn(void* arg);
	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void OnMouseDown(void* arg);
	void OnMouseClicked(IInputInjectorPtr injector);
	bool IsActivated() const { return mActivated; }
	void AlignIconText();

	void UpdateImageSize();
	void SetEnable(bool enable);


protected:
	void SetUseBorder(bool use);

private:

	ImageBoxPtr CreateImageBox();
	void SetAlphaRegionTexture();
	void SetDefaultImageAtlasPathIfNotSet();

private:
	Color mBackColor;
	Color mBackColorOver;
	Color mBackColorDown;
	Color mEdgeColor;
	Color mEdgeColorOver;

	std::string mImageAtlas;
	ImageBoxWeakPtr mImages[ButtonImages::Num];
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
	HorizontalGaugeWeakPtr mProgressBar;
	int mButtonIconSize;
	bool mInProgress;
	bool mNoBackgroundBackup;
	bool mActivated;
	bool mChangeImageActivation;
	bool mIconText;
	bool mNoButton;
};

}