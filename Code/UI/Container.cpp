#include <UI/StdAfx.h>
#include <UI/Container.h>
#include <UI/RadioBox.h>
#include <UI/IUIManager.h>
#include <UI/Scroller.h>
#include <UI/DropDown.h>

namespace fastbird
{

Container::~Container()
{
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		IUIManager::GetUIManager().DeleteComponent(*it);
	}
}

IWinBase* Container::AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
{
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(posX, posY, width, height, type);
	}

	WinBase* pWinBase = (WinBase*)IUIManager::GetUIManager().CreateComponent(type);
	if (pWinBase)
	{
		mChildren.push_back(pWinBase);
		pWinBase->SetParent(this);
		pWinBase->SetNSize(fastbird::Vec2(width, height));
		pWinBase->SetNPos(fastbird::Vec2(posX, posY));
		Container* pContainer = dynamic_cast<Container*>(pWinBase);
		for each(IWinBase* win in mChildren)
		{
			win->RefreshScissorRects(); // for scissor
		}
		IUIManager::GetUIManager().DirtyRenderList();
	}
	return pWinBase;
}

IWinBase* Container::AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type)
{
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(posX, posY, width_aspectRatio, type);
	}

	float width = width_aspectRatio.x;
	float ratio = width_aspectRatio.y;
	Vec2 worldSize = ConvertChildSizeToWorldCoord(Vec2(width, width));
	float iWidth = gEnv->pRenderer->GetWidth() * worldSize.x;
	float iHeight = iWidth / ratio;
	float height = iHeight / (gEnv->pRenderer->GetHeight() * mWNSize.y);
	WinBase* pWinBase = (WinBase*)AddChild(posX, posY, width, height, type);
	if (pWinBase)
		pWinBase->SetAspectRatio(ratio);

	return pWinBase;
}

void Container::RemoveChild(IWinBase* child)
{
	if (mWndContentUI)
	{
		return mWndContentUI->RemoveChild(child);
	}
	if (mBottomChild == child)
		mBottomChild = 0;
	mPendingDelete.push_back(child);
}

IWinBase* Container::GetChild(const char* name, bool includeSubChildren/*= false*/)
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetChild(name, includeSubChildren);
	}

	for each (IWinBase* var in mChildren)
	{
		if (stricmp(var->GetName(), name) == 0)
		{
			return var;
		}
	}
	if (includeSubChildren)
	{
		IWinBase* sub = 0;
		for each (IWinBase* var in mChildren)
		{
			sub = var->GetChild(name, true);
			if (sub)
				return sub;
		}
	}
	return 0;
}

IWinBase* Container::GetChild(unsigned idx)
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetChild(idx);
	}
	assert(idx < mChildren.size());
	auto it = mChildren.begin();
	while (idx)
	{
		++it;
	}
	return *it;

}

unsigned Container::GetNumChildren() const
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetNumChildren();
	}

	return mChildren.size();
}

void Container::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);

	for each (IWinBase* winBase in mChildren)
	{
		winBase->OnStartUpdate(elapsedTime);
	}

	bool deleted = false;
	for each (IWinBase* winBase in mPendingDelete)
	{
		COMPONENTS::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		mChildren.erase(it);
		
		IUIManager::GetUIManager().DeleteComponent(winBase);		
		deleted = true;
	}
	mPendingDelete.clear();
	if (deleted)
		IUIManager::GetUIManager().DirtyRenderList();
}

void Container::OnClickRadio(RadioBox* pRadio)
{
	int groupID = pRadio->GetGroupID();
	for each (IWinBase* winBase in mChildren)
	{
		if ((void*)winBase != (void*)pRadio && winBase->GetType() == ComponentType::RadioBox)
		{
			RadioBox* pRadio = (RadioBox*)winBase;
			if (pRadio->GetGroupID()==groupID)
				pRadio->SetCheck(false);
		}
	}
}

void Container::OnPosChanged()
{
	__super::OnPosChanged();
	for each(WinBase* pWinBase in mChildren)
	{
		pWinBase->UpdateWorldPos(true);
		pWinBase->RefreshScissorRects();
	}
}

void Container::OnSizeChanged()
{
	__super::OnSizeChanged();
	for each(WinBase* pWinBase in mChildren)
	{
		pWinBase->UpdateWorldSize();
		pWinBase->OnSizeChanged();
		pWinBase->RefreshScissorRects();
	}

}

void Container::GatherVisit(std::vector<IUIObject*>& v)
{
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetVisible())
			(*it)->GatherVisit(v);
	}
	__super::GatherVisit(v);
}

//void Container::GatherVisitAlpha(std::vector<IUIObject*>& v)
//{
//	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
//	for (; it != itEnd; it++)
//	{
//		if ((*it)->GetAlphaBlending())
//			(*it)->GatherVisit(v);
//	}
//}

Vec2 Container::ConvertChildSizeToWorldCoord(const fastbird::Vec2& size) const
{
	Vec2 wc = size * mNSize;
	if (mParent)
		wc = mParent->ConvertChildSizeToWorldCoord(wc);

	return wc;
}

Vec2 Container::ConvertWorldSizeToParentCoord(const fastbird::Vec2& worldSize) const
{
	Vec2 wc = worldSize / mNSize;
	if (mParent)
		wc = mParent->ConvertWorldSizeToParentCoord(wc);

	return wc;
}

Vec2 Container::ConvertChildPosToWorldCoord(const fastbird::Vec2& pos) const
{
	Vec2 wp = mNPosAligned + pos * mNSize;
	if (mParent)
		wp = mParent->ConvertChildPosToWorldCoord(wp);

	return wp;
}

Vec2 Container::ConvertWorldPosToParentCoord(const fastbird::Vec2& worldPos) const
{
	Vec2 pp = (worldPos - mWNPos) / mWNSize;

	return pp; // pos in parent space
}



bool Container::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mouse->IsValid() && !keyboard->IsValid())
		return false;

	bool mouseIn = false;
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		mouseIn = (*it)->OnInputFromHandler(mouse, keyboard) || mouseIn;
	}	

	return __super::OnInputFromHandler(mouse, keyboard) || mouseIn;
}

IWinBase* Container::FocusTest(IMouse* mouse)
{
	IWinBase* foundWnd = 0;
	/*COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd && !foundWnd; it++)
	{
		foundWnd = (*it)->FocusTest(normalizedMousePos);
	}

	if (!foundWnd)*/
		foundWnd = __super::FocusTest(mouse);

	return foundWnd;
}

bool Container::GetFocus(bool includeChildren /*= false*/) const
{
	bool focused = __super::GetFocus(includeChildren);

	if (!focused && includeChildren)
	{
		COMPONENTS::const_iterator it = mChildren.begin(), itEnd = mChildren.end();
		for (; it!=itEnd && !focused; it++)
		{
			focused = (*it)->GetFocus(includeChildren);
		}
	}

	return focused;
}

void Container::OnChildPosSizeChanged(WinBase* child)
{
	if (child == mScrollerV)
		return;
	float contentWNEnd = child->GetWNPos().y + child->GetWNSize().y;
	float prevContentWNEnd = 0.0f;
	if (mBottomChild)
		prevContentWNEnd = mBottomChild->GetWNPos().y + mBottomChild->GetWNSize().y;
		
	if (mLastContentWNEnd < contentWNEnd)
	{
		mBottomChild = child;
		mLastContentWNEnd = contentWNEnd;

		float boxWNEnd = mWNPos.y + mWNSize.y;
		float length = contentWNEnd - boxWNEnd;
		float visableRatio = mWNSize.y / (mWNSize.y + length);

		if (contentWNEnd > boxWNEnd && !mScrollerV && mUseScrollerV)
		{
			mScrollerV = static_cast<Scroller*>(AddChild(1.0f, 0.0f, 0.01f, 1.0f, ComponentType::Scroller));
			mScrollerV->SetSizeX(2);
			mScrollerV->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			mScrollerV->SetProperty(UIProperty::BACK_COLOR, "0.46f, 0.46f, 0.36f, 0.7f");
			mScrollerV->SetOwner(this);
		}

		if (mScrollerV)
		{
			mScrollerV->SetNSizeY(visableRatio);
			mScrollerV->SetMaxOffset(Vec2(0, length));				
		}
	}
	
}

void Container::SetVisible(bool visible)
{
	__super::SetVisible(visible);
	for each (auto var in mChildren)
	{
		var->OnParentVisibleChanged(visible);
	}
}

void Container::OnParentVisibleChanged(bool visible)
{
	for each (auto var in mChildren)
	{
		var->OnParentVisibleChanged(visible);
	}
}

void Container::Scrolled()
{
	if (mScrollerV)
	{
		float visableRatio = mScrollerV->GetNSize().y;
		float movable = 1.0f - visableRatio;
		float maxOffset = mScrollerV->GetMaxOffset().y;
		Vec2 offset = mScrollerV->GetOffset();
		float curPos = -offset.y / maxOffset;
		float nPosY = movable * curPos;
		mScrollerV->SetNPosY(nPosY);
		for each(auto item in mChildren)
		{
			if (item->GetType() != ComponentType::Scroller)
				item->SetNPosOffset(offset);
		}
	}
}

void Container::SetNPosOffset(const Vec2& offset)
{
	// scrollbar offset
	assert(GetType() != ComponentType::Scroller);
	__super::SetNPosOffset(offset);
	for each(auto item in mChildren)
	{
		if (item->GetType() != ComponentType::Scroller)
			item->SetNPosOffset(offset);
	}
}

void Container::SetAnimNPosOffset(const Vec2& offset)
{
	__super::SetAnimNPosOffset(offset);
	for each(auto item in mChildren)
	{
		item->SetAnimNPosOffset(offset);
	}
}

bool Container::ParseXML(tinyxml2::XMLElement* pelem)
{
	__super::ParseXML(pelem);

	tinyxml2::XMLElement* pchild = pelem->FirstChildElement("component");
	bool dropdown = GetType() == ComponentType::DropDown;
	while (pchild)
	{
		const char* sz = pchild->Attribute("type");
		if (!sz)
		{
			Error("component doesn't have the type attribute.");
			continue;
		}
		ComponentType::Enum type = ComponentType::ConverToEnum(sz);		
		IWinBase* p = AddChild(0.0f, 0.0f, 1.0f, 1.0f, type);
		assert(p);
		p->ParseXML(pchild);
		
		if (dropdown)
		{
			DropDown* dd = (DropDown*)this;
			dd->AddDropDownItem(p);
		}

		pchild = pchild->NextSiblingElement("component");
	}
	return true;
}

bool Container::SetProperty(UIProperty::Enum prop, const char* val)
{
	if (prop == UIProperty::SCROLLERV)
	{
		// SCROLLERV should set before TITLEBAR property.
		assert(mWndContentUI == 0);
		bool b = StringConverter::parseBool(val);
		mUseScrollerV = b;
		return true;
	}

	if (prop == UIProperty::SCROLLERH)
	{
		bool b = StringConverter::parseBool(val);
		mUseScrollerH = b;
		return true;
	}

	return __super::SetProperty(prop, val);
}


void Container::RefreshScissorRects()
{
	__super::RefreshScissorRects();
	for each (IWinBase* child in mChildren)
	{
		child->RefreshScissorRects();
	}
}

void Container::RemoveAllEvents(bool includeChildren)
{
	__super::RemoveAllEvents(includeChildren);
	if (includeChildren)
	{
		for each (IWinBase* child in mChildren)
		{
			child->RemoveAllEvents(includeChildren);
		}

	}
}

}
