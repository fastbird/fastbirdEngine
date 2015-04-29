#include <UI/StdAfx.h>
#include <UI/RadioBox.h>
#include <UI/ImageBox.h>
#include <UI/StaticText.h>
#include <sstream>

namespace fastbird
{

RadioBox::RadioBox()
	: mChecked(false)
{
	mRadioImageBox = static_cast<ImageBox*>(
		AddChild(0.f, 0.f, 0.05f, 1.0f, ComponentType::ImageBox));
	mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_unchecked");
	mRadioImageBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	mRadioImageBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));

	mStaticText = static_cast<StaticText*>(
		AddChild(0.06f, 0.f, 0.79f, 1.0f, ComponentType::StaticText));
	mStaticText->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 1.0");
	mStaticText->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	mStaticText->RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));

	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());

	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));
}

RadioBox::~RadioBox()
{
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
		if (mParent)
			mParent->OnClickRadio(this);
		SetCheck(StringConverter::parseBool(val));
		return true;
	}
	}
	return __super::SetProperty(prop, val);
}

bool RadioBox::GetProperty(UIProperty::Enum prop, char val[])
{
	switch (prop)
	{
	case UIProperty::RADIO_GROUP:
	{
		sprintf_s(val, 256, "%u", mGroupID);
		return true;
	}
	case UIProperty::RADIO_CHECK:
	{
		sprintf_s(val, 256, "%s", StringConverter::toString(mChecked).c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val);
}

void RadioBox::SetText(const wchar_t* szText)
{
	mStaticText->SetText(szText);
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

	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
}

void RadioBox::OnClicked(void* arg)
{
	SetCheck(!mChecked);
	if (mParent)
		mParent->OnClickRadio(this);
}

void RadioBox::UpdateImage()
{
	if (mChecked)
		mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_checked");
	else
		mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_unchecked");
}

void RadioBox::OnSizeChanged()
{
	__super::OnSizeChanged();
	Vec2 wnSize = mRadioImageBox->GetWNSize();

	auto rtSize = GetRenderTargetSize();
	unsigned width = rtSize.x;
	unsigned height = rtSize.y;
	float fHeight = wnSize.y * height;
	float fWidth = wnSize.x * width;
	if (fHeight > fWidth)
	{
		float wnWidth = fHeight / (float)width;
		mRadioImageBox->SetWNSize(Vec2(wnWidth, wnSize.y));
	}
	else if (fHeight < fWidth)
	{
		float wnHeight = fWidth / (float)height;
		mRadioImageBox->SetWNSize(Vec2(wnSize.x, wnHeight));
	}
}

void RadioBox::OnPosChanged()
{
	__super::OnPosChanged();

	const RECT& region = mRadioImageBox->GetRegion();
	unsigned posx = region.right+4;
	auto rtSize = GetRenderTargetSize();
	float fPosX = (float)posx / rtSize.x;
	Vec2 wnPos = mStaticText->GetWNPos();
	mStaticText->SetWNPos(Vec2(fPosX, wnPos.y));
	
}

void RadioBox::OnMouseHover(void* arg)
{
	if (!mEnable)
		return;
	SetCursor(WinBase::mCursorOver);
}

}