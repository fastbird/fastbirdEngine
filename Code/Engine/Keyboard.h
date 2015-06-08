#pragma once
#ifndef _fastbird_Keyboard_header_included_
#define _fastbird_Keyboard_header_included_

#include <Engine/IKeyboard.h>

namespace fastbird
{
	class Keyboard : public IKeyboard
	{
	public:	
		Keyboard();

	protected:
		virtual void FinishSmartPtr();

	public:
		virtual void PushEvent(HWND hWnd, const KeyboardEvent& keyboardEvent);
		virtual void PushChar(HWND hWnd, unsigned keycode);
		virtual void EndFrame();
		virtual bool IsValid() const { return mValid && !mInvalidatedTemporary; }
		virtual void Invalidate(bool buttonClicked = false);
		virtual void InvalidTemporary(bool invalidate);
		virtual bool IsKeyDown(unsigned short keycode) const;
		virtual bool IsKeyPressed(unsigned short keycode) const;
		virtual bool IsKeyUp(unsigned short keycode) const;
		virtual unsigned GetChar();
		virtual void PopChar();
		virtual void OnKillFocus();
		virtual void ClearBuffer();

	private:
		bool IsPairedKey(unsigned short keycode) const;
		unsigned short TranslateIndex(const KeyboardEvent& keyboardEvent) const;

	private:
		bool mValid;
		bool mKeyDown[255];
		bool mKeyPressed[255]; // Down - up
		bool mKeyUp[255];
		std::queue<unsigned> mCurrentChar;
		float mLastPushKeyTime;
		float mKeyDownDuration[255];
		bool mInvalidatedTemporary;


	};
}

#endif //_fastbird_Keyboard_header_included_