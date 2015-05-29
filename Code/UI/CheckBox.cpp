#include <UI/StdAfx.h>
#include <UI/CheckBox.h>
#include <UI/ImageBox.h>
#include <UI/StaticText.h>
#include <sstream>

namespace fastbird
{

CheckBox::CheckBox()
	: mChecked(false)
	, mCheckImageBox(0)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetNoDrawBackground(true);
	mTextGap.x = 28;
}

CheckBox::~CheckBox()
{
}

void CheckBox::OnCreated()
{
	mCheckImageBox = static_cast<ImageBox*>(AddChild(ComponentType::ImageBox));
	mCheckImageBox->SetRuntimeChild(true);
	mCheckImageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClickedChildren, this, std::placeholders::_1));
	mCheckImageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&CheckBox::OnClickedChildren, this, std::placeholders::_1));
	mCheckImageBox->RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&CheckBox::OnMouseHover, this, std::placeholders::_1));
	mCheckImageBox->SetAlign(ALIGNH::LEFT, ALIGNV::MIDDLE);
	mCheckImageBox->ChangeSize(Vec2I(24, 24));
	mCheckImageBox->ChangeNPos(Vec2(0.0, 0.5f));

	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&CheckBox::OnClicked, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&CheckBox::OnMouseHover, this, std::placeholders::_1));
	

	if (mChecked)
		mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_checked");
	else
		mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_unchecked");
}

void CheckBox::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);
	__super::GatherVisit(v);
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
	gFBEnv->pEngine->GetMouse()->Invalidate();
}

void CheckBox::OnClickedChildren(void* arg)
{
	OnEvent(UIEvents::EVENT_MOUSE_LEFT_CLICK);
}

void CheckBox::UpdateImage()
{
	if (mCheckImageBox)
	{
		if (mChecked)
			mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_checked");
		else
			mCheckImageBox->SetTextureAtlasRegion("es/textures/ui.xml", "checkbox_unchecked");
	}
}

void CheckBox::OnMouseHover(void* arg)
{
	SetCursor(WinBase::sCursorOver);
}


bool CheckBox::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::CHECKBOX_CHECKED:
	{
										 bool checked = StringConverter::parseBool(val);
										 SetCheck(checked);
										 return true;
	}
	}

	return __super::SetProperty(prop, val);
}

bool CheckBox::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::CHECKBOX_CHECKED:
	{
		if (notDefaultOnly)
		{
			if (mChecked == GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mChecked);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	}
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

}