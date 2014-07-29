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

		virtual void PushEvent(const KeyboardEvent& keyboardEvent);
		virtual void PushChar(unsigned keycode);
		virtual void EndFrame();
		virtual bool IsValid() const;
		virtual void Invalidate();
		virtual bool IsKeyDown(unsigned short keycode) const;
		virtual bool IsKeyPressed(unsigned short keycode) const;
		virtual bool IsKeyUp(unsigned short keycode) const;
		virtual unsigned GetChar();
		virtual void OnKillFocus();

	private:
		bool IsPairedKey(unsigned short keycode) const;
		unsigned short TranslateIndex(const KeyboardEvent& keyboardEvent) const;

	private:
		bool mValid;
		bool mKeyDown[255];
		bool mKeyPressed[255]; // Down - up
		bool mKeyUp[255];
		unsigned mCurrentChar;
		float mKeyDownDuration[255];


	};
}

#endif //_fastbird_Keyboard_header_included_