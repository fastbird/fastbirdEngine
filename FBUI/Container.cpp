
/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "StdAfx.h"
#include "Container.h"
#include "RadioBox.h"
#include "Scroller.h"
#include "DropDown.h"
#include "TextField.h"
#include "UIManager.h"

namespace fb
{

Container::Container()
	: mUseScrollerH(false), mUseScrollerV(false), mChildrenPosSizeChanged(false)
	, mChildrenChanged(false), mMatchHeight(false)
	, mCurInputHandlingChanged(false), mHandlingInput(false), mSendEventToChildren(false)
{
}

Container::~Container()
{
	/*for (auto winBase : mPendingDelete)
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
	}*/
}

void Container::OnResolutionChanged(HWindowId hwndId){
	__super::OnResolutionChanged(hwndId);
	for (auto it : mChildren){
		it->OnResolutionChanged(hwndId);
	}
}

WinBasePtr Container::AddChild(ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->AddChild(type);
	}
	auto pWinBase = UIManager::GetInstance().CreateComponent(type);
	if (pWinBase)
	{
		pWinBase->SetHwndId(GetHwndId());
		pWinBase->SetRender3D(mRender3D, GetRenderTargetSize());
		mChildren.push_back(pWinBase);
		pWinBase->SetParent(std::dynamic_pointer_cast<Container>(mSelfPtr.lock()));
		if (mNoMouseEvent)
		{
			pWinBase->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
		}		
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	}
	SetChildrenPosSizeChanged();
	return pWinBase;
}

WinBasePtr Container::AddChild(float posX, float posY, float width, float height, ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	auto contentUI = mWndContentUI.lock();
	if (contentUI && type != ComponentType::Scroller)
	{
		return contentUI->AddChild(posX, posY, width, height, type);
	}

	auto child = AddChild(type);
	if (child)
	{
		child->ChangeNSize(fb::Vec2(width, height));
		child->ChangeNPos(fb::Vec2(posX, posY));
		child->OnCreated();
	}

	return child;
}

WinBasePtr Container::AddChild(const Vec2I& pos, const Vec2I& size, ComponentType::Enum type)
{
	mChildrenChanged = true; // only detecting addition. not deletion.
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->AddChild(pos, size, type);
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

WinBasePtr Container::AddChild(float posX, float posY, const Vec2& width_aspectRatio, ComponentType::Enum type)
{
	mChildrenChanged = true;
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->AddChild(posX, posY, width_aspectRatio, type);
	}

	float width = width_aspectRatio.x;
	float ratio = width_aspectRatio.y;
	auto localRT = GetParentSize();
	float iWidth = localRT.x * width;
	float iHeight = iWidth / ratio;
	float height = iHeight / (float)localRT.y;
	auto winbase = AddChild(posX, posY, width, height, type);
	if (winbase)
		winbase->SetAspectRatio(ratio);
	return winbase;
}

WinBasePtr Container::AddChild(const fb::LuaObject& compTable)
{
	mChildrenChanged = true;
	std::string typeText = compTable.GetField("type_").GetString();
	auto type = ComponentType::ConvertToEnum(typeText.c_str());
	bool dropdown = GetType() == ComponentType::DropDown;
	WinBasePtr p = AddChild(type);
	assert(p);
	p->ParseLua(compTable);

	if (dropdown)
	{
		auto dd = std::dynamic_pointer_cast<DropDown>(mSelfPtr.lock());		
		dd->AddDropDownItem(p);
	}
	SetChildrenPosSizeChanged();
	return p;
}

void Container::RemoveChild(WinBasePtr child, bool immediately)
{
	if (child == 0)
		return;

	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->RemoveChild(child, immediately);
	}

	if (immediately)
	{
		if (mScrollerV.lock() == child)
			mScrollerV.reset();
		DeleteValuesInList(mChildren, child);
	}
	else
	{
		if (!ValueExistsInVector(mPendingDelete, child)) {
			child->ReservePendingDelete(true);
			mPendingDelete.push_back(child);
		}
	}
	SetChildrenPosSizeChanged();	
}

//void Container::RemoveChildNotDelete(WinBasePtr child)
//{
//	if (mWndContentUI)
//	{
//		return mWndContentUI->RemoveChildNotDelete(child);
//	}
//	if (mScrollerV == child)
//		mScrollerV = 0;
//	if (mHandlingInput &&  mChildren.rend() != mCurInputHandling  &&  *mCurInputHandling == child)
//	{
//		auto nextIt = mChildren.erase(--mCurInputHandling.base());
//		if (nextIt == mChildren.end())	{
//			mCurInputHandling = mChildren.rbegin();
//		}
//		else{
//			++nextIt;
//			if (nextIt == mChildren.end())	{
//				mCurInputHandling = mChildren.rbegin();
//			}
//			else
//			{
//				mCurInputHandling = COMPONENTS::reverse_iterator(nextIt);
//			}
//			auto temp = (*mCurInputHandling) == child;
//			assert(!temp);
//		}
//		
//		mCurInputHandlingChanged = true;
//	}
//	else if (mHandlingInput && mCurInputHandling.base() != mChildren.end() && *mCurInputHandling.base() == child){
//		auto nextIt = mChildren.erase(mCurInputHandling.base());
//		if (nextIt == mChildren.end())	{
//			mCurInputHandling = mChildren.rbegin();
//		}
//		else{
//			++nextIt;
//			if (nextIt == mChildren.end())	{
//				mCurInputHandling = mChildren.rbegin();
//			}
//			else
//			{
//				mCurInputHandling = COMPONENTS::reverse_iterator(nextIt);
//			}
//			auto temp = (*mCurInputHandling) == child;
//			assert(!temp);
//		}
//
//		mCurInputHandlingChanged = true;
//	}
//	else
//	{
//		DeleteValuesInList(mChildren, child);
//	}
//	if (UIManager::GetInstance().GetKeyboardFocusUI() == child)
//		UIManager::GetInstance().SetFocusUI(mSelfPtr.lock());
//
//	UIManager::GetInstance().DirtyRenderList(GetHwndId());
//}

void Container::RemoveAllChildren(bool immediately)
{
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->RemoveAllChildren(immediately);
	}
	if (immediately)
	{
		mChildren.clear();
		mChildrenChanged = true;
	}
	else
	{
		for (auto child : mChildren)
		{
			auto childCont = std::dynamic_pointer_cast<Container>(child);
			if (childCont)
				childCont->RemoveAllChildren(false);
			child->ClearName();
			if (!ValueExistsInVector(mPendingDelete, child)){
				mPendingDelete.push_back(child);				
			}
		}		
	}
	mScrollerV.reset();
	SetChildrenPosSizeChanged();
}

void Container::RemoveAllChildExceptRuntime(){
	auto contentUI = mWndContentUI.lock();
	if (contentUI){
		contentUI->RemoveAllChildExceptRuntime();
		return;
	}

	for (auto it = mChildren.begin(); it != mChildren.end();){
		auto child = *it;
		if (!child->IsRuntimeChild()){
			it = mChildren.erase(it);
		}
		else{
			++it;
		}
	}

	mChildrenChanged = true;
}

WinBasePtr Container::GetChild(const std::string& name, bool includeSubChildren/*= false*/)
{
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->GetChild(name, includeSubChildren);
	}

	for (auto var : mChildren)
	{
		if (_stricmp(var->GetName(), name.c_str()) == 0)
		{
			if (!var->IsPendingDeleteReserved())			
				return var;
		}
	}
	if (includeSubChildren)
	{
		WinBasePtr sub = 0;
		for (auto var : mChildren)
		{
			if (!var->IsPendingDeleteReserved()) {
				sub = var->GetChild(name, true);
				if (sub)
					return sub;
			}
		}
	}
	return 0;
}

WinBasePtr Container::GetChild(unsigned idx)
{
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->GetChild(idx);
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
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->GetNumChildren();
	}
	if (excludeRunTimeChild)
	{
		unsigned num = 0;
		for (auto& child : mChildren)
		{
			if (!child->IsRuntimeChild() && !child->IsPendingDeleteReserved())
				++num;
		}
		return num;
	}
	else
	{
		unsigned num = 0;
		for (auto& child : mChildren)
		{
			if (!child->IsPendingDeleteReserved())
				++num;
		}
		return num;
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
		ComponentPtrs::iterator it = std::find(mChildren.begin(), mChildren.end(), winBase);
		if (it != mChildren.end())
		{
			mChildren.erase(it);
			deleted = true;
		}
		else
		{
			assert(0);
		}
		SetChildrenPosSizeChanged();
	}
	mPendingDelete.clear();

	if (deleted)
		UIManager::GetInstance().DirtyRenderList(GetHwndId());

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
		if ((void*)winBase.get() != (void*)pRadio && winBase->GetType() == ComponentType::RadioBox)
		{
			auto pRadio = std::static_pointer_cast<RadioBox>(winBase);
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

void Container::GatherVisit(std::vector<UIObject*>& v)
{
	//char buf[512];
	//sprintf_s(buf, "Container::GatherVisit (%s)", mName.c_str());
	//Profiler p(buf);
	ComponentPtrs::iterator it = mChildren.begin(), itEnd = mChildren.end();
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




bool Container::OnInputFromHandler(IInputInjectorPtr injector)
{
	if (!injector->IsValid(InputDevice::Keyboard) && !injector->IsValid(InputDevice::Mouse))
		return false;

	bool mouseIn = false;
	mHandlingInput = true;
	auto it = mChildren.rbegin();
	for (; it != mChildren.rend();)
	{		
		mCurInputHandlingChanged = false;
		mCurInputHandling = it;
		mouseIn = (*it)->OnInputFromHandler(injector) || mouseIn;
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

	return __super::OnInputFromHandler(injector) || mouseIn;
}

void Container::NotifySizeChange() {
	for (auto& child : mChildren){
		child->SetParent(std::static_pointer_cast<Container>(mSelfPtr.lock()));
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
		ComponentPtrs::const_iterator it = mChildren.begin(), itEnd = mChildren.end();
		for (; it!=itEnd && !focused; it++)
		{
			focused = (*it)->GetFocus(includeChildren);
		}
	}

	return focused;
}

void Container::RefreshVScrollbar()
{
	if (mName == "shipdesc") {
		int a = 0;
		a++;
	}
	if (mMatchHeight) {
		auto scrollerV = mScrollerV.lock();
		if (scrollerV) {
			RemoveChild(scrollerV, false);
			mScrollerV.reset();
		}
		return;
	}
	auto contentUI = mWndContentUI.lock();
	if (contentUI){
		contentUI->RefreshVScrollbar();
		return;
	}

	auto scrollerV = mScrollerV.lock();
	if (!scrollerV && !mUseScrollerV)
		return;

	// find last wn
	int level = 1;
	float contentEnd = GetChildrenContentEnd(level) + 4.f / (float)GetRenderTargetSize().y;
	float contentHeight = contentEnd - mWNPos.y;
	const auto& finalSize = GetFinalSize();
	float wnSizeY = finalSize.y / (float)GetRenderTargetSize().y;
	
	float length = contentHeight - wnSizeY;
	if (length > 0.0001f)
	{
		float visableRatio = wnSizeY / contentHeight;

		if (!scrollerV && mUseScrollerV)
		{
			scrollerV = std::static_pointer_cast<Scroller>(AddChild(1.0f, 0.0f, 0.01f, 1.0f, ComponentType::Scroller));
			mScrollerV = scrollerV;
			scrollerV->SetRuntimeChild(true);
			scrollerV->SetRender3D(mRender3D, GetRenderTargetSize());
			scrollerV->ChangeSizeX(4);
			if (!mBorders.empty())
			{
				scrollerV->SetProperty(UIProperty::OFFSETX, "-4");
			}
			scrollerV->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
			scrollerV->SetUseAbsPos(false);
			scrollerV->SetProperty(UIProperty::BACK_COLOR, "0.46f, 0.46f, 0.36f, 0.7f");			
			scrollerV->SetVisible(true);
			const Vec2& offset = scrollerV->GetOffset();
			for (auto child : mChildren) {
				if (child->GetType() != ComponentType::Scroller) {
					child->SetWNScrollingOffset(offset);
				}
			}
		}

		if (scrollerV)
		{
			scrollerV->SetVisible(true);
			scrollerV->ChangeNSizeY(visableRatio);
			//scrollerV->OnPosChanged(false);			
			scrollerV->SetMaxOffset(Vec2(0, length));			
		}
	}
	else
	{
		if (scrollerV)
		{
			scrollerV->SetVisible(false);
			scrollerV->ResetScroller();
			if (scrollerV->GetVisible())
			{
				for (auto child : mChildren)
				{
					if (child->GetType() != ComponentType::Scroller)
					{
						child->SetWNScrollingOffset(Vec2(0, 0));
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

	for (auto& pWinBase : mChildren)
	{		
		float wnEnd = pWinBase->GetWNPos().y + pWinBase->GetContentHeight();
		contentWNEnd = std::max(wnEnd, contentWNEnd);
	}
	return contentWNEnd - mWNPos.y;
}

float Container::GetContentEnd(int& level) const {
	float end = __super::GetContentEnd(level);	
	if (level == 0 || mChildren.empty()){
		return end;
	}
	auto cend = GetChildrenContentEnd(level);
	end = std::max(cend, end);	
	return end;	
}

void Container::SetChildrenContentEndFunc(ChildrenContentEndFunc func){
	mChildrenContentEndFunc = func;
}

float Container::GetChildrenContentEnd(int& level) const{
	if (level == 0) {
		return  __super::GetContentEnd(level);
	}

	--level;

	auto contentWnd = mWndContentUI.lock();
	if (contentWnd) {
		return contentWnd->GetChildrenContentEnd(level);
	}

	if (mChildrenContentEndFunc){
		return mChildrenContentEndFunc();
	}

	float end = 0;
	for (auto& winbase : mChildren){		
		if (!winbase->IsPendingDeleteReserved()) {
			float cend = winbase->GetContentEnd(level);
			end = std::max(end, cend);
		}
	}
	return end;
}

int Container::GetChildrenContentEndInPixel() const {
	int level = 1;
	auto end = GetChildrenContentEnd(level);
	end *= GetParentSize().y;
	
	return Round(end);
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

Vec2 Container::Scrolled()
{
	if (mScrolledFunc){
		mScrolledFunc();
	}

	auto scrollerV = mScrollerV.lock();
	auto contentUI = mWndContentUI.lock();
	Vec2 offset(0, 0);
	if (scrollerV)
	{
		float visableRatio = scrollerV->GetNSize().y;
		float movable = 1.0f - visableRatio;
		float maxOffset = scrollerV->GetMaxOffset().y;
		offset = scrollerV->GetOffset();
		float curPos = -offset.y / maxOffset;
		float nPosY = movable * curPos;
		scrollerV->ChangeNPosY(nPosY);
		{
			for (auto child : mChildren)
			{
				if (child->GetType() != ComponentType::Scroller)
				{
					child->SetWNScrollingOffset(offset);
				}
			}
		}
		TriggerRedraw();
	}
	else if (contentUI && !contentUI->SetScrolledFunc())
	{
		contentUI->Scrolled();
	}
	return offset;
}

void Container::SetWNScrollingOffset(const Vec2& offset)
{
	// scrollbar offset
	assert(GetType() != ComponentType::Scroller);
	__super::SetWNScrollingOffset(offset);
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
			Logger::Log(FB_ERROR_LOG_ARG, "component doesn't have the type attribute.");
			break;
		}
		ComponentType::Enum type = ComponentType::ConvertToEnum(sz);
		WinBasePtr p = AddChild(type);
		assert(p);
		p->SetRender3D(mRender3D, GetRenderTargetSize());		
		if (dropdown) {
			p->SetProperty(UIProperty::TEXT_LEFT_GAP, "4");
		}
		p->ParseXML(pchild);
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
	for (auto& child : mChildren)
	{
		if (!child->IsRuntimeChild())
		{
			auto compElem = elem.GetDocument()->NewElement("component");
			elem.InsertEndChild(compElem);
			child->Save(*compElem);
		}
		else if(!child->IsRuntimeChildRecursive()){
			auto childCont = std::dynamic_pointer_cast<Container>(child);
			if (childCont){
				childCont->SaveChildren(elem);
			}
		}
	}
}

bool Container::ParseLua(const fb::LuaObject& compTable)
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
				Logger::Log(FB_ERROR_LOG_ARG, "Component should have type_ attribute.");				
				assert(0);
				continue;
			}
			auto typee = ComponentType::ConvertToEnum(type.c_str());
			if (typee != ComponentType::NUM)
			{
				WinBasePtr p = AddChild(typee);
				assert(p);
				p->SetRender3D(mRender3D, GetRenderTargetSize());
				p->ParseLua(child);				

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
	int level = 1;
	auto nend = GetChildrenContentEnd(level);
	auto contentWNEnd = Round(nend * GetRenderTargetSize().y);
	/*int contentWNEnd = 0;
	for (auto& pWinBase : mChildren)
	{
		if (pWinBase->GetVisible())
		{
			if (checkName && strlen(pWinBase->GetName())==0)
				continue;
			int wnEnd = (pWinBase->GetFinalPos().y + pWinBase->GetFinalSize().y);
			contentWNEnd = std::max(wnEnd, contentWNEnd);
		}
	}*/

	int sizeY = contentWNEnd - GetFinalPos().y;
	// todo: remove this hard coded number.
	// it relative the gap between descriptions and buttons in the docking ui
	// or the Item/Object tooltip.
	ChangeSizeY(sizeY+10);
}

bool Container::SetProperty(UIProperty::Enum prop, const char* val)
{
	switch (prop)
	{
	case UIProperty::SCROLLERV:
	{
		if (mName == "shipdesc") {
			int a = 0;
			a++;
		}
								  // SCROLLERV should set before TITLEBAR property.
								  bool b = StringConverter::ParseBool(val);
								  mUseScrollerV = b;
									RefreshVScrollbar();
								  return true;
	}
	case UIProperty::SCROLLERV_OFFSET:
	{
		RefreshVScrollbar();
		auto scrollerV = mScrollerV.lock();
		if (scrollerV)
		{
			auto offset = scrollerV->GetOffset();
			offset.y = StringConverter::ParseReal(val);
			scrollerV->SetOffset(offset);
		}
		return true;
	}
	case UIProperty::SCROLLERH:
	{
								  bool b = StringConverter::ParseBool(val);
								  mUseScrollerH = b;
								  return true;
	}
	case UIProperty::MATCH_HEIGHT:
	{
								// call MatchUIHeight() lua function instead of using this property.
								mMatchHeight = StringConverter::ParseBool(val);
								return true;
	}
	case UIProperty::SEND_EVENT_TO_CHILDREN:
	{
		mSendEventToChildren = StringConverter::ParseBool(val);
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
		auto data = StringConverter::ToString(mUseScrollerV);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::SCROLLERV_OFFSET:
	{
		if (notDefaultOnly)
			return false;
		
		auto contentWnd = mWndContentUI.lock();
		if (contentWnd) {
			return contentWnd->GetProperty(prop, val, bufsize, notDefaultOnly);
		}

		auto scrollerV = mScrollerV.lock();
		if (scrollerV)
		{
			sprintf_s(val, 256, "%.4f", scrollerV->GetOffset().y);
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
		auto data = StringConverter::ToString(mUseScrollerH);
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
		auto data = StringConverter::ToString(mMatchHeight);
		strcpy_s(val, bufsize, data.c_str());
		return true;
	}
	case UIProperty::SEND_EVENT_TO_CHILDREN:
	{
		if (notDefaultOnly){
			if (mSendEventToChildren == UIProperty::GetDefaultValueBool(prop))
				return false;
		}
		strcpy_s(val, bufsize, StringConverter::ToString(mSendEventToChildren).c_str());		
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
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->GetScrollOffset();		
	}
	auto scrollerV = mScrollerV.lock();
	if (!scrollerV)
		return Vec2::ZERO;

	return scrollerV->GetOffset();
}

void Container::SetScrollOffset(const Vec2& offset){
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		contentUI->SetScrollOffset(offset);
		return;
	}
	auto scrollerV = mScrollerV.lock();
	if (!scrollerV) {
		RefreshVScrollbar();
		scrollerV = mScrollerV.lock();
	}
	if (scrollerV){
		scrollerV->SetOffset(offset);
	}
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

void Container::SetHwndId(HWindowId hwndId)
{
	__super::SetHwndId(hwndId);
	for (auto child : mChildren)
	{
		child->SetHwndId(hwndId);
	}
}

WinBasePtr Container::WinBaseWithPoint(const Vec2I& pt, const RegionTestParam& param) const
{
	if (param.mCheckMouseEvent && mNoMouseEvent)
		return 0;

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
					auto cont = std::dynamic_pointer_cast<Container>(child);
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
		auto it = std::find(param.mExceptions.begin(), param.mExceptions.end(), mSelfPtr.lock());
		if (it == param.mExceptions.end())
			return mSelfPtr.lock();
	}
	return 0;
}

WinBasePtr Container::WinBaseWithTabOrder(unsigned tabOrder) const
{
	VectorMap<unsigned, WinBasePtr> winbases;
	GetRootWnd()->GatherTabOrder(winbases);
	if (winbases.empty())
		return 0;
	auto injector = InputManager::GetInstance().GetInputInjector();	
	if (injector->IsKeyDown(VK_SHIFT)){
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

void Container::GatherTabOrder(VectorMap<unsigned, WinBasePtr>& winbases) const
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
	auto keyfocusWnd = UIManager::GetInstance().GetKeyboardFocusUI();
	if (keyfocusWnd)
	{
		auto tabOrder = keyfocusWnd->GetTabOrder();
		auto injector = InputManager::GetInstance().GetInputInjector();		
		if (tabOrder == -1){
			nextOrder = 0;
		}
		else{
			int biggest = -1;
			GetRootWnd()->GetBiggestTabOrder(biggest);
			if (injector->IsKeyDown(VK_SHIFT)) {
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
			UIManager::GetInstance().SetFocusUI(win);
			if (win->GetType() == ComponentType::TextField){
				auto tf = std::static_pointer_cast<TextField>(win);
				tf->SelectAll();
			}
		}
		else
		{
			__super::TabPressed();
		}
	}
}

void Container::TransferChildrenTo(ContainerPtr destContainer){
	ComponentPtrs remained;
	auto contentUI = mWndContentUI.lock();
	for (auto& child : mChildren){
		bool transferred = false;		
		if (child != destContainer && child != contentUI){
			auto found = mDoNotTransfer.find(child);
			if (found == mDoNotTransfer.end()){
				destContainer->AddChild(child);
				transferred = true;
			}			
		}
		if (!transferred){
			remained.push_back(child);
		}
	}
	mChildren = remained;
	/*if (mWndContentUI == destContainer)
		mChildren.push_back(destContainer);*/
	mScrollerV.reset();
	SetChildrenPosSizeChanged();
}

void Container::AddChild(WinBasePtr child){
	if (!child)
		return;

	mChildrenChanged = true; // only detecting addition. not deletion.
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		contentUI->AddChild(child);
		return;
	}

	mChildren.push_back(child);	
	child->SetHwndId(GetHwndId());
	child->SetRender3D(mRender3D, GetRenderTargetSize());
	if (mNoMouseEvent)
	{
		child->SetProperty(UIProperty::NO_MOUSE_EVENT, "true");
	}
	child->SetParent(std::dynamic_pointer_cast<Container>(mSelfPtr.lock()));
	UIManager::GetInstance().DirtyRenderList( GetHwndId() );
	child->OnParentSizeChanged();
	child->OnParentPosChanged();

	SetChildrenPosSizeChanged();	

	auto scroller = mScrollerV.lock();
	if (scroller) {
		const Vec2& offset = scroller->GetOffset();
		if (child->GetType() != ComponentType::Scroller)
		{
			child->SetWNScrollingOffset(offset);
		}
	}
}

void Container::AddChildSimple(WinBasePtr child){
	if (!child)
		return;

	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		contentUI->AddChildSimple(child);
		return;
	}
	mChildren.push_back(child);
}

void Container::DoNotTransfer(WinBasePtr child){
	mDoNotTransfer.insert(child);
}

bool Container::HasScissorIgnoringChild() const{
	for (auto child : mChildren){
		if (!child->GetUseScissor())
			return true;
	}
	return false;
}

void Container::ProcessWheel(IInputInjectorPtr injector) {
	auto contentUI = mWndContentUI.lock();
	if (contentUI) {
		contentUI->ProcessWheel(injector);
	}

	for (auto child : mChildren) {
		child->ProcessWheel(injector);
	}
	__super::ProcessWheel(injector);
}

WinBase* Container::GetScroller(bool checkChildren) {
	auto contentUI = mWndContentUI.lock();
	if (contentUI) {
		auto scroller = contentUI->GetScroller(checkChildren);
		if (scroller)
			return scroller;
	}
	auto sc = mScrollerV.lock();
	if (sc) {
		return sc.get();
	}
	if (checkChildren) {
		for (auto child : mChildren) {
			auto scroller = child->GetScroller(checkChildren);
			if (scroller)
				return scroller;
		}
	}

	return __super::GetScroller(checkChildren);

}

void Container::OnMouseIn(IInputInjectorPtr injector, bool propergated){
	__super::OnMouseIn(injector, propergated);
	if (mSendEventToChildren){
		for (auto& it : mChildren){
			if (it->GetReceiveEventFromParent()){
				it->OnMouseIn(injector, true);
			}
		}
	}
}
void Container::OnMouseOut(IInputInjectorPtr injector, bool propergated){
	__super::OnMouseOut(injector, propergated);
	if (mSendEventToChildren){
		for (auto& it : mChildren){
			if (it->GetReceiveEventFromParent()){
				it->OnMouseOut(injector, true);
			}
		}
	}
}
void Container::OnMouseHover(IInputInjectorPtr injector, bool propergated){
	__super::OnMouseHover(injector, propergated);
	if (mSendEventToChildren){
		for (auto& it : mChildren){
			if (it->GetReceiveEventFromParent()){
				it->OnMouseHover(injector, true);
			}
		}
	}
}

void Container::ReservePendingDelete(bool pendingDelete) {
	auto contentUI = mWndContentUI.lock();
	if (contentUI)
	{
		return contentUI->ReservePendingDelete(pendingDelete);
	}
	__super::ReservePendingDelete(pendingDelete);
}

void Container::RemoveGapChildren() {
	auto contentUI = mWndContentUI.lock();
	if (contentUI) {
		contentUI->RemoveGapChildren();
		return;
	}

	if (mChildren.empty())
		return;

	struct comp {
		WinBase* c;
		int mEnd;

		comp(WinBase* _c) {
			c = _c;
			mEnd = c->GetPos().y + c->GetSize().y;
		}
		bool operator < (const comp& other) const {
			return mEnd < other.mEnd;
		}
	};

	std::vector<comp> comps;
	for (auto c : mChildren)
	{
		if (c && c != mScrollerV.lock() && !ValueExistsInVector(mPendingDelete, c))
			comps.push_back(comp(c.get()));
	}
	if (comps.empty())
		return;
	
	std::sort(comps.begin(), comps.end());
	auto num = (int)comps.size();
	for (int i = num-1; i >= 1; --i) {
		auto prevEnd = comps[i-1].mEnd;
		auto nowStart = comps[i].c->GetPos().y;
		if (nowStart - prevEnd > 10) {
			int move = (nowStart - prevEnd) - 10;
			for (int c = i; c < num; ++c) {
				comps[c].c->Move(Vec2I(0, -move));
			}
		}
	}
	auto prevEnd = 0;
	auto nowStart = comps[0].c->GetPos().y;
	if (nowStart - prevEnd > 10) {
		int move = (nowStart - prevEnd) - 10;
		for (int c = 0; c < num; ++c) {
			comps[c].c->Move(Vec2I(0, -move));
		}
	}
}

void Container::GetChildrenNames(LuaObject& t) {
	auto contentUI = mWndContentUI.lock();
	if (contentUI) {
		contentUI->GetChildrenNames(t);
		return;
	}
	int n = 1;
	for (auto c : mChildren) {
		if (!ValueExistsInVector(mPendingDelete, c))
			t.SetSeq(n++, c->GetName());
	}
}

bool Container::GetComponentsInRegion(const Rect& r, std::vector<WinBasePtr>& comps) {
	if (!GetVisible())
		return false;

	auto found = __super::GetComponentsInRegion(r, comps);
	if (!found) {
		for (auto c : mChildren) {
			c->GetComponentsInRegion(r, comps);
		}
	}
	return false;
}

}