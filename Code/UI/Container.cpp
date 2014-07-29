#include <UI/StdAfx.h>
#include <UI/Container.h>
#include <UI/RadioBox.h>
#include <UI/IUIManager.h>

namespace fastbird
{

Container::~Container()
{
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		delete (*it);
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
	}
	return pWinBase;
}

void Container::RemoveChild(IWinBase* child)
{
	mPendingDelete.push_back(child);
}

void Container::OnStartUpdate()
{
	for each (IWinBase* winBase in mChildren)
	{
		winBase->OnStartUpdate();
	}
	for each (IWinBase* winBase in mPendingDelete)
	{
		COMPONENTS::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		mChildren.erase(it);
		
		IUIManager::GetUIManager().OnDeleteWinBase(winBase);
		delete winBase;
		
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



void Container::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mouse->IsValid() && !keyboard->IsValid())
		return;

	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		(*it)->OnInputFromHandler(mouse, keyboard);
	}	

	__super::OnInputFromHandler(mouse, keyboard);
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

}