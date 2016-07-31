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
#include "TextField.h"
#include "KeyboardCursor.h"
#include "UIManager.h"
#include "ImageBox.h"
#include "PropertyList.h"
#include "ListItem.h"
#include "ListBox.h"
#include "UIEvents.h"
#include "FBInputManager/TextManipulator.h"
#include "UIObject.h"
#include "IUIEditor.h"

namespace fb
{
const float TextField::LEFT_GAP = 0.001f;

TextFieldPtr TextField::Create(){
	TextFieldPtr p(new TextField, [](TextField* obj){ delete obj; });
	p->mSelfPtr = p;
	return p;
}

TextField::TextField()
	: WinBase()
	, mPasswd(false)
	, mCursorOffset(0)
{
	mUIObject = UIObject::Create(GetRenderTargetSize(), this);
	mUIObject->SetMaterial("EssentialEngineData/materials/UITextField.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextSize(mTextSize);
	mUIObject->SetTextColor(mTextColor);
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&TextField::OnClicked, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&TextField::OnDoubleClicked, this, std::placeholders::_1));
}

TextField::~TextField()
{
}

void TextField::GatherVisit(std::vector<UIObject*>& v)
{
	v.push_back(mUIObject.get());	
	if (UIManager::GetInstance().GetKeyboardFocusUI().get() == this)
		v.push_back(KeyboardCursor::GetInstance().GetUIObject().get());

	__super::GatherVisit(v);
}

bool TextField::OnInputFromHandler(IInputInjectorPtr injector)
{
	if (!mEnable)
		return false;

	bool mouseIn = __super::OnInputFromHandler(injector);

	if (!mVisibility.IsVisible() || !GetFocus(false))
		return mouseIn;

	if (injector->IsValid(InputDevice::Keyboard)) {
		auto ch = injector->GetChar();
		if (ch == VK_TAB)	{
			if (IsKeyboardFocused())
			{
				auto listbox = IsInListBox();
				if (listbox)
				{
					injector->PopChar();
					if (injector->IsKeyDown(VK_SHIFT)) {
						listbox->IterateItem(false, true);
					}
					else{
						listbox->IterateItem(true, true);
					}
				}
				else{
					OnEvent(UIEvents::EVENT_ENTER);
				}
			}
		}
		else if (ch == VK_RETURN)
		{
			bool succ = false;
			if (OnEvent(UIEvents::EVENT_ENTER))
			{
				succ = true;
			}
			else
			{
				auto listbox = IsInListBox();
				if (listbox)
				{
					auto eventHandler = dynamic_cast<EventHandler*>(listbox.get());
					if (eventHandler)
					{
						if (eventHandler->OnEvent(UIEvents::EVENT_ENTER)){
							succ = true;
						}
					}
				}
			}
			if (succ){
				injector->PopChar();
				injector->Invalidate(InputDevice::Keyboard);
				TriggerRedraw();
				
			}
		}
		else if (ch == VK_ESCAPE){
			auto prop = IsInPropertyList();
			if (prop) {
				prop->MoveFocusToKeyItem();
				injector->PopChar();
				injector->Invalidate(InputDevice::Keyboard);
			}
		}
		else{
			UIManager::GetInstance().GetTextManipulator()->ConsumeInput(injector, mouseIn);
		}
	}
	
	return mouseIn;
}

void TextField::OnFocusGain()
{
	KeyboardCursor::GetInstance().SetHwndId(GetHwndId());
	UIManager::GetInstance().DirtyRenderList(mHwndId);

	auto mani = UIManager::GetInstance().GetTextManipulator();
	mani->AddObserver(ITextManipulatorObserver::Default, std::dynamic_pointer_cast<ITextManipulatorObserver>(mSelfPtr.lock()));
	mani->SetText(&mTextw);
	

	auto propertyList = IsInPropertyList();
	if (propertyList)
	{
		ListItem* listItem = (ListItem*)GetParent().get();
		propertyList->SetFocusRow(listItem->GetRowIndex());
	}

	KeyboardCursor::GetInstance().SetScissorRegion(GetScissorRegion());
}

void TextField::OnFocusLost()
{
	UIManager::GetInstance().DirtyRenderList(mHwndId);
	auto mani = UIManager::GetInstance().GetTextManipulator();
	auto propertyList = IsInPropertyList();
	if (propertyList)
	{
		auto parent = GetParent();
		assert(parent && parent->GetType() == ComponentType::ListItem);
		ListItem* valueItem = (ListItem*)parent.get();
		auto index = valueItem->GetRowIndex();
		propertyList->RemoveHighlight(index);
		auto uiEditor = UIManager::GetInstance().GetUIEditor();
		auto editingUI = uiEditor->GetCurSelected();
		std::string key, value;
		propertyList->GetCurKeyValue(key, value);
		if (editingUI)
		{
			char buf[UIManager::PROPERTY_BUF_SIZE] = { 0 };
			auto prop = UIProperty::IsUIProperty(key.c_str());
			if (prop != UIProperty::COUNT){
				auto got = editingUI->GetProperty(prop, buf, UIManager::PROPERTY_BUF_SIZE, false);
				if (got)
					SetText(AnsiToWide(buf));
			}
			else{
				auto e = UIEvents::IsUIEvents(key.c_str());
				if (e != UIEvents::EVENT_NUM){
					auto szEvent = editingUI->GetEvent(e);
					editingUI->SetEvent(e, szEvent);
				}
			}
		}
	}
	mani->SetText(0);
	mani->RemoveObserver(ITextManipulatorObserver::Default, std::dynamic_pointer_cast<ITextManipulatorObserver>(mSelfPtr.lock()));
	TriggerRedraw();
}

void TextField::SetPasswd(bool passwd)
{
	mPasswd = passwd;
}

void TextField::OnCursorPosChanged(TextManipulator* mani)
{	
	TriggerRedraw();
	int cursorPos = mani->GetCursorPos();	
	Vec2I textStartWPos = mUIObject->GetTextStartWPos();
	textStartWPos.x += mCursorOffset;
	if (mani->IsHighlighting())
	{
		auto start = mani->GetHighlightStart();
		auto end = cursorPos;
		if (start > end)
		{
			std::swap(start, end);
		}
		if (end - start > 0){
			auto font = Renderer::GetInstance().GetFontWithHeight(mTextSize);
			if (font){
				float width = font->GetTextWidth(
					((const char*)mTextw.c_str()) + (start * 2),
					(end - start) * 2);
				float leftGap = font->GetTextWidth(
					(const char*)mTextw.c_str(), start * 2);				
				KeyboardCursor::GetInstance().SetSize(Vec2I((int)width, (int)mTextSize));
				KeyboardCursor::GetInstance().SetPos(
					Vec2I(textStartWPos.x + (int)leftGap,
					textStartWPos.y - Round(mTextSize))
					);
			}
		}
		else{
			auto font = Renderer::GetInstance().GetFontWithHeight(mTextSize);
			float aWidth = font->GetTextWidth((const char*)AnsiToWide("A", 1), 2);
			Vec2I cursorSize(Round(aWidth), 2);			
			KeyboardCursor::GetInstance().SetSize(Vec2I((int)1, (int)mTextSize));
			KeyboardCursor::GetInstance().SetPos(
				Vec2I(textStartWPos.x, textStartWPos.y - Round(mTextSize))
				);
		}
	}
	else
	{
		auto font = Renderer::GetInstance().GetFontWithHeight(mTextSize);
		float aWidth = font->GetTextWidth((const char*)AnsiToWide("A", 1), 2);
		Vec2I cursorSize(Round(aWidth), 2);
		KeyboardCursor::GetInstance().SetSize(cursorSize);
		float width = font->GetTextWidth(
			(const char*)mTextw.c_str(), cursorPos * 2);	

		Vec2I visualCursorPos(textStartWPos.x + Round(width),
			textStartWPos.y - WinBase::BOTTOM_GAP - 2);
		KeyboardCursor::GetInstance().SetPos(visualCursorPos);

		// check region
		// right
		Rect region = GetScissorRegion();
		if (region.right - mTextGap.y < visualCursorPos.x + cursorSize.x){
			int offset = visualCursorPos.x + cursorSize.x - (region.right - mTextGap.y);
			mCursorOffset -= offset;
			mUIObject->SetTextOffsetForCursorMovement(Vec2I(mCursorOffset, 0));
			KeyboardCursor::GetInstance().SetPos(Vec2I(visualCursorPos.x - offset, visualCursorPos.y));
		}
		else{
			if (region.left + mTextGap.x > visualCursorPos.x){
				int offset = region.left + mTextGap.x - visualCursorPos.x;
				mCursorOffset += offset;
				mUIObject->SetTextOffsetForCursorMovement(Vec2I(mCursorOffset, 0));
				KeyboardCursor::GetInstance().SetPos(Vec2I(visualCursorPos.x + offset, visualCursorPos.y));
			}
		}
		

	}
}
void TextField::OnTextChanged(TextManipulator* mani)
{
	TriggerRedraw();
	SetText(mTextw.c_str());
	mTextBeforeTranslated.clear();
}

void TextField::SetUseBorder(bool use)
{
	if (use && mBorders.empty())
	{
		auto T = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		T->SetHwndId(GetHwndId());
		mBorders.push_back(T);
		T->SetRender3D(mRender3D, GetRenderTargetSize());
		T->SetManualParent(mSelfPtr.lock());
		T->ChangeSizeY(1);
		T->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");


		auto L = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		L->SetHwndId(GetHwndId());
		mBorders.push_back(L);
		L->SetRender3D(mRender3D, GetRenderTargetSize());
		L->SetManualParent(mSelfPtr.lock());
		L->ChangeSizeX(1);
		L->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		auto R = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		R->SetHwndId(GetHwndId());
		mBorders.push_back(R);
		R->SetRender3D(mRender3D, GetRenderTargetSize());
		R->SetManualParent(mSelfPtr.lock());
		R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		R->ChangeSizeX(1);
		R->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");


		auto B = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		B->SetHwndId(GetHwndId());
		mBorders.push_back(B);
		B->SetRender3D(mRender3D, GetRenderTargetSize());
		B->SetManualParent(mSelfPtr.lock());
		B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		B->ChangeSizeY(1);
		B->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		auto LT = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		LT->SetHwndId(GetHwndId());
		mBorders.push_back(LT);
		LT->SetRender3D(mRender3D, GetRenderTargetSize());
		LT->SetManualParent(mSelfPtr.lock());
		LT->ChangeSize(Vec2I(1, 1));
		LT->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		auto RT = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		RT->SetHwndId(GetHwndId());
		mBorders.push_back(RT);
		RT->SetRender3D(mRender3D, GetRenderTargetSize());
		RT->SetManualParent(mSelfPtr.lock());
		RT->ChangeSize(Vec2I(1, 1));
		RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		RT->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		auto LB = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		LB->SetHwndId(GetHwndId());
		mBorders.push_back(LB);
		LB->SetRender3D(mRender3D, GetRenderTargetSize());
		LB->SetManualParent(mSelfPtr.lock());
		LB->ChangeSize(Vec2I(1, 1));
		LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		LB->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		auto RB = std::static_pointer_cast<ImageBox>(UIManager::GetInstance().CreateComponent(ComponentType::ImageBox));
		RB->SetHwndId(GetHwndId());
		mBorders.push_back(RB);
		RB->SetRender3D(mRender3D, GetRenderTargetSize());
		RB->SetManualParent(mSelfPtr.lock());
		RB->ChangeSize(Vec2I(1, 1));
		RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		RB->SetTextureAtlasRegion("EssentialEngineData/textures/ui.xml", "ThinBorder");

		RefreshBorder();
		RefreshScissorRects();
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	}
	else if (!use && !mBorders.empty())
	{
		mBorders.clear();
		UIManager::GetInstance().DirtyRenderList(GetHwndId());
	}
}

void TextField::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	if (UIManager::GetInstance().GetKeyboardFocusUI().get() == this)
	{
		OnCursorPosChanged(UIManager::GetInstance().GetTextManipulator());
		KeyboardCursor::GetInstance().SetScissorRegion(GetScissorRegion());
	}
}

void TextField::OnSizeChanged(){
	__super::OnSizeChanged();
	if (UIManager::GetInstance().GetKeyboardFocusUI().get() == this)
	{
		OnCursorPosChanged(UIManager::GetInstance().GetTextManipulator());
		KeyboardCursor::GetInstance().SetScissorRegion(GetScissorRegion());
	}
}

PropertyListPtr TextField::IsInPropertyList() const
{
	auto parent = GetParent();
	if (parent && parent->GetType() == ComponentType::ListItem)
	{
		auto pp = parent->GetParent();
		if (pp && pp->GetType() == ComponentType::PropertyList)
		{
			return std::dynamic_pointer_cast<PropertyList>(pp);
		}
	}
	return 0;
}

ListBoxPtr TextField::IsInListBox() const{
	auto parent = GetParent();
	if (parent && parent->GetType() == ComponentType::ListItem)
	{
		auto pp = parent->GetParent();
		if (pp && pp->GetType() == ComponentType::ListBox){
			return std::dynamic_pointer_cast<ListBox>(pp);
		}
	}
	return 0;
}

void TextField::SelectAll()
{
	UIManager::GetInstance().GetTextManipulator()->SelectAll();
	TriggerRedraw();
}

void TextField::OnClicked(void* arg){
	assert(this == arg);
	if (!IsKeyboardFocused()){
		UIManager::GetInstance().SetFocusUI(mSelfPtr.lock());
	}
	auto injector = InputManager::GetInstance().GetInputInjector();	
	long x, y;
	injector->GetMousePos(x, y);
	Vec2I cursorPos(x, y);
	const auto& finalPos = GetFinalPos();
	cursorPos = cursorPos - finalPos;
	cursorPos.x -= mTextGap.x;
	cursorPos.x += mCursorOffset;

	auto font = Renderer::GetInstance().GetFontWithHeight(mTextSize);
	if (font)
	{		
		float length = 0.f;
		if (cursorPos.x >= (int)mTextWidth){
			UIManager::GetInstance().GetTextManipulator()->SetCursorPos(mTextw.size());
		}
		else{
			for (int i = 0; i < (int)mTextw.size(); i++)
			{
				float halfLength = font->GetTextWidth((const char*)&mTextw[i], 2) *.5f;
				length += halfLength;
				if (cursorPos.x < length)
				{
					UIManager::GetInstance().GetTextManipulator()->SetCursorPos(i);
					break;
				}
				length += halfLength;
			}
		}		
	}
	injector->Invalidate(InputDevice::Mouse);

}
void TextField::OnDoubleClicked(void* arg){
	UIManager::GetInstance().GetTextManipulator()->SelectAll();
}
}