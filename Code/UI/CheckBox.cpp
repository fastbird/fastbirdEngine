#include <UI/StdAfx.h>
#include <UI/CheckBox.h>
#include <UI/ImageBox.h>
#include <UI/StaticText.h>
#include <sstream>

namespace fastbird
{

CheckBox::CheckBox()
	: mChecked(false)
{
	mCheckImageBox = static_cast<ImageBox*>(
		AddChild(0.f, 0.f, 0.05f, 1.0f, ComponentType::ImageBox));
	mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_unchecked");
	mCheckImageBox->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClicked, this, std::placeholders::_1));
	mStaticText = static_cast<StaticText*>(
		AddChild(0.06f, 0.f, 0.79f, 1.0f, ComponentType::StaticText));
	mStaticText->SetProperty(UIProperty::BACK_COLOR, "0.1, 0.1, 0.1, 1.0");
	mStaticText->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClicked, this, std::placeholders::_1));

	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

CheckBox::~CheckBox()
{
}

void CheckBox::GatherVisit(std::vector<IUIObject*>& v)
{
	__super::GatherVisit(v);
}

void CheckBox::OnSizeChanged()
{
	__super::OnSizeChanged();
	mCheckImageBox->SetSize(Vec2I(mSize.y, mSize.y));
	mStaticText->SetPosX(mSize.y+2);
}

void CheckBox::SetText(const wchar_t* szText)
{
	mStaticText->SetText(szText);
}

void CheckBox::SetCheck(bool check)
{
	if (mChecked != check)
	{
		mChecked = check;
		UpdateImage();
		
	}
}

bool CheckBox::GetCheck() const
{
	return mChecked;
}

void CheckBox::OnClicked(void* arg)
{
	mChecked = !mChecked;
	UpdateImage();

	OnEvent(IEventHandler::EVENT_MOUSE_LEFT_CLICK);
}

void CheckBox::UpdateImage()
{
	if (mChecked)
		mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_checked");
	else
		mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_unchecked");
}

}