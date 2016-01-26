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
#include "FBCommonHeaders/platform.h"
#include "IInputInjector.h"

namespace fb{
	FB_DECLARE_SMART_PTR(InputInjector);
	class InputInjector : public IInputInjector{
		FB_DECLARE_PIMPL_NON_COPYABLE(InputInjector);
		InputInjector();
		~InputInjector();

	public:
		static InputInjectorPtr Create();
		

		//-------------------------------------------------------------------
		// IInputInjector
		//-------------------------------------------------------------------
		bool IsValid(InputDevice::Enum type) const;
		void Invalidate(InputDevice::Enum type) const;
		void InvalidateClickTime() const;
		void InvalidTemporary(InputDevice::Enum type, bool invalidate);
		
		//-------------------------------------------------------------------
		// Keyboard
		//-------------------------------------------------------------------
		bool IsKeyDown(unsigned short keycode) const;
		bool IsKeyPressed(unsigned short keycode) const;
		bool IsKeyUp(unsigned short keycode) const;
		unsigned GetChar();
		void PopChar();
		void ClearBuffer();
		void ClearKeydown();

		//-------------------------------------------------------------------
		// Mouse
		//-------------------------------------------------------------------
		// Positions		
		void GetDeltaXY(long &x, long &y) const;
		Vec2ITuple GetDeltaXY() const;
		void GetAbsDeltaXY(long &x, long &y) const;
		Vec2ITuple GetAbsDeltaXY() const;
		void GetMousePos(long &x, long &y) const;
		Vec2ITuple GetMousePos() const;
		void GetMousePrevPos(long &x, long &y) const;
		void GetMouseNPos(Real &x, Real &y) const; // normalized pos(0.0~1.0)
		Vec2Tuple GetMouseNPos() const;
		bool IsMouseMoved() const;
		void LockMousePos(bool lock, void* key);
		bool IsMouseIn(int left, int top, int right, int bottom);
		Real GetSensitivity() const;

		// Dragging
		void GetDragStart(long &x, long &y) const;
		Vec2ITuple GetDragStartedPos() const;
		bool IsDragStartIn(int left, int top, int right, int bottom) const;
		bool IsDragStarted(int& outX, int& outY) const;
		bool IsDragEnded() const;
		void PopDragEvent();
		bool IsRDragStarted(int& outX, int& outY) const;
		bool IsRDragEnded(int& outX, int& outY) const;
		void PopRDragEvent();

		// Buttons
		bool IsLButtonDownPrev() const;
		bool IsLButtonDown(Real* time = 0) const;
		bool IsLButtonClicked() const;
		bool IsLButtonDoubleClicked() const;
		bool IsLButtonPressed() const;
		bool IsRButtonDown(Real* time = 0) const;
		bool IsRButtonDownPrev() const;
		bool IsRButtonClicked() const;
		bool IsRButtonPressed() const;
		bool IsMButtonDown() const;
		void ClearButton();
		void NoClickOnce();
		void ClearRightDown();
		Real GetLButtonDownTime() const;

		// Wheel		
		long GetWheel() const;
		void PopWheel();
		void ClearWheel();
		Real GetWheelSensitivity() const;
		unsigned long GetNumLinesWheelScroll() const;

		// Command
		void CursorToCenter();
		void SetCursorPosition(const Vec2ITuple& pos);

	private:
		friend class InputManager;
		void SetKeyboard(IKeyboardPtr keyboard);
		void SetMouse(IMousePtr mouse);
	};
}