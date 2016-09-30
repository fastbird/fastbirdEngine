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
#ifndef fastbird_Mouse_header_included_
#define fastbird_Mouse_header_included_

#include "IMouse.h"
#include "FBRenderer/IRenderTargetObserver.h"

namespace fb
{
	class Mouse : public IMouse, public IRenderTargetObserver
	{
		class MouseImpl;
		MouseImpl* mImpl;
		Mouse(const Mouse&) = delete;
		Mouse& Mouse::operator= (const Mouse&) = delete;

	public:
		Mouse();
		~Mouse();

	public:

		//-------------------------------------------------------------------
		// IInputDevice
		//-------------------------------------------------------------------
		virtual void EndFrame(TIME_PRECISION gameTimeInSec);
		virtual bool IsValid() const;
		virtual void Invalidate(bool buttonClicked = false);
		virtual void InvalidTemporary(bool invalidate);
		
		//-------------------------------------------------------------------
		// IMouse
		//-------------------------------------------------------------------
		virtual void PushEvent(HWindow handle, const MouseEvent& mouseEvent, TIME_PRECISION);
		
		// Positions		
		virtual void GetDpiDependentDeltaXY(long &x, long &y) const;
		virtual Vec2ITuple GetDpiDependentDeltaXY() const;
		// exactlay match to the cursor
		virtual void GetAbsDeltaXY(long &x, long &y) const;
		virtual Vec2ITuple GetAbsDeltaXY() const;
		virtual void GetPos(long &x, long &y) const;
		virtual Vec2ITuple GetPos() const;
		virtual void GetPrevPos(long &x, long &y) const;
		// normalized pos(0.0~1.0)
		virtual void GetNPos(Real &x, Real &y) const;
		virtual Vec2Tuple GetNPos() const;
		virtual bool IsMoved() const;
		virtual void LockMousePos(bool lock, void* key);
		virtual bool IsIn(int left, int top, int right, int bottom);
		virtual Real GetSensitivity() const;
		virtual bool IsCursorVisible() const OVERRIDE;

		// Dragging
		virtual void GetDragStart(long &x, long &y) const;
		virtual Vec2ITuple GetDragStartedPos() const;
		virtual bool IsDragStartIn(int left, int top, int right, int bottom) const;
		virtual bool IsDragStarted(int& outX, int& outY) const;
		virtual bool IsDragEnded() const;
		virtual void PopDragEvent();
		virtual bool IsRDragStarted(int& outX, int& outY) const;
		virtual bool IsRDragEnded(int& outX, int& outY) const;
		virtual void PopRDragEvent();

		// Buttons
		virtual bool IsLButtonDownPrev() const;
		virtual bool IsLButtonDown(Real* time = 0) const;
		virtual bool IsLButtonClicked() const;
		virtual bool IsLButtonUp() const OVERRIDE;
		virtual bool IsLButtonDoubleClicked() const;
		virtual bool IsLButtonPressed() const;
		virtual bool IsRButtonDown(Real* time = 0) const;
		virtual bool IsRButtonDownPrev() const;
		virtual bool IsRButtonClicked() const;
		virtual bool IsRButtonPressed() const;
		virtual bool IsMButtonDown() const;
		virtual void ClearButton();
		virtual void NoClickOnce();
		virtual void ClearRightDown();
		virtual Real GetLButtonDownTime() const;

		// Wheel
		virtual long GetWheel() const;
		virtual void PopWheel();
		virtual void ClearWheel();
		virtual unsigned long GetNumLinesWheelScroll() const;


		virtual void OnKillFocus();
		virtual void OnSetFocus(HWindow hWnd);
		virtual void CursorToCenter();
		virtual void SetCursorPosition(int x, int y);
		virtual void AddHwndInterested(HWindow wnd);
		virtual void SetSensitivity(Real sens);
		virtual Real GetWheelSensitivity() const;
		virtual void SetWheelSensitivity(Real sens);

		//-------------------------------------------------------------------
		// IRenderTargetObserver
		//-------------------------------------------------------------------
		virtual void OnRenderTargetSizeChanged(int x, int y, HWindow associatedWindow);
	};
}

#endif