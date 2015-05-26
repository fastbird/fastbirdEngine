#include <Engine/StdAfx.h>
#include <Engine/TextManipulator.h>
#include <Engine/ITextManipulatorListener.h>
#include <CommonLib/ClipboardData.h>

namespace fastbird
{

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
	gFBEnv->pEngine->GetKeyboard()->ClearBuffer();
	mText = text;
	EndHighlighting();
	if (mText)
	{
		mCursorPos = mText->size();
		OnCursorPosChanged();
	}
}

void TextManipulator::OnInput(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mText)
		return;

	if (unsigned int chr = keyboard->GetChar())
	{
		if (chr == 22) // Synchronous idle - ^V
		{
			keyboard->PopChar();
			std::string data = GetClipboardDataAsString(gFBEnv->pEngine->GetMainWndHandle());
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
				SetClipboardStringData(gFBEnv->pEngine->GetMainWndHandle(), data.c_str());
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

		keyboard->PopChar();
	}

	if (keyboard->IsKeyPressed(VK_HOME))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		mCursorPos = 0;
		OnCursorPosChanged();
	}
	else if (keyboard->IsKeyPressed(VK_END))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		mCursorPos = mText->size();
		OnCursorPosChanged();
	}
	else if (keyboard->IsKeyPressed(VK_DELETE))
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
	else if (keyboard->IsKeyPressed(VK_LEFT))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		if (mCursorPos>0)
		{
			mCursorPos--;
		}
		OnCursorPosChanged();
	}
	else if (keyboard->IsKeyPressed(VK_RIGHT))
	{
		Highlighting(keyboard->IsKeyDown(VK_SHIFT));
		if (mCursorPos < (int)mText->size())
		{
			mCursorPos++;
		}
		OnCursorPosChanged();
	}

	keyboard->Invalidate();
}

void TextManipulator::AddListener(ITextManipulatorListener* l)
{
	if (!mListeners.empty())
	{
		Error(FB_DEFAULT_DEBUG_ARG, "You didn't remove the previous listener.");
		return;
	}
	mListeners.push_back(l);
}
void TextManipulator::RemoveListener(ITextManipulatorListener* l)
{
	DeleteValuesInVector(mListeners, l);
}

void TextManipulator::OnCursorPosChanged()
{
	for (auto l : mListeners)
	{
		l->OnCursorPosChanged(this);
	}
}

void TextManipulator::OnTextChanged()
{
	for (auto l : mListeners)
	{
		l->OnTextChanged(this);
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