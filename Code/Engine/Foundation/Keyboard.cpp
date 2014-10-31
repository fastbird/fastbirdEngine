#include "Engine/StdAfx.h"
#include <Engine/Foundation/Keyboard.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <CommonLib/Debug.h>

namespace fastbird
{

	//--------------------------------------------------------------------------
	Keyboard::Keyboard()
		:mValid(true)
	{
		memset(mKeyDown, 0, sizeof(mKeyDown));
		memset(mKeyPressed, 0, sizeof(mKeyPressed));
		memset(mKeyDownDuration, 0, sizeof(mKeyDownDuration));
		memset(mKeyUp, 0, sizeof(mKeyUp));
	}

	//--------------------------------------------------------------------------
	void Keyboard::PushEvent(const KeyboardEvent& keyboardEvent)
	{
		/*DebugOutput("MakeCode = %d, Flags = %x, VKey = %d, Message = %d, ExtraInformation = %d",
			keyboardEvent.MakeCode, keyboardEvent.Flags, keyboardEvent.VKey, keyboardEvent.Message, keyboardEvent.ExtraInformation);*/

		if (keyboardEvent.Flags & 1) // key up
		{

			unsigned int key[] = {keyboardEvent.VKey, 0};
			if (IsPairedKey(keyboardEvent.VKey))
				key[1] = TranslateIndex(keyboardEvent);

			for (unsigned i=0; i<2; i++)
			{
				if (key[i]==0)
					continue;

				mKeyDown[key[i]] = false;
				mKeyUp[key[i]] = true;
			}
		}
		if (!(keyboardEvent.Flags & 1))// key down
		{
			unsigned int key[] = {keyboardEvent.VKey, 0};
			if (IsPairedKey(keyboardEvent.VKey))
				key[1] = TranslateIndex(keyboardEvent);

			for (unsigned i=0; i<2; i++)
			{
				if (key[i]==0)
					continue;

				mKeyPressed[key[i]] = true;
				mKeyDown[key[i]] = true;
			}
		}
	}

	void Keyboard::PushChar(unsigned keycode)
	{
		mCurrentChar = keycode;
	}

	//--------------------------------------------------------------------------
	void Keyboard::EndFrame()
	{
		mValid = true;
		memset(mKeyPressed, 0, sizeof(mKeyPressed));
		memset(mKeyUp, 0, sizeof(mKeyUp));
		mCurrentChar = 0;
	}

	//--------------------------------------------------------------------------
	void Keyboard::Invalidate()
	{
		mValid = false;
	}

	//--------------------------------------------------------------------------
	bool Keyboard::IsKeyDown(unsigned short keycode) const
	{
		return mKeyDown[keycode];
	}

	bool Keyboard::IsKeyPressed(unsigned short keycode) const
	{
		return mKeyPressed[keycode];
	}

	bool Keyboard::IsKeyUp(unsigned short keycode) const
	{
		return mKeyUp[keycode];
	}

	unsigned Keyboard::GetChar()
	{
		return mCurrentChar;
	}

	//--------------------------------------------------------------------------
	void Keyboard::OnKillFocus()
	{
		memset(mKeyDown, 0, sizeof(mKeyDown));
		memset(mKeyPressed, 0, sizeof(mKeyPressed));
	}

	bool Keyboard::IsPairedKey(unsigned short keycode) const
	{
		return keycode == VK_SHIFT || keycode == VK_CONTROL || keycode == VK_MENU;
	}

	//--------------------------------------------------------------------------
	unsigned short Keyboard::TranslateIndex(const KeyboardEvent& keyboardEvent) const
	{
		assert(IsPairedKey(keyboardEvent.VKey));
		
		int offset = 0;
		if (keyboardEvent.Flags & KEYBOARD_FLAG_KEY_E1)
		{
			offset = 1;
		}
		
		switch(keyboardEvent.VKey)
		{
		case VK_SHIFT:
			{
				return VK_LSHIFT + offset;
			}
			break;

		case VK_CONTROL:
			{
				return VK_LCONTROL + offset;
			}
			break;

		case VK_MENU:
			{
				return VK_LMENU + offset;
			}
			break;

		default:
			{
				assert(0 && "Not defined pair key!");
				return 0;
			}

		}


	}

}