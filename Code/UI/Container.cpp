#include <UI/StdAfx.h>
#include <UI/Container.h>
#include <UI/RadioBox.h>
#include <UI/IUIManager.h>
#include <UI/Scroller.h>
#include <UI/DropDown.h>

namespace fastbird
{

Container::Container()
	: mScrollerV(0)
	, mUseScrollerH(false), mUseScrollerV(false), mChildrenPosSizeChanged(false)
	, mWndContentUI(0), mChildrenChanged(false), mMatchHeight(false)
	, mCurInputHandlingChanged(false), mHandlingInput(false)
{
}

Container::~Container()
{
	for (auto winBase : mPendingDelete)
	{
		COMPONENTS::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		if (it != mChildren.end())
		{
			if (*it == mScrollerV)
				mScrollerV = 0;
			mChildren.erase(it);
			gFBEnv->pUIManager->DeleteComponent(winBase);
		}
		else
		{
			assert(0);
		}
	}
	mPendingDelete.clear();

	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		gFBEnv->pUIManager->DeleteComponent(*it);
	}
}

IWinBase* Container::AddChild(ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(type);
	}
	WinBase* pWinBase = (WinBase*)gFBEnv->pUIManager->CreateComponent(type);
	if (pWinBase)
	{
		pWinBase->SetHwndId(GetHwndId());
		mChildren.push_back(pWinBase);
		if (mNoMouseEvent)
		{
			pWinBase->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
		}
		pWinBase->SetParent(this);
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	}
	SetChildrenPosSizeChanged();
	return pWinBase;
}

IWinBase* Container::AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(posX, posY, width, height, type);
	}

	auto child = AddChild(type);
	if (child)
	{
		child->SetNSize(fastbird::Vec2(width, height));
		child->SetNPos(fastbird::Vec2(posX, posY));
		child->RefreshScissorRects(); // for scissor
		child->OnCreated();
	}

	return child;
}

IWinBase* Container::AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(pos, size, type);
	}

	auto child = AddChild(type);
	if (child)
	{
		child->SetSize(size);
		child->SetPos(pos);
		child->RefreshScissorRects(); // for scissor
		child->OnCreated();
	}

	return child;
}

IWinBase* Container::AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type)
{
	mChildrenChanged = true;
	if (mWndContentUI)
	{
		return mWndContentUI->AddChild(posX, posY, width_aspectRatio, type);
	}

	float width = width_aspectRatio.x;
	float ratio = width_aspectRatio.y;
	Vec2 worldSize = ConvertChildSizeToWorldCoord(Vec2(width, width));
	auto rtSize = GetRenderTargetSize();
	float iWidth = rtSize.x * worldSize.x;
	float iHeight = iWidth / ratio;
	float height = iHeight / (rtSize.y * mWNSize.y);
	WinBase* winbase = (WinBase*)AddChild(posX, posY, width, height, type);
	if (winbase)
		winbase->SetAspectRatio(ratio);
	return winbase;
}

IWinBase* Container::AddChild(const fastbird::LuaObject& compTable)
{
	mChildrenChanged = true;
	std::string typeText = compTable.GetField("type_").GetString();
	auto type = ComponentType::ConverToEnum(typeText.c_str());
	bool dropdown = GetType() == ComponentType::DropDown;
	IWinBase* p = AddChild(type);
	assert(p);
	p->ParseLua(compTable);
	p->RefreshScissorRects(); // for scissor
	p->OnCreated();	

	if (dropdown)
	{
		DropDown* dd = (DropDown*)this;
		dd->AddDropDownItem(p);
	}
	SetChildrenPosSizeChanged();
	return p;
}

void Container::RemoveChild(IWinBase* child, bool immediately)
{
	if (mWndContentUI)
	{
		return mWndContentUI->RemoveChild(child, immediately);
	}

	if (immediately)
	{
		if (mScrollerV == child)
			mScrollerV = 0;
		gFBEnv->pUIManager->DeleteComponent(child);
		DeleteValuesInList(mChildren, child);
	}
	else
	{
		if (ValueNotExistInVector(mPendingDelete, child))
			mPendingDelete.push_back(child);
	}
	SetChildrenPosSizeChanged();
}

void Container::RemoveChildNotDelete(IWinBase* child)
{
	if (mWndContentUI)
	{
		return mWndContentUI->RemoveChildNotDelete(child);
	}
	if (mScrollerV == child)
		mScrollerV = 0;
	if (mHandlingInput &&  mChildren.rend() != mCurInputHandling  &&  (*mCurInputHandling) == child)
	{
		auto test = *mCurInputHandling;
		auto nextIt = mChildren.erase(--mCurInputHandling.base());
		if (nextIt == mChildren.end())	{
			mCurInputHandling = mChildren.rbegin();
		}
		else{
			++nextIt;
			if (nextIt == mChildren.end())	{
				mCurInputHandling = mChildren.rbegin();
			}
			else
			{
				mCurInputHandling = COMPONENTS::reverse_iterator(nextIt);
			}
		}
		
		mCurInputHandlingChanged = true;
	}
	else
	{
		DeleteValuesInList(mChildren, child);
	}
}

void Container::RemoveAllChild(bool immediately)
{
	if (mWndContentUI)
	{
		return mWndContentUI->RemoveAllChild(immediately);
	}
	if (immediately)
	{
		while (!mChildren.empty())
		{
			if (mScrollerV == *mChildren.begin())
			{
				mScrollerV = 0;
			}
			gFBEnv->pUIManager->DeleteComponent(*mChildren.begin());
			mChildren.erase(mChildren.begin());
		}
		mChildrenChanged = true;
	}
	else
	{
		for (auto child : mChildren)
		{
			Container* childCont = dynamic_cast<Container*>(child);
			if (childCont)
				childCont->RemoveAllChild(false);
			child->ClearName();
			if (ValueNotExistInVector(mPendingDelete, child))
				mPendingDelete.push_back(child);
		}
	}
	SetChildrenPosSizeChanged();
}

IWinBase* Container::GetChild(const char* name, bool includeSubChildren/*= false*/)
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetChild(name, includeSubChildren);
	}

	for (auto var : mChildren)
	{
		if (stricmp(var->GetName(), name) == 0)
		{
			return var;
		}
	}
	if (includeSubChildren)
	{
		IWinBase* sub = 0;
		for (auto var : mChildren)
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
	mChildrenChanged = false;
	__super::OnStartUpdate(elapsedTime);

	for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
	{
		(*it)->OnStartUpdate(elapsedTime);
	}

	bool deleted = false;
	for (auto winBase : mPendingDelete)
	{
		COMPONENTS::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		if (it != mChildren.end())
		{
			if (*it == mScrollerV)
				mScrollerV = 0;
			mChildren.erase(it);
			gFBEnv->pUIManager->DeleteComponent(winBase);
			deleted = true;
		}
		else
		{
			assert(0);
		}
		mChildrenPosSizeChanged = true;
	}
	mPendingDelete.clear();
	if (deleted)
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());

	if (mChildrenPosSizeChanged)
	{
		mChildrenPosSizeChanged = false;
		if (mMatchHeight)
		{
			MatchHeight(false);
		}
		RefreshVScrollbar();
	}
}

void Container::OnClickRadio(RadioBox* pRadio)
{
	int groupID = pRadio->GetGroupID();
	for (auto winBase : mChildren)
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
	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		pWinBase->UpdateWorldPos(true);
		pWinBase->RefreshScissorRects();
	}
	RefreshVScrollbar();
}

void Container::OnSizeChanged()
{
	__super::OnSizeChanged();
	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		pWinBase->UpdateWorldSize();
		pWinBase->UpdateNPos();
		pWinBase->OnSizeChanged();
	}
	RefreshVScrollbar();
}

void Container::OnAlphaChanged()
{
	__super::OnAlphaChanged();
	for (auto i : mChildren)
	{
		i->OnAlphaChanged();
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
	if (mNSize.x == 0.0f)
		wc.x = worldSize.x;
	if (mNSize.y == 0.0f)
		wc.y = worldSize.y;

	if (mParent)
		wc = mParent->ConvertWorldSizeToParentCoord(wc);

	return wc;
}

Vec2 Container::ConvertChildPosToWorldCoord(const fastbird::Vec2& pos) const
{
	Vec2 wp = (mNPosAligned + mNPosOffset) + pos * mNSize;
	if (mParent)
		wp = mParent->ConvertChildPosToWorldCoord(wp);

	return wp;
}

Vec2 Container::ConvertWorldPosToParentCoord(const fastbird::Vec2& worldPos) const
{
	Vec2 pp = (worldPos - (mWNPos+mWNPosOffset)) / mWNSize;
	if (mWNSize.x == 0.0f)
	{
		pp.x = worldPos.x;
	}
	if (mWNSize.y == 0.0f)
	{
		pp.y = worldPos.y;
	}

	return pp; // pos in parent space
}



bool Container::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mouse->IsValid() && !keyboard->IsValid())
		return false;

	bool mouseIn = false;
	mHandlingInput = true;
	auto it = mChildren.rbegin();
	for (; it != mChildren.rend();)
	{		
		mCurInputHandlingChanged = false;
		mCurInputHandling = it;
		mouseIn = (*it)->OnInputFromHandler(mouse, keyboard) || mouseIn;
		if (mChildrenChanged)
			break;
		if (!mCurInputHandlingChanged)
		{
			++it;
		}
		else
		{
			it = mCurInputHandling;
		}
	}	
	mHandlingInput = false;

	if (keyboard->GetChar() == VK_TAB)
	{
		if (gFBUIManager->GetKeyboardFocusUI() == this)
		{
			keyboard->PopChar();
			TabPressed();
		}
	}

	if (mNoMouseEvent)
		return mouseIn;

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

void Container::RefreshVScrollbar()
{
	if (mWNSize.y == NotDefined)
		return;

	// find last wn
	float contentHeight = GetContentHeight();
	float boxWNEnd = mWNPos.y + mWNSize.y;
	float length = contentHeight - mWNSize.y;
	if (length > 0.0001f)
	{
		float visableRatio = mWNSize.y / (mWNSize.y + length);

		if (!mScrollerV && mUseScrollerV)
		{
			mScrollerV = static_cast<Scroller*>(AddChild(1.0f, 0.0f, 0.01f, 1.0f, ComponentType::Scroller));
			mScrollerV->SetRuntimeChild(true);
			mScrollerV->SetRender3D(mRender3D, GetRenderTargetSize());
			mScrollerV->SetSizeX(4);
			if (!mBorders.empty())
			{
				mScrollerV->SetProperty(UIProperty::OFFSETX, "-4");
			}
			mScrollerV->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			mScrollerV->SetProperty(UIProperty::BACK_COLOR, "0.46f, 0.46f, 0.36f, 0.7f");
			mScrollerV->SetOwner(this);
		}

		if (mScrollerV)
		{
			mScrollerV->SetNSizeY(visableRatio);
			mScrollerV->SetVisible(true);
			mScrollerV->SetMaxOffset(Vec2(0, length));
		}
	}
	else
	{
		if (mScrollerV)
		{
			mScrollerV->SetVisible(false);
			mScrollerV->ResetScroller();
			if (mScrollerV->GetVisible())
			{
				for (auto child : mChildren)
				{
					if (child->GetType() != ComponentType::Scroller)
					{
						child->SetWNPosOffset(Vec2(0, 0));
					}
				}
			}
		}
	}
	
}

float Container::GetContentHeight() const
{
	float contentWNEnd = 0;
	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		float wnEnd = pWinBase->GetWNPos().y + pWinBase->GetWNSize().y;
		contentWNEnd = std::max(wnEnd, contentWNEnd);
	}
	return contentWNEnd - mWNPos.y;
}

bool Container::SetVisible(bool visible)
{
	bool changed = __super::SetVisible(visible);
	if (changed)
	{
		for (auto var : mChildren)
		{
			if ((visible == true && var->GetInheritVisibleTrue()) || !visible)
			{
				var->SetVisible(visible);
			}
			var->OnParentVisibleChanged(visible);
		}
	}
	if (mMatchHeight)
	{
		MatchHeight(false);
	}
	return changed;
}

bool Container::SetVisibleChildren(bool show)
{
	for (auto var : mChildren)
	{
		var->SetVisible(show);
	}
	return true;
}

void Container::SetVisibleInternal(bool visible)
{
	__super::SetVisibleInternal(visible);
	for (auto var : mChildren)
	{
		if ((visible && var->GetInheritVisibleTrue()) || !visible)
		{
			var->SetVisibleInternal(visible);
		}
		var->OnParentVisibleChanged(visible);
	}
}

void Container::OnParentVisibleChanged(bool visible)
{
	for (auto var : mChildren)
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
		const Vec2& offset = mScrollerV->GetOffset();
		float curPos = -offset.y / maxOffset;
		float nPosY = movable * curPos;
		mScrollerV->SetNPosY(nPosY);
		{
			for (auto child : mChildren)
			{
				if (child->GetType() != ComponentType::Scroller)
				{
					child->SetWNPosOffset(offset);
				}
					
			}
		}
		TriggerRedraw();
	}
	else if (mWndContentUI)
	{
		mWndContentUI->Scrolled();
	}
}

void Container::SetWNPosOffset(const Vec2& offset)
{
	// scrollbar offset
	assert(GetType() != ComponentType::Scroller);
	__super::SetWNPosOffset(offset);
}

void Container::SetAnimNPosOffset(const Vec2& offset)
{
	__super::SetAnimNPosOffset(offset);
	for (auto item : mChildren)
	{
		item->SetAnimNPosOffset(offset);
	}
}

void Container::SetAnimScale(const Vec2& scale, const Vec2& pivot)
{
	__super::SetAnimScale(scale, pivot);
	for (auto item : mChildren)
	{
		item->SetAnimScale(scale, pivot);
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
			break;
		}
		ComponentType::Enum type = ComponentType::ConverToEnum(sz);		
		IWinBase* p = AddChild(type);
		assert(p);
		p->SetRender3D(mRender3D, GetRenderTargetSize());
		p->ParseXML(pchild);
		p->OnCreated();
		
		if (dropdown)
		{
			DropDown* dd = (DropDown*)this;
			dd->AddDropDownItem(p);
		}

		pchild = pchild->NextSiblingElement("component");
	}
	return true;
}

void Container::Save(tinyxml2::XMLElement& elem)
{
	__super::Save(elem);

	for (auto child : mChildren)
	{
		if (!child->IsRuntimeChild())
		{
			auto compElem = elem.GetDocument()->NewElement("component");
			elem.InsertEndChild(compElem);
			child->Save(*compElem);
		}
	}
}

bool Container::ParseLua(const fastbird::LuaObject& compTable)
{
	__super::ParseLua(compTable);
	auto children = compTable.GetField("children");
	bool dropdown = GetType() == ComponentType::DropDown;
	if (children.IsTable())
	{
		auto it = children.GetSequenceIterator();
		LuaObject child;
		while (it.GetNext(child))
		{
			std::string type = child.GetField("type_").GetString();
			if (type.empty())
			{
				Error("Component should have type_ attribute.");
				assert(0);
				break;
			}
			auto typee = ComponentType::ConverToEnum(type.c_str());
			if (typee != ComponentType::NUM)
			{
				IWinBase* p = AddChild(typee);
				assert(p);
				p->SetRender3D(mRender3D, GetRenderTargetSize());
				p->ParseLua(child);
				p->OnCreated();

				if (dropdown)
				{
					DropDown* dd = (DropDown*)this;
					dd->AddDropDownItem(p);
				}
			}
		}
	}

	if (mMatchHeight)
	{
		MatchHeight(false);
	}

	return true;
}

void Container::MatchHeight(bool checkName)
{
	float contentWNEnd = 0.f;
	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		if (pWinBase->GetVisible())
		{
			if (checkName && strlen(pWinBase->GetName())==0)
				continue;
			float wnEnd = pWinBase->GetWNPos().y + pWinBase->GetWNSize().y;
			contentWNEnd = std::max(wnEnd, contentWNEnd);
		}
	}

	float sizeY = contentWNEnd - mWNPos.y;
	if (mParent)
	{
		float nsizeY = ConvertWorldSizeToParentCoord(Vec2(sizeY, sizeY)).y;
		SetNSizeY(nsizeY);
	}
	else
	{
		SetNSizeY(sizeY);
	}

	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		if (pWinBase->GetVisible())
		{
			pWinBase->UpdateWorldPos();
		}
		pWinBase->RefreshScissorRects();
	}
}

bool Container::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::SCROLLERV:
	{
								  // SCROLLERV should set before TITLEBAR property.
								  assert(mWndContentUI == 0);
								  bool b = StringConverter::parseBool(val);
								  mUseScrollerV = b;
								  return true;
	}
	case UIProperty::SCROLLERV_OFFSET:
	{
		RefreshVScrollbar();
		if (mScrollerV)
		{
			auto offset = mScrollerV->GetOffset();
			offset.y = StringConverter::parseReal(val);
			mScrollerV->SetOffset(offset);
		}
	}
	case UIProperty::SCROLLERH:
	{
								  bool b = StringConverter::parseBool(val);
								  mUseScrollerH = b;
								  return true;
	}
	case UIProperty::MATCH_HEIGHT:
	{
								// call MatchUIHeight() lua function instead of using this property.
								mMatchHeight = StringConverter::parseBool(val);
								return true;
	}

	}

	return __super::SetProperty(prop, val);
}

bool Container::GetProperty(UIProperty::Enum prop, char val[], bool notDefaultOnly)
{

	switch (prop)
	{
	case UIProperty::SCROLLERV:
	{
		if (notDefaultOnly)
		{
			if (mUseScrollerV == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mUseScrollerV);
		strcpy(val, data.c_str());
		return true;
	}
	case UIProperty::SCROLLERV_OFFSET:
	{
		if (notDefaultOnly)
			return false;

		if (mScrollerV)
		{
			sprintf_s(val, 256, "%.4f", mScrollerV->GetOffset().y);
			return true;
		}
		else
		{
			sprintf_s(val, 256, "%.4f", 0.f);
			return true;
		}
	}
	case UIProperty::SCROLLERH:
	{
		if (notDefaultOnly)
		{
			if (mUseScrollerH == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mUseScrollerH);
		strcpy(val, data.c_str());
		return true;		
	}
	case UIProperty::MATCH_HEIGHT:
	{
		if (notDefaultOnly)
		{
			if (mMatchHeight == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		auto data = StringConverter::toString(mMatchHeight);
		strcpy(val, data.c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, notDefaultOnly);
}


void Container::RefreshScissorRects()
{
	__super::RefreshScissorRects();
	for (auto child : mChildren)
	{
		child->RefreshScissorRects();
	}
}

void Container::RemoveAllEvents(bool includeChildren)
{
	__super::RemoveAllEvents(includeChildren);
	if (includeChildren)
	{
		for (auto child : mChildren)
		{
			child->RemoveAllEvents(includeChildren);
		}

	}
}

float Container::PixelToLocalNWidth(int pixel) const
{
	if (mWndContentUI)
		return mWndContentUI->PixelToLocalNWidth(pixel);

	return __super::PixelToLocalNWidth(pixel);

}
float Container::PixelToLocalNHeight(int pixel) const
{
	if (mWndContentUI)
		return mWndContentUI->PixelToLocalNHeight(pixel);

	return __super::PixelToLocalNHeight(pixel);
}
Vec2 Container::PixelToLocalNSize(const Vec2I& pixel) const
{
	if (mWndContentUI)
		return mWndContentUI->PixelToLocalNSize(pixel);

	return __super::PixelToLocalNSize(pixel);
}

const Vec2& Container::GetScrollOffset() const
{
	assert(mScrollerV);
	return mScrollerV->GetOffset();
}

void Container::SetRender3D(bool render3D, const Vec2I& renderTargetSize)
{
	__super::SetRender3D(render3D, renderTargetSize);
	for (auto child : mChildren)
	{
		child->SetRender3D(render3D, renderTargetSize);
	}
}

void Container::StartHighlight(float speed)
{
	for (auto child : mChildren)
	{
		child->StartHighlight(speed);
	}
	__super::StartHighlight(speed);
}

void Container::StopHighlight()
{
	for (auto child : mChildren)
	{
		child->StopHighlight();
	}
	__super::StopHighlight();
}

void Container::SetHwndId(HWND_ID hwndId)
{
	__super::SetHwndId(hwndId);
	for (auto child : mChildren)
	{
		child->SetHwndId(hwndId);
	}
}

IWinBase* Container::WinBaseWithPoint(const Vec2I& pt, bool container) const
{
	for (auto child : mChildren)
	{
		if (container)
		{
			auto cont = dynamic_cast<Container*>(child);
			if (!cont)
				continue;
		}
		auto found = child->WinBaseWithPoint(pt, container);
		if (found)
			return found;
	}

	if (container)
		return 0;

	return __super::WinBaseWithPoint(pt, container);
}

IWinBase* Container::WinBaseWithTabOrder(unsigned tabOrder) const
{
	VectorMap<unsigned, IWinBase*> winbases;
	__super::GatherTabOrder(winbases);
	for (auto child : mChildren)
	{
		child->GatherTabOrder(winbases);
	}
	if (winbases.empty())
		return 0;

	for (auto it : winbases)
	{
		if (it.first >= tabOrder)
		{
			return it.second;
		}
	}
	return winbases.begin()->second;
}

void Container::GatherTabOrder(VectorMap<unsigned, IWinBase*>& winbases) const
{
	if (!mEnable)
		return;

	__super::GatherTabOrder(winbases);
	for (auto child : mChildren)
	{
		child->GatherTabOrder(winbases);
	}
}
// you need to check whether current focus has tab order
void Container::TabPressed()
{
	auto root = GetRootWnd();
	unsigned nextOrder = 0;
	auto keyfocusWnd = gFBUIManager->GetKeyboardFocusUI();
	if (keyfocusWnd)
	{
		auto tabOrder = keyfocusWnd->GetTabOrder();
		if (tabOrder != -1)
		{
			nextOrder = tabOrder + 1;
		}
	}
	auto win = root->WinBaseWithTabOrder(nextOrder);
	if (win)
	{
		gFBUIManager->SetFocusUI(win);
	}
}

}
