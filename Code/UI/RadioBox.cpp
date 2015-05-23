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
{
	mRadioImageBox = static_cast<ImageBox*>(
		AddChild(Vec2I(0, 0), Vec2I(24, 24), ComponentType::ImageBox));
	mRadioImageBox->SetRuntimeChild(true);
	mRadioImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "radiobox_unchecked");
	mRadioImageBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&RadioBox::OnChildrenClicked, this, std::placeholders::_1));
	mRadioImageBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&RadioBox::OnMouseHover, this, std::placeholders::_1));

	mStaticText = static_cast<StaticText*>(
		AddChild(0.06f, 0.f, 0.79f, 1.0f, ComponentType::StaticText));
	mStaticText->ChangePosX(24);
	mStaticText->SetRuntimeChild(true);
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

bool RadioBox::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
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
		strcpy(val, data.c_str());
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
		strcpy(val, data.c_str());
		return true;
	}
	}
	return __super::GetProperty(prop, val, notDefaultOnly);
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
	int sizeY = GetFinalSize().y;
	mRadioImageBox->ChangeSize(Vec2I(sizeY, sizeY));
	mStaticText->ChangePosX(sizeY+4);
}

void RadioBox::OnMouseHover(void* arg)
{
	if (!mEnable)
		return;
	SetCursor(WinBase::sCursorOver);
}

}