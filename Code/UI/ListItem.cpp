#include <UI/StdAfx.h>
#include <UI/ListItem.h>
#include <UI/PropertyList.h>
#include <UI/CheckBox.h>

namespace fastbird{

//-----------------------------------------------------------------------------
const float ListItem::LEFT_GAP = 0.001f;
const size_t ListItem::INVALID_INDEX = -1;

ListItem::ListItem()
	: mRowIndex(INVALID_INDEX)
	, mColIndex(INVALID_INDEX)
	, mNoBackground(true)
	, mBackColor("0.1, 0.3, 0.3, 0.7")
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

ListItem::~ListItem()
{

}

void ListItem::SetRowIndex(size_t index)
{
	mRowIndex = index;
}
void ListItem::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);

	__super::GatherVisit(v);
}

void ListItem::OnPosChanged()
{
	__super::OnPosChanged();
	//mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

void ListItem::OnSizeChanged()
{
	__super::OnSizeChanged();
	//mUIObject->SetTextStartNPos(Vec2(mWNPos.x, mWNPos.y + mWNSize.y - GetTextBottomGap()));
}

CheckBox* ListItem::GetCheckBox() const
{
	for (auto& child : mChildren)
	{
		if (child->GetType() == ComponentType::CheckBox)
		{
			return dynamic_cast<CheckBox*>(child);
		}
	}
	return 0;
}

void ListItem::SetBackColor(const char* backColor)
{
	if_assert_fail(backColor)
		return;
	mBackColor = backColor;
}

void ListItem::SetNoBackground(bool noBackground)
{
	mNoBackground = noBackground;
}

void ListItem::OnFocusGain()
{
	if (mParent && mParent->GetType() == ComponentType::PropertyList)
	{
		PropertyList* prop = (PropertyList*)mParent;
		prop->SetFocusRow(mRowIndex);
	}
	//SetProperty(UIProperty::NO_BACKGROUND, "false");
	TriggerRedraw();
}

void ListItem::OnFocusLost()
{
	//SetProperty(UIProperty::NO_BACKGROUND, "true");
	if (mParent && mParent->GetType() == ComponentType::PropertyList)
	{

	}
	TriggerRedraw();
}

//bool ListItem::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
//{
//	if (!GetFocus(true))
//		return mMouseIn;
//
//	bool mousein = __super::OnInputFromHandler(mouse, keyboard);
//
//	if (keyboard->IsValid() && mParent && mParent->GetType() == ComponentType::PropertyList)
//	{
//		ListBox* listbox = (ListBox*)mParent;
//		auto c = keyboard->GetChar();
//		if (c)
//		{
//			keyboard->PopChar();
//			keyboard->Invalidate();
//			if (c == VK_TAB)
//			{
//				if (keyboard->IsKeyDown(VK_SHIFT))
//					listbox->MoveFocusToEdit(mRowIndex - 1);
//				else
//					listbox->MoveFocusToEdit(mRowIndex);
//			}
//			else
//			{
//				prop->GoToNext(c, mRowIndex);
//			}
//		}
//
//		if (keyboard->IsKeyPressed(VK_DOWN)){
//			prop->MoveLine(false, true);
//			keyboard->Invalidate();
//		}
//		else if (keyboard->IsKeyPressed(VK_UP)) 	{
//			prop->MoveLine(false, false);
//			keyboard->Invalidate();
//		}
//		else if (keyboard->IsKeyPressed(VK_RIGHT)){
//			prop->MoveFocusToEdit(mRowIndex);
//			keyboard->Invalidate();
//		}
//	}	
//
//	return mousein;
//}

}
