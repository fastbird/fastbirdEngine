#pragma once
#ifndef _fastbird_IKeyboard_header_included_
#define _fastbird_IKeyboard_header_included_

#include <Engine/IInputDevice.h>

namespace fastbird
{
	struct KeyboardEvent
	{
		unsigned short MakeCode;
		unsigned short Flags;
		unsigned short Reserved;
		unsigned short VKey;
		unsigned int   Message;
		unsigned long  ExtraInformation;

	};

	class IKeyboard : public IInputDevice
	{
	public:
		enum KEYBOARD_FLAG
		{
			KEYBOARD_FLAG_KEY_BREAK = 0x01,
			KEYBOARD_FLAG_KEY_E0 = 0x02,
			KEYBOARD_FLAG_KEY_E1 = 0x04,
			KEYBOARD_FLAG_KEY_MAKE = 0,
		};
		virtual ~IKeyboard(){}
		virtual void PushEvent(const KeyboardEvent& keyboardEvent) = 0;
		virtual void PushChar(unsigned keycode) = 0;
		virtual bool IsKeyDown(unsigned short keycode) const = 0;
		virtual bool IsKeyPressed(unsigned short keycode) const = 0;
		virtual bool IsKeyUp(unsigned short keycode) const = 0;
		virtual unsigned GetChar() = 0;
		virtual void OnKillFocus() = 0;
	};
}


#endif //_fastbird_IKeyboard_header_included_