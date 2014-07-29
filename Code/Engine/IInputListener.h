#pragma once
#ifndef _fastbird_InputListener_header_included_
#define _fastbird_InputListener_header_included_

namespace fastbird
{
	class IMouse;
	class IKeyboard;

	class IInputListener
	{
	public:
		enum INPUT_LISTEN_CATEGORY
		{
			// handled first
			INPUT_LISTEN_PRIORITY_CONSOLE,
			INPUT_LISTEN_PRIORITY_UI, 
			INPUT_LISTEN_PRIORITY_INTERACT,
			INPUT_LISTEN_PRIORITY_CAMERA,			

			INPUT_LISTEN_PRIORITY_COUNT
		};
		IInputListener() : mFBInputListenerEnabled(true) {}
		virtual ~IInputListener(){}
		virtual void OnInput(IMouse* pMouse, IKeyboard* pKeyboard) = 0;
		virtual void EnableInputListener(bool enable) { mFBInputListenerEnabled = enable; }
		virtual bool IsEnabledInputLIstener() const { return mFBInputListenerEnabled; }

		INPUT_LISTEN_CATEGORY mFBInputListenerCategory;
		int mFBInputListenerPriority; // lower value processed first.
		bool mFBInputListenerEnabled;
	};
}

#endif //_fastbird_InputListener_header_included_