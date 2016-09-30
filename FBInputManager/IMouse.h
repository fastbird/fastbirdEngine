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
#ifndef _fastbird_IMouse_header_included_
#define _fastbird_IMouse_header_included_

#include "IInputDevice.h"
#include "FBCommonHeaders/Types.h"
#include "MouseEvent.h"

namespace fb
{
	FB_DECLARE_SMART_PTR(IMouse);	
	class IMouse : public IInputDevice
	{
	public:
		enum MOUSE_BUTTON_FLAG
		{
			MOUSE_BUTTON_FLAG_LEFT_BUTTON_DOWN=0x0001,
			MOUSE_BUTTON_FLAG_LEFT_BUTTON_UP=0x0002,
			MOUSE_BUTTON_FLAG_RIGHT_BUTTON_DOWN=0x0004,
			MOUSE_BUTTON_FLAG_RIGHT_BUTTON_UP=0x0008,
			MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_DOWN=0x0010,
			MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_UP=0x0020,
			
			MOUSE_BUTTON_FLAG_BUTTON_1_DOWN=MOUSE_BUTTON_FLAG_LEFT_BUTTON_DOWN,
			MOUSE_BUTTON_FLAG_BUTTON_1_UP=MOUSE_BUTTON_FLAG_LEFT_BUTTON_UP,
			MOUSE_BUTTON_FLAG_BUTTON_2_DOWN=MOUSE_BUTTON_FLAG_RIGHT_BUTTON_DOWN,
			MOUSE_BUTTON_FLAG_BUTTON_2_UP=MOUSE_BUTTON_FLAG_RIGHT_BUTTON_UP,
			MOUSE_BUTTON_FLAG_BUTTON_3_DOWN=MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_DOWN,
			MOUSE_BUTTON_FLAG_BUTTON_3_UP=MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_UP,
			
			MOUSE_BUTTON_FLAG_BUTTON_4_DOWN=0x0040,
			MOUSE_BUTTON_FLAG_BUTTON_4_UP=0x0080,
			MOUSE_BUTTON_FLAG_BUTTON_5_DOWN=0x0100,
			MOUSE_BUTTON_FLAG_BUTTON_5_UP=0x0200,
			/*
			 * If usButtonFlags has RI_MOUSE_WHEEL, the wheel delta is stored in usButtonData.
			 * Take it as a signed value.
			 */
			MOUSE_BUTTON_FLAG_MOUSE_WHEEL=0x0400,
		};

		enum MOUSE_BUTTON
		{
			MOUSE_BUTTON_LEFT = 0x1,
			MOUSE_BUTTON_RIGHT = 0x2,
			MOUSE_BUTTON_MIDDLE = 0x4,
			MOUSE_BUTTON_4 = 0x8,
			MOUSE_BUTTON_5 = 0x10,
		};

		virtual void PushEvent(HWindow handle, const MouseEvent& mouseEvent, TIME_PRECISION gameTimeInSec) = 0;

		// Positions
		virtual void GetDpiDependentDeltaXY(long &x, long &y) const = 0;
		virtual Vec2ITuple GetDpiDependentDeltaXY() const = 0;
		// exactlay match to the cursor
		virtual void GetAbsDeltaXY(long &x, long &y) const = 0;
		virtual Vec2ITuple GetAbsDeltaXY() const = 0;
		virtual void GetPos(long &x, long &y) const = 0;
		virtual Vec2ITuple GetPos() const = 0;
		virtual void GetPrevPos(long &x, long &y) const = 0;
		// normalized pos(0.0~1.0)
		virtual void GetNPos(Real &x, Real &y) const = 0;
		virtual Vec2Tuple GetNPos() const = 0;
		virtual bool IsMoved() const = 0;
		virtual void LockMousePos(bool lock, void* key) = 0;
		virtual bool IsIn(int left, int top, int right, int bottom) = 0;
		virtual Real GetSensitivity() const = 0;
		virtual bool IsCursorVisible() const = 0;

		// Dragging
		virtual void GetDragStart(long &x, long &y) const = 0;
		virtual Vec2ITuple GetDragStartedPos() const = 0;
		virtual bool IsDragStartIn(int left, int top, int right, int bottom) const = 0;
		virtual bool IsDragStarted(int& outX, int& outY) const = 0;
		virtual bool IsDragEnded() const = 0;
		virtual void PopDragEvent() = 0;
		virtual bool IsRDragStarted(int& outX, int& outY) const = 0;
		virtual bool IsRDragEnded(int& outX, int& outY) const = 0;
		virtual void PopRDragEvent() = 0;

		// Buttons
		virtual bool IsLButtonDownPrev() const = 0;
		virtual bool IsLButtonDown(Real* time = 0) const = 0;
		virtual bool IsLButtonClicked() const = 0;
		virtual bool IsLButtonUp() const = 0;
		virtual bool IsLButtonDoubleClicked() const = 0;
		virtual bool IsLButtonPressed() const = 0;
		virtual bool IsRButtonDown(Real* time = 0) const = 0;		
		virtual bool IsRButtonDownPrev() const = 0;
		virtual bool IsRButtonClicked() const = 0;
		virtual bool IsRButtonPressed() const = 0;
		virtual bool IsMButtonDown() const = 0;
		virtual void ClearButton() = 0;
		virtual void NoClickOnce() = 0;
		virtual void ClearRightDown() = 0;
		virtual Real GetLButtonDownTime() const = 0;

		// Wheel
		virtual long GetWheel() const = 0;
		virtual void PopWheel() = 0;
		virtual void ClearWheel() = 0;		
		virtual unsigned long GetNumLinesWheelScroll() const = 0;
		
		
		virtual void OnKillFocus() = 0;
		virtual void OnSetFocus(HWindow hWnd) = 0;
		virtual void CursorToCenter() = 0;
		virtual void SetCursorPosition(int x, int y) = 0;
		virtual void AddHwndInterested(HWindow wnd) = 0;
		virtual void SetSensitivity(Real sens) = 0;
		virtual Real GetWheelSensitivity() const = 0;
		virtual void SetWheelSensitivity(Real sens) = 0;
	};
}

#endif //_fastbird_IMouse_header_included_