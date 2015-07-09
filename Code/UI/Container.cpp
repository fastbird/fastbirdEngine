#include <UI/StdAfx.h>
#include <UI/Container.h>
#include <UI/RadioBox.h>
#include <UI/IUIManager.h>
#include <UI/Scroller.h>
#include <UI/DropDown.h>
#include <UI/TextField.h>

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
		pWinBase->SetRender3D(mRender3D, GetRenderTargetSize());
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
	if (mWndContentUI && type != ComponentType::Scroller)
	{
		return mWndContentUI->AddChild(posX, posY, width, height, type);
	}

	auto child = AddChild(type);
	if (child)
	{
		child->ChangeNSize(fastbird::Vec2(width, height));
		child->ChangeNPos(fastbird::Vec2(posX, posY));
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
		child->ChangeSize(size);
		child->ChangePos(pos);
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
	auto localRT = GetParentSize();
	float iWidth = localRT.x * width;
	float iHeight = iWidth / ratio;
	float height = iHeight / (float)localRT.y;
	WinBase* winbase = (WinBase*)AddChild(posX, posY, width, height, type);
	if (winbase)
		winbase->SetAspectRatio(ratio);
	return winbase;
}

IWinBase* Container::AddChild(const fastbird::LuaObject& compTable)
{
	mChildrenChanged = true;
	std::string typeText = compTable.GetField("type_").GetString();
	auto type = ComponentType::ConvertToEnum(typeText.c_str());
	bool dropdown = GetType() == ComponentType::DropDown;
	IWinBase* p = AddChild(type);
	assert(p);
	p->ParseLua(compTable);
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
	if (child == 0)
		return;

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
	if (mHandlingInput &&  mChildren.rend() != mCurInputHandling  &&  *mCurInputHandling == child)
	{
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
			auto temp = (*mCurInputHandling) == child;
			assert(!temp);
		}
		
		mCurInputHandlingChanged = true;
	}
	else if (mHandlingInput && mCurInputHandling.base() != mChildren.end() && *mCurInputHandling.base() == child){
		auto nextIt = mChildren.erase(mCurInputHandling.base());
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
			auto temp = (*mCurInputHandling) == child;
			assert(!temp);
		}

		mCurInputHandlingChanged = true;
	}
	else
	{
		DeleteValuesInList(mChildren, child);
	}
	if (gFBUIManager->GetKeyboardFocusUI() == child)
		gFBUIManager->SetFocusUI(this);
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

IWinBase* Container::GetChild(const std::string& name, bool includeSubChildren/*= false*/)
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetChild(name, includeSubChildren);
	}

	for (auto var : mChildren)
	{
		if (_stricmp(var->GetName(), name.c_str()) == 0)
		{
			auto it = std::find(mPendingDelete.begin(), mPendingDelete.end(), var);
			if (it == mPendingDelete.end())
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
	if (mChildren.empty())
		return 0;
	if (idx >= mChildren.size())
		return 0;
	auto it = mChildren.begin();
	while (idx)
	{
		++it;
	}
	return *it;

}

unsigned Container::GetNumChildren(bool excludeRunTimeChild) const
{
	if (mWndContentUI)
	{
		return mWndContentUI->GetNumChildren();
	}
	if (excludeRunTimeChild)
	{
		unsigned num = 0;
		for (auto& child : mChildren)
		{
			if (!child->IsRuntimeChild())
				++num;
		}
		return num;
	}
	else
	{
		return mChildren.size();
	}
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

void Container::OnSizeChanged()
{
	__super::OnSizeChanged();
	NotifySizeChange();

	RefreshVScrollbar();
}

void Container::OnPosChanged(bool anim){
	__super::OnPosChanged(anim);
	NotifyPosChange();
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
	//char buf[512];
	//sprintf_s(buf, "Container::GatherVisit (%s)", mName.c_str());
	//Profiler p(buf);
	COMPONENTS::iterator it = mChildren.begin(), itEnd = mChildren.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetVisible() && !(*it)->GetGatheringException())
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

	if (mNoMouseEvent)
		return mouseIn;

	return __super::OnInputFromHandler(mouse, keyboard) || mouseIn;
}

void Container::NotifySizeChange() {
	for (auto& child : mChildren){
		child->OnParentSizeChanged();
	}
}

void Container::NotifyPosChange(){
	for (auto& child : mChildren){
		child->OnParentPosChanged();
	}
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
	if (mWndContentUI){
		mWndContentUI->RefreshVScrollbar();
		return;
	}

	if (!mScrollerV && !mUseScrollerV)
		return;

	// find last wn
	float contentEnd = GetChildrenContentEnd() + 4.f / (float)GetRenderTargetSize().y;
	float contentHeight = contentEnd - mWNPos.y;
	const auto& finalSize = GetFinalSize();
	float wnSizeY = finalSize.y / (float)GetRenderTargetSize().y;
	
	float length = contentHeight - wnSizeY;
	if (length > 0.0001f)
	{
		float visableRatio = wnSizeY / contentHeight;

		if (!mScrollerV && mUseScrollerV)
		{
			mScrollerV = static_cast<Scroller*>(AddChild(1.0f, 0.0f, 0.01f, 1.0f, ComponentType::Scroller));
			mScrollerV->SetRuntimeChild(true);
			mScrollerV->SetRender3D(mRender3D, GetRenderTargetSize());
			mScrollerV->ChangeSizeX(4);
			if (!mBorders.empty())
			{
				mScrollerV->SetProperty(UIProperty::OFFSETX, "-4");
			}
			mScrollerV->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			mScrollerV->SetUseAbsPos(false);
			mScrollerV->SetProperty(UIProperty::BACK_COLOR, "0.46f, 0.46f, 0.36f, 0.7f");
			mScrollerV->SetOwner(this);
			mScrollerV->SetVisible(true);
		}

		if (mScrollerV)
		{
			mScrollerV->SetVisible(true);
			mScrollerV->ChangeNSizeY(visableRatio);
			//mScrollerV->OnPosChanged(false);			
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
						child->SetWNScollingOffset(Vec2(0, 0));
					}
				}
			}
		}
	}
	
}

float Container::GetContentHeight() const
{
	float contentWNEnd = 0;
	if (mChildren.empty())
		return GetFinalSize().y / (float)GetRenderTargetSize().y;

	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		float wnEnd = pWinBase->GetWNPos().y + pWinBase->GetContentHeight();
		contentWNEnd = std::max(wnEnd, contentWNEnd);
	}
	return contentWNEnd - mWNPos.y;
}

float Container::GetContentEnd() const{
	float end = __super::GetContentEnd();
	if (mChildren.empty()){
		return end;
	}

	for (auto i : mChildren){
		WinBase* winbase = (WinBase*)i;
		float cend = winbase->GetContentEnd();
		end = std::max(end, cend);		
	}
	return end;	
}

void Container::SetChildrenContentEndFunc(ChildrenContentEndFunc func){
	mChildrenContentEndFunc = func;
}

float Container::GetChildrenContentEnd() const{
	if (mChildrenContentEndFunc){
		return mChildrenContentEndFunc();
	}

	float end = 0;
	for (auto i : mChildren){
		WinBase* winbase = (WinBase*)i;
		float cend = winbase->GetContentEnd();
		end = std::max(end, cend);
	}
	return end;
}

void Container::SetSpecialOrder(int specialOrder){
	__super::SetSpecialOrder(specialOrder);
	for (auto c : mChildren){
		if (c->IsRuntimeChild()){
			c->SetSpecialOrder(specialOrder);
		}
	}
}

bool Container::SetVisible(bool visible)
{
	bool changed = __super::SetVisible(visible);
	for (auto var : mChildren)
	{
		if ((visible == true && var->GetInheritVisibleTrue()) || !visible)
		{
			var->SetVisible(visible);
		}
		var->OnParentVisibleChanged(visible);
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
			var->SetVisible(visible);
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

void Container::SetScrolledFunc(ScrolledFunc func){
	mScrolledFunc = func;
}
void Container::Scrolled()
{
	if (mScrolledFunc){
		mScrolledFunc();
	}

	if (mScrollerV)
	{
		float visableRatio = mScrollerV->GetNSize().y;
		float movable = 1.0f - visableRatio;
		float maxOffset = mScrollerV->GetMaxOffset().y;
		const Vec2& offset = mScrollerV->GetOffset();
		float curPos = -offset.y / maxOffset;
		float nPosY = movable * curPos;
		mScrollerV->ChangeNPosY(nPosY);
		{
			for (auto child : mChildren)
			{
				if (child->GetType() != ComponentType::Scroller)
				{
					child->SetWNScollingOffset(offset);
				}
					
			}
		}
		TriggerRedraw();
	}
	else if (mWndContentUI && !mWndContentUI->SetScrolledFunc())
	{
		mWndContentUI->Scrolled();
	}
}

void Container::SetWNScollingOffset(const Vec2& offset)
{
	// scrollbar offset
	assert(GetType() != ComponentType::Scroller);
	__super::SetWNScollingOffset(offset);
}

bool Container::ParseXML(tinyxml2::XMLElement* pelem)
{
	__super::ParseXML(pelem);
	ParseXMLChildren(pelem);
	
	return true;
}

void Container::ParseXMLChildren(tinyxml2::XMLElement* pelem){
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
		ComponentType::Enum type = ComponentType::ConvertToEnum(sz);
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
		p->SetVisible(GetVisible());

		pchild = pchild->NextSiblingElement("component");
	}
}

void Container::Save(tinyxml2::XMLElement& elem)
{
	__super::Save(elem);
	SaveChildren(elem);
	
}

void Container::SaveChildren(tinyxml2::XMLElement& elem){
	for (auto child : mChildren)
	{
		if (!child->IsRuntimeChild())
		{
			auto compElem = elem.GetDocument()->NewElement("component");
			elem.InsertEndChild(compElem);
			child->Save(*compElem);
		}
		else if(!child->IsRuntimeChildRecursive()){
			auto childCont = dynamic_cast<Container*>(child);
			if (childCont){
				childCont->SaveChildren(elem);
			}
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
			auto typee = ComponentType::ConvertToEnum(type.c_str());
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
	int contentWNEnd = 0;
	for (auto i : mChildren)
	{
		WinBase* pWinBase = (WinBase*)i;
		if (pWinBase->GetVisible())
		{
			if (checkName && strlen(pWinBase->GetName())==0)
				continue;
			int wnEnd = (pWinBase->GetFinalPos().y + pWinBase->GetFinalSize().y);
			contentWNEnd = std::max(wnEnd, contentWNEnd);
		}
	}

	int sizeY = contentWNEnd - GetFinalPos().y;
	ChangeSizeY(sizeY);
}

bool Container::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::SCROLLERV:
	{
								  // SCROLLERV should set before TITLEBAR property.
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

bool Container::GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly)
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
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
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}

	}

	return __super::GetProperty(prop, val, bufsize, notDefaultOnly);
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

IWinBase* Container::WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const
{
	auto in = IsIn(pt, param.mIgnoreScissor);
	bool hasScissorIgnoringChild = HasScissorIgnoringChild();
	if (in || hasScissorIgnoringChild){
		if (param.mTestChildren){
			for (auto child : mChildren)
			{
				if (!child->GetVisible())
					continue;
				
				if (param.mCheckMouseEvent && child->GetNoMouseEvent())
					continue;

				if (!in && child->GetUseScissor())
					continue;

				if (param.mOnlyContainer)
				{
					auto cont = dynamic_cast<Container*>(child);
					if (!cont)
						continue;
				}
				auto found = child->WinBaseWithPoint(pt, param);
				if (found){
					auto it = std::find(param.mExceptions.begin(), param.mExceptions.end(), found);
					if (it == param.mExceptions.end())
						return found;
				}
			}
		}
	}
	if (GetGhost() || (param.mNoRuntimeComp && mRunTimeChild))
		return 0;
	
	if (param.mCheckMouseEvent && GetNoMouseEventAlone())
		return 0;

	if (in && GetVisible()){
		auto it = std::find(param.mExceptions.begin(), param.mExceptions.end(), this);
		if (it == param.mExceptions.end())
			return (IWinBase*)this;
	}
	return 0;
}

IWinBase* Container::WinBaseWithTabOrder(unsigned tabOrder) const
{
	VectorMap<unsigned, IWinBase*> winbases;
	GetRootWnd()->GatherTabOrder(winbases);
	if (winbases.empty())
		return 0;

	auto keyboard = gFBEnv->pEngine->GetKeyboard();
	if (keyboard->IsKeyDown(VK_SHIFT)){
		for (auto rit = winbases.rbegin(); rit != winbases.rend(); ++rit){
			if ((*rit).first <= tabOrder)
			{
				return (*rit).second;
			}
		}
	}
	else
	{
		for (auto it : winbases)
		{
			if (it.first >= tabOrder)
			{
				return it.second;
			}
		}
	}
	return 0;
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

void Container::GetBiggestTabOrder(int& curBiggest) const{
	__super::GetBiggestTabOrder(curBiggest);
	for (auto child : mChildren){
		child->GetBiggestTabOrder(curBiggest);
	}
}

// you need to check whether current focus has tab order
void Container::TabPressed()
{
		int nextOrder = 0;
	auto keyfocusWnd = gFBUIManager->GetKeyboardFocusUI();
	if (keyfocusWnd)
	{
		auto tabOrder = keyfocusWnd->GetTabOrder();
		auto keyboard = gFBEnv->pEngine->GetKeyboard();
		if (tabOrder == -1){
			nextOrder = 0;
		}
		else{
			int biggest = -1;
			GetRootWnd()->GetBiggestTabOrder(biggest);
			if (keyboard->IsKeyDown(VK_SHIFT)) {
				nextOrder = tabOrder - 1;
				if (nextOrder == -1)
				{					
					nextOrder = biggest;
				}
			}
			else{
				nextOrder = tabOrder + 1;
				if (nextOrder > biggest){
					nextOrder = 0;
				}
			}
		}
	}
	if (nextOrder != -1){
		auto win = WinBaseWithTabOrder(nextOrder);
		if (win)
		{
			gFBUIManager->SetFocusUI(win);
			if (win->GetType() == ComponentType::TextField){
				TextField* tf = (TextField*)win;
				tf->SelectAll();
			}
		}
		else
		{
			__super::TabPressed();
		}
	}
}

void Container::TransferChildrenTo(Container* destContainer){
	COMPONENTS remained;
	for (auto child : mChildren){
		if (child != destContainer && child != mWndContentUI){
			auto found = mDoNotTransfer.find(child);
			if (found == mDoNotTransfer.end()){
				destContainer->AddChild(child);
			}
			else{
				remained.push_back(child);
			}
		}
	}
	mChildren = remained;
	if (mWndContentUI == destContainer)
		mChildren.push_back(destContainer);
	mScrollerV = 0;
	SetChildrenPosSizeChanged();
}

void Container::AddChild(IWinBase* child){
	if (!child)
		return;

	mChildrenChanged = true; // only detecting addition. not deletion.
	if (mWndContentUI)
	{
		mWndContentUI->AddChild(child);
		return;
	}

	mChildren.push_back(child);	
	child->SetHwndId(GetHwndId());
	child->SetRender3D(mRender3D, GetRenderTargetSize());
	if (mNoMouseEvent)
	{
		child->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	}
	WinBase* winbase = (WinBase*)child;
	winbase->SetParent(this);
	gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	winbase->OnParentSizeChanged();
	winbase->OnParentPosChanged();

	SetChildrenPosSizeChanged();	
}

void Container::AddChildSimple(IWinBase* child){
	if (!child)
		return;

	if (mWndContentUI)
	{
		mWndContentUI->AddChildSimple(child);
		return;
	}
	mChildren.push_back(child);
}

void Container::DoNotTransfer(IWinBase* child){
	mDoNotTransfer.insert(child);
}

bool Container::HasScissorIgnoringChild() const{
	for (auto child : mChildren){
		if (!child->GetUseScissor())
			return true;
	}
	return false;
}

}