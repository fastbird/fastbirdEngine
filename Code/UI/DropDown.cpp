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
{
	mUIObject = IUIObject::CreateUIObject(false);
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

	mButton = (Button*)IUIManager::GetUIManager().CreateComponent(ComponentType::Button);
	mButton->RegisterEventFunc(IEventHandler::EVENT_MOUSE_DOWN,
		std::bind(&DropDown::OnMouseClick, this, std::placeholders::_1));
	mButton->SetNSizeY(1.f);
	mButton->SetSizeX(24);
	mButton->SetNPos(Vec2(1, 0));
	mButton->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
	mButton->SetProperty(UIProperty::NO_BACKGROUND, "true");
	mButton->SetProperty(UIProperty::TEXTUREATLAS, "es/textures/ui.xml");
	mButton->SetProperty(UIProperty::BACKGROUND_IMAGE, "dropdown");
	mButton->SetProperty(UIProperty::BACKGROUND_IMAGE_HOVER, "dropdown_hover");
}

DropDown::~DropDown()
{
	IUIManager::GetUIManager().DeleteComponent(mButton);
}

void DropDown::GatherVisit(std::vector<IUIObject*>& v)
{
	if (!mVisible)
		return;
	v.push_back(mUIObject);
	__super::GatherVisit(v);
	mButton->GatherVisit(v);
}

void DropDown::OnPosChanged()
{
	__super::OnPosChanged();
	AlignText();
	
	Vec2 btnPos= mWNPos;
	btnPos.x += mWNSize.x;
	mButton->SetWNPos(btnPos);
}

void DropDown::OnSizeChanged()
{
	__super::OnSizeChanged();
	AlignText();

	mButton->SetWNSize(mWNSize);
	mButton->SetSizeX(24);
}

bool DropDown::SetProperty(UIProperty::Enum prop, const char* val)
{
	__super::SetProperty(prop, val);
	if (prop == UIProperty::TEXT_COLOR)
		mUIObject->SetTextColor(mTextColor);

	return true;
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
	IUIManager::GetUIManager().DirtyRenderList();
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
	mButton->OnMouseHover(0);
}
void DropDown::OnMouseOut(void* arg)
{
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
	IUIManager::GetUIManager().DirtyRenderList();
}

size_t DropDown::AddDropDownItem(WCHAR* szString)
{
	mDropDownItems.push_back((Button*)AddChild(0.0f, 1.0f, 1.0f, 1.0f, ComponentType::Button));
	Button* pDropDownItem = mDropDownItems.back();
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
	item->SetNPos(Vec2(0.0f, 1.0f));
	Button* pDropDownItem = mDropDownItems.back();	
	size_t index = mDropDownItems.size() - 1;
	SetCommonProperty(pDropDownItem, index);

	if (mDropDownItems.size() == 1)
	{
		OnItemSelected(item);
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

}