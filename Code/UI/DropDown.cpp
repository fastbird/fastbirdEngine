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
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));

	RegisterEventFunc(IEventHandler::EVENT_MOUSE_HOVER,
		std::bind(&DropDown::OnMouseHover, this, std::placeholders::_1));
	RegisterEventFunc(IEventHandler::EVENT_MOUSE_OUT,
		std::bind(&DropDown::OnMouseOut, this, std::placeholders::_1));
}

DropDown::~DropDown()
{
	if (sCurrentDropDown == this)
		sCurrentDropDown = 0;
	gFBEnv->pUIManager->DeleteComponent(mButton);
	mDropDownItems.clear();
}

void DropDown::OnCreated()
{
	mButton = (Button*)gFBEnv->pUIManager->CreateComponent(ComponentType::Button);

	mButton->SetHwndId(GetHwndId());
	mButton->SetRender3D(mRender3D, GetRenderTargetSize());
	mButton->RegisterEventFunc(IEventHandler::EVENT_MOUSE_DOWN,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));
	mButton->SetSize(Vec2I(24, 24));
	mButton->SetProperty(UIProperty::ALIGNH, "right");
	Vec2I btnPos = GetFinalPos();
	btnPos.x += GetFinalSize().x;
	mButton->ChangePos(btnPos);

	mButton->SetProperty(UIProperty::NO_BACKGROUND, "true");
	mButton->SetProperty(UIProperty::TEXTUREATLAS, "es/textures/ui.xml");
	mButton->SetProperty(UIProperty::REGION, "dropdown");
	mButton->SetProperty(UIProperty::HOVER_IMAGE, "dropdown_hover");


}

void DropDown::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisibility.IsVisible())
		return;
	v.push_back(mUIObject);
	__super::GatherVisit(v);
	if (mButton)
		mButton->GatherVisit(v);
}

void DropDown::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	
	if (mButton){
		Vec2I btnPos = GetFinalPos();
		btnPos.x += GetFinalSize().x;
		mButton->ChangePos(btnPos);
	}
		
}

void DropDown::OnSizeChanged()
{
	__super::OnSizeChanged();
	AlignText();

	if (mButton)
	{
		mButton->SetSizeY(GetFinalSize().y);
		mButton->SetSizeX(24);
		mButton->OnSizeChanged();
	}
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

bool DropDown::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
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
		strcpy(val, data.c_str());
		return true;
	}

	}
	
	return __super::GetProperty(prop, val, notDefaultOnly);
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
	for (auto var : mDropDownItems)
	{
		var->SetVisible(false);
	}
}

void DropDown::OnMouseHover(void* arg)
{
	if (mButton)
		mButton->OnMouseHover(0);
}
void DropDown::OnMouseOut(void* arg)
{
	if (mButton)
		mButton->OnMouseOut(0);
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
	OnEvent(IEventHandler::EVENT_DROP_DOWN_SELECTED);
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
}

size_t DropDown::AddDropDownItem(WCHAR* szString)
{
	mDropDownItems.push_back((Button*)AddChild(0.0f, 1.0f, 1.0f, 1.0f, ComponentType::Button));
	Button* pDropDownItem = mDropDownItems.back();
	// dropdown need to be saved.
//	pDropDownItem->SetRuntimeChild(true);
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
	item->SetNSize(Vec2(1.0f, 1.0f));
	item->SetNPos(Vec2(0.0f, (float)mDropDownItems.size()));
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
	item->SetSizeY(24);
	item->SetInitialOffset(Vec2I(0, 24 * index));

	WinBase* wb = (WinBase*)item;
	wb->RegisterEventFunc(IEventHandler::EVENT_MOUSE_LEFT_CLICK,
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


bool DropDown::SetVisible(bool show)
{
	bool ret = __super::SetVisible(show);
	for (auto item : mDropDownItems)
	{
		item->SetVisible(false);
	}
	mButton->SetVisible(show);
	return ret;
}

void DropDown::SetHwndId(HWND_ID hwndId)
{
	__super::SetHwndId(hwndId);
	if (mButton)
		mButton->SetHwndId(hwndId);

}

}