#include <UI/StdAfx.h>
#include <UI/DropDown.h>
#include <UI/IUIManager.h>
#include <UI/Button.h>

namespace fastbird
{

const float DropDown::LEFT_GAP = 0.001f;

DropDown* DropDown::sCurrentDropDown = 0;

DropDown::DropDown()
	: mCursorPos(0)
	, mPasswd(false)
	, mCurIdx(0)
	, mReservedIdx(-1)
	, mButton(0)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextColor(mTextColor);
	mUIObject->SetNoDrawBackground(true);
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));

	/*RegisterEventFunc(UIEvents::EVENT_MOUSE_HOVER,
		std::bind(&DropDown::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_OUT,
		std::bind(&DropDown::OnMouseOut, this, std::placeholders::_1));*/
}

DropDown::~DropDown()
{
	if (sCurrentDropDown == this)
		sCurrentDropDown = 0;
	mDropDownItems.clear();
}

void DropDown::OnCreated()
{
	mButton = (Button*)AddChild(1.0f, 0.0f, 0.1, 1.0f, ComponentType::Button);
	mButton->ChangeSize(Vec2I(24, 24));
	mButton->SetProperty(UIProperty::ALIGNH, "right");
	mButton->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));

	mButton->SetProperty(UIProperty::NO_BACKGROUND, "true");
	mButton->SetProperty(UIProperty::TEXTUREATLAS, "es/textures/ui.xml");
	mButton->SetProperty(UIProperty::REGION, "dropdown");
	mButton->SetProperty(UIProperty::HOVER_IMAGE, "dropdown_hover");
	mButton->SetRuntimeChild(true);


}

void DropDown::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject);
	__super::GatherVisit(v);
}

bool DropDown::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::DROPDOWN_INDEX:
	{
		unsigned index = StringConverter::parseUnsignedInt(val);
		if (index < mDropDownItems.size())
			SetSelectedIndex(index);
		else
			SetReservedIndex(index);
		return true;
	}
	}


	return __super::SetProperty(prop, val);
}

bool DropDown::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
{
	switch (prop)
	{
	case UIProperty::DROPDOWN_INDEX:
	{
		std::string data;
		if (mReservedIdx!=-1)
		{
			data = StringConverter::toString(mReservedIdx);
		}
		else
		{
			data = StringConverter::toString(mCurIdx);
		}
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	}
	
	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
}

void DropDown::OnMouseClick(void* arg)
{
	if (mDropDownItems.empty())
		return;
	bool vis = mDropDownItems[0]->GetVisible();

	if (!vis)
	{
		// become visible
		if (sCurrentDropDown)
			sCurrentDropDown->CloseOptions();

		sCurrentDropDown = this;

	}
	else
	{
		// become invisible
		if (sCurrentDropDown == this)
			sCurrentDropDown = 0;
	}

	for (auto var : mDropDownItems)
	{
		var->SetVisible(!vis);
	}
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
}

void DropDown::CloseOptions()
{
	for (auto var : mDropDownItems)
	{
		var->SetVisible(false);
	}
}

void DropDown::OnFocusLost()
{
	if (gFBUIManager->GetNewFocusUI() == mButton)
		return;
	for (auto var : mDropDownItems)
	{
		var->SetVisible(false);
	}
}

void DropDown::OnItemSelected(void* arg)
{
	size_t index = -1;
	size_t it = 0;
	for (auto var : mDropDownItems)
	{
		if (var == arg)
		{
			index = it;
			SetText(var->GetText());
		}
		var->SetVisible(false);
		++it;
	}
	assert(index != -1);
	mCurIdx = index; 
	OnEvent(UIEvents::EVENT_DROP_DOWN_SELECTED);
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
}

size_t DropDown::AddDropDownItem(WCHAR* szString)
{
	mDropDownItems.push_back((Button*)AddChild(0.0f, 1.0f, 1.0f, 1.0f, ComponentType::Button));
	Button* pDropDownItem = mDropDownItems.back();
	// dropdown need to be saved.
//	pDropDownItem->SetRuntimeChild(true);
	pDropDownItem->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
	pDropDownItem->SetText(szString);
	size_t index = mDropDownItems.size()-1;
	SetCommonProperty(pDropDownItem, index);
	if (mDropDownItems.size() == 1)
	{
		OnItemSelected(mDropDownItems.back());
	}
	

	return index;
}

size_t DropDown::AddDropDownItem(IWinBase* item)
{
	mDropDownItems.push_back((Button*)item);
	item->SetProperty(UIProperty::INHERIT_VISIBLE_TRUE, "false");
	item->ChangeNSize(Vec2(1.0f, 1.0f));
	item->ChangeNPos(Vec2(0.0f, 1.0f));
	Button* pDropDownItem = mDropDownItems.back();	
	size_t index = mDropDownItems.size() - 1;
	SetCommonProperty(pDropDownItem, index);
	if (mDropDownItems.size() == 1)
	{
		OnItemSelected(item);
	}
	if (mReservedIdx != -1 && mReservedIdx < mDropDownItems.size())
	{
		unsigned resv = mReservedIdx;
		mReservedIdx = -1;
		SetSelectedIndex(resv);
	}

	return index;
}

void DropDown::SetCommonProperty(IWinBase* item, size_t index)
{
	item->SetVisible(false);
	item->SetProperty(UIProperty::NO_BACKGROUND, "false");
	item->SetProperty(UIProperty::BACK_COLOR, "0, 0, 0, 1.0");
	item->SetProperty(UIProperty::BACK_COLOR_OVER, "0.1, 0.1, 0.1, 1.0");
	item->SetProperty(UIProperty::USE_SCISSOR, "false");
	item->SetProperty(UIProperty::SPECIAL_ORDER, "3");
	item->ChangeSizeY(24);
	item->SetInitialOffset(Vec2I(0, 24 * index));

	WinBase* wb = (WinBase*)item;
	wb->RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnItemSelected, this, std::placeholders::_1));
}

size_t DropDown::GetSelectedIndex() const
{
	return mCurIdx;
}

void DropDown::SetSelectedIndex(size_t index)
{
	if (index >= mDropDownItems.size())
	{
		assert(0);
		return;
	}

	mCurIdx = index;
	SetText(mDropDownItems[index]->GetText());
}

void DropDown::SetReservedIndex(size_t index)
{
	mReservedIdx = index;
}

bool DropDown::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (mDropDownItems.empty())
		return __super::OnInputFromHandler(mouse, keyboard);

	if (keyboard && keyboard->IsValid())
	{
		if (keyboard->IsKeyPressed(VK_ESCAPE))
		{
			if (mDropDownItems[0]->GetVisible())
			{
				for (auto var : mDropDownItems)
				{
					var->SetVisible(false);
				}
				keyboard->Invalidate();
			}
		}
	}
	return __super::OnInputFromHandler(mouse, keyboard);
}

void DropDown::OnParentVisibleChanged(bool show)
{
	if (!show)
	{
		for (auto var : mDropDownItems)
		{
			var->SetVisible(false);
		}
	}
}

void DropDown::ModifyItem(unsigned index, UIProperty::Enum prop, const char* szString){
	assert(index < mDropDownItems.size());
	mDropDownItems[index]->SetProperty(prop, szString);
	if (mCurIdx == index){
		SetProperty(prop, szString);
	}
}


}