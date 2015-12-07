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

#pragma once
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "FBInputDevice.h"
#include "IInputInjector.h" // convenient include
namespace fb{
	FB_DECLARE_SMART_PTR(IRenderTargetObserver);
	FB_DECLARE_SMART_PTR(IMouse);
	FB_DECLARE_SMART_PTR(IKeyboard);
	FB_DECLARE_SMART_PTR(IInputConsumer);
	FB_DECLARE_SMART_PTR(IInputInjector);
	FB_DECLARE_SMART_PTR(InputManager);
	/** Handles user input. 
	You can register your objects which need to receive user input, 
	inherit the object from IInputConsumer and register it to 
	this manager.The object you registered should be Unregistered 
	before it destroyed.
	\ingroup FBInputManager
	*/
	class FB_DLL_INPUTMANAGER InputManager{
		FB_DECLARE_PIMPL_NON_COPYABLE(InputManager);
		InputManager();
		~InputManager();
	public:

		static InputManagerPtr Create();
		static bool HasInstance();
		static InputManager& GetInstance();

		void SetMainWindowHandle(HWindow window);
		HWindow GetMainWindowHandle() const;

		//-------------------------------------------------------------------
		// Manager
		//-------------------------------------------------------------------		
		void SetKeyboard(IKeyboardPtr keyboard);
		void SetMouse(IMousePtr mouse);

		/** Register an input consumer.
		You need unregister when the consumer is destroyed or does not 
		need to getinput information any more.
		\param consumer
		\param priority number one priority is handled first. 
		i.e. the lowest value is handled first. Check the default 
		priority at IInputConsumer::Priority
		*/
		void RegisterInputConsumer(IInputConsumerPtr consumer, int priority);
		void UnregisterInputConsumer(IInputConsumerPtr consumer, int priority);

		/** Will send input injector to every consumers
		*/
		void Update();

		void Invalidate(InputDevice::Enum type, bool includeButtonClicks = false);
		void InvalidTemporary(InputDevice::Enum type, bool invalidate);
		bool IsValid(InputDevice::Enum type) const;
		void EndFrame(TIME_PRECISION gameTimeInSecond);

		//-------------------------------------------------------------------
		// Common
		//-------------------------------------------------------------------
		void OnSetFocus(HWindow hWnd);
		void OnKillFocus();
		void AddHwndInterested(HWindow wnd);
		void SetInputInjector(IInputInjectorPtr injector);
		IInputInjectorPtr GetInputInjector() const;
		typedef std::vector<IRenderTargetObserverPtr> RenderTargetObservers;
		RenderTargetObservers GetRenderTargetObservers() const;

		//-------------------------------------------------------------------
		// Keyboard
		//-------------------------------------------------------------------
		void PushKeyEvent(HWindow hWnd, const KeyboardEvent& keyboardEvent);
		void PushChar(HWindow hWnd, unsigned keycode, TIME_PRECISION gameTimeInSec);
		void ClearBuffer();
		

		//-------------------------------------------------------------------
		// Mouse
		//-------------------------------------------------------------------
		void PushMouseEvent(HWindow handle, const MouseEvent& mouseEvent, TIME_PRECISION gameTimeInSec);
		Real GetSensitivity() const;
		void SetSensitivity(Real sens);
		Real GetWheelSensitivity() const;
		void SetWheelSensitivity(Real sens);
		void SetMousePosition(int x, int y);
		void MouseToCenter();		
	};
}