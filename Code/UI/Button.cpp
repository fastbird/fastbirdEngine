#include <UI/StdAfx.h>
#include <UI/Button.h>
#include <Engine/GlobalEnv.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{

const float Button::LEFT_GAP = 0.001f;

Button::Button()
	: WinBase()
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->SetMaterial("es/Materials/UIButton.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mTextColor = Vec3(1.0f, 0.92f, 0.0f);
	mUIObject->SetTextColor(mTextColor);
	mBackColor = Color(0.0f, 0.0f, 0.0f);
	mBackColorOver = Color(0.2f, 0.2f, 0.2f);
	mBackColorDown = Color(1.f, 1.f, 1.f);
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
	
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_DOWN, 
		std::bind(&Button::OnMouseDown, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER, 
		std::bind(&Button::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_OUT, 
		std::bind(&Button::OnMouseOut, this, std::placeholders::_1));
}

Button::~Button()
{
}

void Button::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);
}

void Button::OnMouseDown(void* arg)
{
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorDown.GetVec4());
	mUIObject->SetTextColor(mTextColorDown);
}

void Button::OnMouseHover(void* arg)
{
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColorOver.GetVec4());
	mUIObject->SetTextColor(mTextColorHover);
	SetCursor(WinBase::mCursorOver);
}

void Button::OnMouseOut(void* arg)
{
	mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
	mUIObject->SetTextColor(mTextColor);
}

bool Button::SetProperty(UIProperty::Enum prop, const char* val)
{
	if (prop == UIProperty::BACK_COLOR)
	{
		mBackColor = StringConverter::parseVec4(val);
		mUIObject->GetMaterial()->SetDiffuseColor(mBackColor.GetVec4());
		return true;
	}
	
	if (prop == UIProperty::BACK_COLOR_OVER)
	{
		Vec4 color;
		color = StringConverter::parseVec4(val);
		mBackColorOver = color;
		return true;
	}

	return __super::SetProperty(prop, val);
}

}