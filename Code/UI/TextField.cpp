#include <UI/StdAfx.h>
#include <UI/TextField.h>
#include <UI/KeyboardCursor.h>
#include <UI/IUIManager.h>
#include <UI/ImageBox.h>
#include <UI/PropertyList.h>
#include <UI/ListItem.h>
#include <UI/ListBox.h>
#include <UI/IUIEditor.h>
#include <UI/UIEvents.h>
#include <Engine/TextManipulator.h>


namespace fastbird
{
const float TextField::LEFT_GAP = 0.001f;

TextField::TextField()
	: WinBase()
	, mPasswd(false)
	, mCursorOffset(0)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, GetRenderTargetSize());
	mUIObject->SetMaterial("es/Materials/UITextField.material");
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
	mUIObject->SetTextColor(mTextColor);
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_CLICK,
		std::bind(&TextField::OnClicked, this, std::placeholders::_1));
	RegisterEventFunc(UIEvents::EVENT_MOUSE_LEFT_DOUBLE_CLICK,
		std::bind(&TextField::OnDoubleClicked, this, std::placeholders::_1));
}

TextField::~TextField()
{
	auto mani = gFBUIManager->GetTextManipulator();
	if (mani){
		mani->RemoveListener(this);
	}
}

void TextField::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
	if (gFBUIManager->GetKeyboardFocusUI() == this)
		v.push_back(KeyboardCursor::GetKeyboardCursor().GetUIObject());

	__super::GatherVisit(v);
}

bool TextField::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mEnable)
		return false;

	bool mouseIn = __super::OnInputFromHandler(mouse, keyboard);

	if (!mVisibility.IsVisible() || !GetFocus(false))
		return mouseIn;

	if (keyboard->IsValid()) {
		auto ch = keyboard->GetChar();
		if (ch == VK_TAB)	{
			if (IsKeyboardFocused())
			{
				auto listbox = IsInListBox();
				if (listbox)
				{
					keyboard->PopChar();
					if (keyboard->IsKeyDown(VK_SHIFT)) {
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
					auto eventHandler = dynamic_cast<IEventHandler*>(listbox);
					if (eventHandler)
					{
						if (eventHandler->OnEvent(UIEvents::EVENT_ENTER)){
							succ = true;
						}
					}
				}
			}
			if (succ){
				keyboard->PopChar();
				keyboard->Invalidate();
				TriggerRedraw();
				
			}
		}
		else if (ch == VK_ESCAPE){
			auto prop = IsInPropertyList();
			if (prop) {
				prop->MoveFocusToKeyItem();
				keyboard->PopChar();
				keyboard->Invalidate();
			}
		}
		else{
			gFBUIManager->GetTextManipulator()->OnInput(mouse, keyboard);
		}
	}
	
	return mouseIn;
}

void TextField::SetText(const wchar_t* szText)
{
	mTextw = szText;
	if (mUIObject)
	{
		if (!mPasswd)
		{
			mUIObject->SetText(szText);
		}
		else
		{
			std::wstring asterisks(mTextw.size(), L'*');
			mUIObject->SetText(asterisks.c_str());
		}
	}
	CalcTextWidth();

	AlignText();
	TriggerRedraw();
}

void TextField::OnFocusGain()
{
	KeyboardCursor::GetKeyboardCursor().SetHwndId(GetHwndId());
	gFBUIManager->DirtyRenderList(mHwndId);

	auto mani = gFBUIManager->GetTextManipulator();
	mani->AddListener(this);
	mani->SetText(&mTextw);
	

	auto propertyList = IsInPropertyList();
	if (propertyList)
	{
		ListItem* listItem = (ListItem*)mParent;
		propertyList->SetFocusRow(listItem->GetRowIndex());
	}

	KeyboardCursor::GetKeyboardCursor().SetScissorRegion(GetScissorRegion());
}

void TextField::OnFocusLost()
{
	gFBUIManager->DirtyRenderList(mHwndId);
	auto mani = gFBUIManager->GetTextManipulator();
	auto propertyList = IsInPropertyList();
	if (propertyList)
	{
		assert(mParent && mParent->GetType() == ComponentType::ListItem);
		ListItem* valueItem = (ListItem*)mParent;
		auto index = valueItem->GetRowIndex();
		propertyList->RemoveHighlight(index);
		auto uiEditor = gFBUIManager->GetUIEditor();
		IWinBase* editingUI = uiEditor->GetCurSelected();
		std::string key, value;
		propertyList->GetCurKeyValue(key, value);
		if (editingUI)
		{
			char buf[256] = { 0 };
			auto prop = UIProperty::IsUIProperty(key.c_str());
			if (prop != UIProperty::COUNT){
				auto got = editingUI->GetProperty(prop, buf, 256, false);
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
	mani->RemoveListener(this);
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
			gFBEnv->pRenderer->GetFont()->SetHeight(mTextSize);
			float width = gFBEnv->pRenderer->GetFont()->GetTextWidth(
				((const char*)mTextw.c_str()) + (start * 2),
				(end - start+1) * 2);
			float leftGap = gFBEnv->pRenderer->GetFont()->GetTextWidth(
				(const char*)mTextw.c_str(), start * 2);
			gFBEnv->pRenderer->GetFont()->SetBackToOrigHeight();
			KeyboardCursor::GetKeyboardCursor().SetSize(Vec2I((int)width, (int)mTextSize));
			KeyboardCursor::GetKeyboardCursor().SetPos(
				Vec2I(textStartWPos.x + (int)leftGap,
				textStartWPos.y - Round(mTextSize))
				);
		}
		else{
			gFBEnv->pRenderer->GetFont()->SetHeight(mTextSize);
			float aWidth = gFBEnv->pRenderer->GetFont()->GetTextWidth((const char*)AnsiToWide("A", 1), 2);
			Vec2I cursorSize(Round(aWidth), 2);
			gFBEnv->pRenderer->GetFont()->SetBackToOrigHeight();
			KeyboardCursor::GetKeyboardCursor().SetSize(Vec2I((int)1, (int)mTextSize));
			KeyboardCursor::GetKeyboardCursor().SetPos(
				Vec2I(textStartWPos.x, textStartWPos.y - Round(mTextSize))
				);
		}
	}
	else
	{
		
		gFBEnv->pRenderer->GetFont()->SetHeight(mTextSize);
			float aWidth = gFBEnv->pRenderer->GetFont()->GetTextWidth((const char*)AnsiToWide("A", 1), 2);
			Vec2I cursorSize(Round(aWidth), 2);
			KeyboardCursor::GetKeyboardCursor().SetSize(cursorSize);
			float width = gFBEnv->pRenderer->GetFont()->GetTextWidth(
				(const char*)mTextw.c_str(), cursorPos * 2);

		gFBEnv->pRenderer->GetFont()->SetBackToOrigHeight();

		Vec2I visualCursorPos(textStartWPos.x + Round(width),
			textStartWPos.y - WinBase::BOTTOM_GAP - 2);
		KeyboardCursor::GetKeyboardCursor().SetPos(visualCursorPos);

		// check region
		// right
		RECT region = GetScissorRegion();
		if (region.right - mTextGap.y < visualCursorPos.x + cursorSize.x){
			int offset = visualCursorPos.x + cursorSize.x - (region.right - mTextGap.y);
			mCursorOffset -= offset;
			mUIObject->SetTextOffsetForCursorMovement(Vec2I(mCursorOffset, 0));
			KeyboardCursor::GetKeyboardCursor().SetPos(Vec2I(visualCursorPos.x - offset, visualCursorPos.y));
		}
		else{
			if (region.left + mTextGap.x > visualCursorPos.x){
				int offset = region.left + mTextGap.x - visualCursorPos.x;
				mCursorOffset += offset;
				mUIObject->SetTextOffsetForCursorMovement(Vec2I(mCursorOffset, 0));
				KeyboardCursor::GetKeyboardCursor().SetPos(Vec2I(visualCursorPos.x + offset, visualCursorPos.y));
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
		ImageBox* T = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		T->SetHwndId(GetHwndId());
		mBorders.push_back(T);
		T->SetRender3D(mRender3D, GetRenderTargetSize());
		T->SetManualParent(this);
		T->ChangeSizeY(1);
		T->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");


		ImageBox* L = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		L->SetHwndId(GetHwndId());
		mBorders.push_back(L);
		L->SetRender3D(mRender3D, GetRenderTargetSize());
		L->SetManualParent(this);
		L->ChangeSizeX(1);
		L->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		ImageBox* R = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		R->SetHwndId(GetHwndId());
		mBorders.push_back(R);
		R->SetRender3D(mRender3D, GetRenderTargetSize());
		R->SetManualParent(this);
		R->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		R->ChangeSizeX(1);
		R->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");


		ImageBox* B = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		B->SetHwndId(GetHwndId());
		mBorders.push_back(B);
		B->SetRender3D(mRender3D, GetRenderTargetSize());
		B->SetManualParent(this);
		B->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		B->ChangeSizeY(1);
		B->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		ImageBox* LT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		LT->SetHwndId(GetHwndId());
		mBorders.push_back(LT);
		LT->SetRender3D(mRender3D, GetRenderTargetSize());
		LT->SetManualParent(this);
		LT->ChangeSize(Vec2I(1, 1));
		LT->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		ImageBox* RT = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RT->SetHwndId(GetHwndId());
		mBorders.push_back(RT);
		RT->SetRender3D(mRender3D, GetRenderTargetSize());
		RT->SetManualParent(this);
		RT->ChangeSize(Vec2I(1, 1));
		RT->SetAlign(ALIGNH::RIGHT, ALIGNV::TOP);
		RT->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		ImageBox* LB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);;
		LB->SetHwndId(GetHwndId());
		mBorders.push_back(LB);
		LB->SetRender3D(mRender3D, GetRenderTargetSize());
		LB->SetManualParent(this);
		LB->ChangeSize(Vec2I(1, 1));
		LB->SetAlign(ALIGNH::LEFT, ALIGNV::BOTTOM);
		LB->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		ImageBox* RB = (ImageBox*)gFBEnv->pUIManager->CreateComponent(ComponentType::ImageBox);
		RB->SetHwndId(GetHwndId());
		mBorders.push_back(RB);
		RB->SetRender3D(mRender3D, GetRenderTargetSize());
		RB->SetManualParent(this);
		RB->ChangeSize(Vec2I(1, 1));
		RB->SetAlign(ALIGNH::RIGHT, ALIGNV::BOTTOM);
		RB->SetTextureAtlasRegion("es/textures/ui.xml", "ThinBorder");

		RefreshBorder();
		RefreshScissorRects();
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	}
	else if (!use && !mBorders.empty())
	{
		for (auto ib : mBorders)
		{
			gFBEnv->pUIManager->DeleteComponent(ib);
		}
		mBorders.clear();
		gFBEnv->pUIManager->DirtyRenderList(GetHwndId());
	}
}

void TextField::OnPosChanged(bool anim)
{
	__super::OnPosChanged(anim);
	if (gFBUIManager->GetKeyboardFocusUI() == this)
	{
		OnCursorPosChanged(gFBUIManager->GetTextManipulator());
		KeyboardCursor::GetKeyboardCursor().SetScissorRegion(GetScissorRegion());
	}
}

void TextField::OnSizeChanged(){
	__super::OnSizeChanged();
	if (gFBUIManager->GetKeyboardFocusUI() == this)
	{
		OnCursorPosChanged(gFBUIManager->GetTextManipulator());
		KeyboardCursor::GetKeyboardCursor().SetScissorRegion(GetScissorRegion());
	}
}

PropertyList* TextField::IsInPropertyList() const
{
	if (mParent && mParent->GetType() == ComponentType::ListItem)
	{
		auto pp = mParent->GetParent();
		if (pp && pp->GetType() == ComponentType::PropertyList)
		{
			return (PropertyList*)pp;
		}
	}
	return 0;
}

ListBox* TextField::IsInListBox() const{
	if (mParent && mParent->GetType() == ComponentType::ListItem)
	{
		auto pp = mParent->GetParent();
		ListBox* listbox = dynamic_cast<ListBox*>(pp);
		return listbox;
	}
	return 0;
}

void TextField::SelectAll()
{
	gFBUIManager->GetTextManipulator()->SelectAll();
	TriggerRedraw();
}

void TextField::OnClicked(void* arg){
	assert(this == arg);
	if (!IsKeyboardFocused()){
		gFBUIManager->SetFocusUI(this);
	}
	auto mouse = gFBEnv->pEngine->GetMouse();
	long x, y;
	mouse->GetPos(x, y);
	Vec2I cursorPos(x, y);
	const auto& finalPos = GetFinalPos();
	cursorPos = cursorPos - finalPos;
	cursorPos.x -= mTextGap.x;
	cursorPos.x += mCursorOffset;

	auto font = gFBEnv->pRenderer->GetFont();
	if (font)
	{
		font->SetHeight(mTextSize);
		float length = 0.f;
		if (cursorPos.x >= (int)mTextWidth){
			gFBUIManager->GetTextManipulator()->SetCursorPos(mTextw.size());
		}
		else{
			for (int i = 0; i < (int)mTextw.size(); i++)
			{
				float halfLength = font->GetTextWidth((const char*)&mTextw[i], 2) *.5f;
				length += halfLength;
				if (cursorPos.x < length)
				{
					gFBUIManager->GetTextManipulator()->SetCursorPos(i);
					break;
				}
				length += halfLength;
			}
		}
		font->SetBackToOrigHeight();
	}
	mouse->Invalidate();

}
void TextField::OnDoubleClicked(void* arg){
	gFBUIManager->GetTextManipulator()->SelectAll();
}
}