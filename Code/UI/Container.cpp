#include <UI/StdAfx.h>
#include <UI/Container.h>
#include <UI/RadioBox.h>
#include <UI/IUIManager.h>
#include <UI/Scroller.h>

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
	WinBase* pWinBase = (WinBase*)IUIManager::GetUIManager().CreateComponent(type);
	if (pWinBase)
	{
		mChildren.push_back(pWinBase);
		pWinBase->SetParent(this);
		pWinBase->SetNSize(fastbird::Vec2(width, height));
		pWinBase->SetNPos(fastbird::Vec2(posX, posY));
		Container* pContainer = dynamic_cast<Container*>(pWinBase);
		IUIManager::GetUIManager().DirtyRenderList();
	}
	return pWinBase;
}

IWinBase* Container::AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type)
{
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
	mPendingDelete.push_back(child);
}

IWinBase* Container::GetChild(const char* name)
{
	for each (IWinBase* var in mChildren)
	{
		if (stricmp(var->GetName(), name) == 0)
		{
			return var;
		}
	}
	return 0;
}

void Container::OnStartUpdate(float elapsedTime)
{
	__super::OnStartUpdate(elapsedTime);

	for each (IWinBase* winBase in mChildren)
	{
		winBase->OnStartUpdate(elapsedTime);
	}

	for each (IWinBase* winBase in mPendingDelete)
	{
		COMPONENTS::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		mChildren.erase(it);
		
		IUIManager::GetUIManager().OnDeleteWinBase(winBase);
		FB_SAFE_DEL(winBase);
		
	}
	mPendingDelete.clear();
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
		pWinBase->UpdateWorldPos();
	}
}

void Container::OnSizeChanged()
{
	__super::OnSizeChanged();
	for each(WinBase* pWinBase in mChildren)
	{
		pWinBase->UpdateWorldSize();
		pWinBase->OnSizeChanged();
	}

}

void Container::GatherVisit(std::vector<IUIObject*>& v)
{
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		(*it)->GatherVisit(v);
	}
}

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

IWinBase* Container::FocusTest(Vec2 normalizedMousePos)
{
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	IWinBase* foundWnd = 0;
	for (; it!=itEnd && !foundWnd; it++)
	{
		foundWnd = (*it)->FocusTest(normalizedMousePos);
	}

	if (!foundWnd)
		foundWnd = __super::FocusTest(normalizedMousePos);

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
	float contentWNEnd = child->GetWNPos().y + child->GetWNSize().y;
	float boxWNEnd = mWNPos.y + mWNSize.y;

	if (contentWNEnd > boxWNEnd && !mScroller)
	{
		mScroller = static_cast<Scroller*>(AddChild(0.99f, 0.0f, 0.01f, 1.0f, ComponentType::Scroller));
		mScroller->SetOwner(this);
	}

	if (mScroller)
	{
		float length = contentWNEnd - boxWNEnd;
		if (length>0)
			mScroller->SetMaxOffset(Vec2(0, length));
	}
}

void Container::Scrolled()
{
	if (mScroller)
	{
		Vec2 offset = mScroller->GetOffset();
		for each(auto item in mChildren)
		{
			if (item->GetType() != ComponentType::Scroller)
				item->SetNPosOffset(offset);
		}
	}
}

void Container::SetNPosOffset(const Vec2& offset)
{
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
		pchild = pchild->NextSiblingElement("component");
	}
	return true;
}

}