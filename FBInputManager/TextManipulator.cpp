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

#include "stdafx.h"
#include "TextManipulator.h"
#include "InputManager.h"
#include "FBSystemLib/ClipboardData.h"
#undef min
#undef max
namespace fb
{

FB_IMPLEMENT_STATIC_CREATE(TextManipulator);

TextManipulator::TextManipulator()
	:mHighlightStart(-1)
	, mCursorPos(0)
	, mText(0)
{

}

TextManipulator::~TextManipulator()
{

}

// Call AddListener first.
void TextManipulator::SetText(std::wstring* text)
{
	InputManager::GetInstance().ClearBuffer();
	mText = text;
	EndHighlighting();
	if (mText)
	{
		mCursorPos = mText->size();
		OnCursorPosChanged();
	}
}

void TextManipulator::ConsumeInput(IInputInjectorPtr injector)
{
	if (!mText)
		return;

	if (unsigned int chr = injector->GetChar())
	{
		auto mainWindowHandle = InputManager::GetInstance().GetMainWindowHandle();
		if (chr == 22) // Synchronous idle - ^V
		{
			injector->PopChar();	
			std::string data = GetClipboardDataAsString(mainWindowHandle);
			if (!data.empty())
			{
				if (IsHighlighting()){
					auto it = mText->begin() + mCursorPos;
					auto end = mText->begin() + mHighlightStart;
					if (mCursorPos > mHighlightStart)
					{
						std::swap(it, end);
					}
					mCursorPos = std::distance(mText->begin(), it);
					// delete selected string
					mText->erase(it, end);
					EndHighlighting();
				}
				std::wstring wclipData = AnsiToWide(data.c_str());
				mText->insert(mText->begin() + mCursorPos, wclipData.begin(), wclipData.end());				
				mCursorPos += data.size();
				OnCursorPosChanged();
				OnTextChanged();
			}
		}
		else if (chr == 3){ // ^C
			if (IsHighlighting()){
				auto it = mText->begin() + mCursorPos;
				auto end = mText->begin() + mHighlightStart;
				if (mCursorPos > mHighlightStart)
				{
					std::swap(it, end);
				}
				std::string data(it, end);
				SetClipboardStringData(mainWindowHandle, data.c_str());
			}
		}
		else{
			switch (chr)
			{
			case VK_BACK:
			{			
				if (IsHighlighting() && !mText->empty())
				{
					auto it = mText->begin() + mCursorPos;
					auto end = mText->begin() + mHighlightStart;
					if (mCursorPos > mHighlightStart)
					{
						std::swap(it, end);
					}
					mCursorPos = std::distance(mText->begin(), it);
					mText->erase(it, end);
					EndHighlighting();
				}
				else if (mCursorPos > 0 && !mText->empty())
				{
					mText->erase(mText->begin() + mCursorPos - 1);
					mCursorPos--;
				}
				OnCursorPosChanged();
				OnTextChanged();
			}
			break;
			default:
			{
				if (IsHighlighting())
				{
					auto it = mText->begin() + mCursorPos;
					auto end = mText->begin() + mHighlightStart;
					if (mCursorPos > mHighlightStart)
					{
						std::swap(it, end);
					}
					mCursorPos = std::distance(mText->begin(), it);
					// delete selected string
					mText->erase(it, end);
					EndHighlighting();
				}

				mText->insert(mText->begin() + mCursorPos, chr);
				mCursorPos++;
				OnCursorPosChanged();
				OnTextChanged();
			}
			}
		}

		injector->PopChar();
	}

	if (injector->IsKeyPressed(VK_HOME))
	{
		Highlighting(injector->IsKeyDown(VK_SHIFT));
		mCursorPos = 0;
		OnCursorPosChanged();
	}
	else if (injector->IsKeyPressed(VK_END))
	{
		Highlighting(injector->IsKeyDown(VK_SHIFT));
		mCursorPos = mText->size();
		OnCursorPosChanged();
	}
	else if (injector->IsKeyPressed(VK_DELETE))
	{
		if (!mText->empty())
		{
			if (IsHighlighting())
			{
				auto it = mText->begin() + mCursorPos;
				auto end = mText->begin() + mHighlightStart;
				if (mCursorPos > mHighlightStart)
				{
					std::swap(it, end);
				}
				mCursorPos = std::distance(mText->begin(), it);
				mText->erase(it, end);
				EndHighlighting();
			}
			else
			{
				mText->erase(mText->begin() + mCursorPos);
			}
		}
		OnCursorPosChanged();
		OnTextChanged();
	}
	else if (injector->IsKeyPressed(VK_LEFT))
	{
		Highlighting(injector->IsKeyDown(VK_SHIFT));
		if (mCursorPos>0)
		{
			mCursorPos--;
		}
		OnCursorPosChanged();
	}
	else if (injector->IsKeyPressed(VK_RIGHT))
	{
		Highlighting(injector->IsKeyDown(VK_SHIFT));
		if (mCursorPos < (int)mText->size())
		{
			mCursorPos++;
		}
		OnCursorPosChanged();
	}

	injector->Invalidate(InputDevice::Keyboard);
}

void TextManipulator::OnCursorPosChanged()
{
	auto& observers = mObservers_[ITextManipulatorObserver::Default];
	for (auto it = observers.begin(); it != observers.end(); /**/){
		auto observer = it->lock();
		if (!observer){
			it = observers.erase(it);
			continue;
		}
		++it;
		observer->OnCursorPosChanged(this);
	}
}

void TextManipulator::OnTextChanged()
{
	auto& observers = mObservers_[ITextManipulatorObserver::Default];
	for (auto it = observers.begin(); it != observers.end(); /**/){
		auto observer = it->lock();
		if (!observer){
			it = observers.erase(it);
			continue;
		}
		++it;
		observer->OnTextChanged(this);
	}
}

void TextManipulator::Highlighting(bool shiftkey)
{
	if (shiftkey)
	{
		if (!mText->empty())
		{
			if (!IsHighlighting())
			{
				StartHighlighting();
			}
		}
	}
	else
	{
		EndHighlighting();
	}
}

void TextManipulator::StartHighlighting()
{
	assert(!IsHighlighting());
	mHighlightStart = mCursorPos;
}

int TextManipulator::GetCursorPos() const
{
	return mCursorPos;
}

void TextManipulator::SetCursorPos(int pos)
{
	if (!mText)
	{
		assert(0);
		return;
	}
	mCursorPos = pos;
	mCursorPos = std::min(mCursorPos, (int)mText->size());
	OnCursorPosChanged();
}

void TextManipulator::SelectAll()
{
	if (!mText)
		return;
	if (mText->size() == 0)
	{
		EndHighlighting();
		return;
	}

	EndHighlighting();
	mCursorPos = 0;
	StartHighlighting();
	mCursorPos = mText->size();
	Highlighting(true);
	OnCursorPosChanged();

}

}