#include <UI/StdAfx.h>
#include <UI/RadioBox.h>
#include <UI/ImageBox.h>
#include <UI/StaticText.h>
#include <sstream>

namespace fastbird
{

RadioBox::RadioBox()
	: mChecked(false)
	, mGroupID(-1)
	, mStaticText(0)
	, mRadioImageBox(0)
{
	
}

RadioBox::~RadioBox()
{
}

void RadioBox::OnCreated(){
	mRadioImageBox = static_cast<ImageBox*>(
		AddChild(Vec2I(0, 0), Vec2I(mSize.y, mSize.y), ComponentType::ImageBox));
	mRadioImageBox->SetRuntimeChild(true);	
	mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_unchecked");
	mRadioImageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	mRadioImageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	mRadioImageBox->SetUseAbsPos(false);
	mRadioImageBox->SetUseAbsSize(false);
	mRadioImageBox->SetProperty(UIProperty::KEEP_UI_RATIO, "false");
	mRadioImageBox->SetProperty(UIProperty::IMAGE_LINEAR_SAMPLER, "true");
	UpdateImage();

	mStaticText = static_cast<StaticText*>(
		AddChild(0.06f, 0.f, 0.79f, 1.0f, ComponentType::StaticText));
	mStaticText->ChangePosX(24);
	mStaticText->SetRuntimeChild(true);
	mStaticText->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 0.5");
	mStaticText->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	mStaticText->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	mStaticText->SetUseAbsPos(false);
	mStaticText->SetUseAbsSize(false);
	if (!mTempString.empty()){
		mStaticText->SetText(mTempString.c_str());
		ClearWithSwap(mTempString);		
	}

	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
}

void RadioBox::GatherVisit(std::vector<IUIObject*>& v)
{
	__super::GatherVisit(v);
}

bool RadioBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::RADIO_GROUP:
	{
		SetGroupID(StringConverter::parseUnsignedInt(val));
		return true;
	}
	case UIProperty::RADIO_CHECK:
	{
		SetCheck(StringConverter::parseBool(val));
		return true;
	}
	}
	return __super::SetProperty(prop, val);
}

bool RadioBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::RADIO_GROUP:
	{
		if (notDefaultOnly)
		{
			if (mGroupID == UIProperty::GetDefaultValueInt(prop))
				return false;
		}
		auto data = StringConverter::toString(mGroupID);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::RADIO_CHECK:
	{
		if (notDefaultOnly)
		{
			if (mChecked == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mChecked);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void RadioBox::OnSizeChanged(){
	__super::OnSizeChanged();
	if (mRadioImageBox)
		mRadioImageBox->ChangeSize(Vec2I(mSize.y, mSize.y));
}

void RadioBox::SetText(const wchar_t* szText)
{
	if (mStaticText)
		mStaticText->SetText(szText);
	else
		mTempString = szText;
}

void RadioBox::SetCheck(bool check)
{
	if (mChecked != check)
	{
		mChecked = check;
		UpdateImage();
	}
}

bool RadioBox::GetCheck() const
{
	return mChecked;
}

void RadioBox::OnChildrenClicked(void* arg)
{
	SetCheck(!mChecked);
	if (mParent)
		mParent->OnClickRadio(this);

	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
}

void RadioBox::OnClicked(void* arg)
{
	SetCheck(!mChecked);
	if (mParent)
		mParent->OnClickRadio(this);
}

void RadioBox::UpdateImage()
{
	if (!mRadioImageBox)
		return;
	if (mChecked)
		mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_checked");
	else
		mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_unchecked");
}

void RadioBox::OnMouseHover(void* arg)
{
	if (!mEnable)
		return;
	SetCursor(WinBase::sCursorOver);
}

}