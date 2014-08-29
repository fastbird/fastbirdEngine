#pragma once
#ifndef fastbird_Mouse_header_included_
#define fastbird_Mouse_header_included_

#include <Engine/IMouse.h>

namespace fastbird
{
	class Mouse : public IMouse
	{
	public:
		static void GetCurrentMousePos(long& x, long& y);
		static void SetCurrentMousePos(long x, long y);

		Mouse();

		virtual void PushEvent(const MouseEvent& mouseEvent);

		virtual void EndFrame();
		virtual bool IsValid() const { return mValid; }
		virtual void Invalidate();
		virtual void GetHDDeltaXY(long &x, long &y) const;
		virtual void GetDeltaXY(long &x, long &y) const;
		virtual void GetPos(long &x, long &y) const;
		virtual void GetPrevPos(long &x, long &y) const;
		virtual void GetNPos(float &x, float &y) const;
		virtual Vec2 GetNPos() const;
		virtual void GetDragStart(long &x, long &y) const;
		virtual bool IsDragStartIn(const RECT& region) const;

		virtual bool IsLButtonDownPrev() const;
		virtual bool IsLButtonDown(float* time = 0) const;
		virtual bool IsLButtonClicked() const;
		virtual bool IsLButtonDoubleClicked() const;
		virtual bool IsLButtonPressed() const;
		virtual bool IsRButtonDown(float* time = 0) const;
		virtual bool IsRButtonClicked() const;
		virtual bool IsRButtonPressed() const;
		virtual bool IsMButtonDown() const;
		virtual bool IsMoved() const;

		virtual long GetWheel() const;
		virtual unsigned long GetNumLinesWheelScroll() const;

		virtual void LockMousePos(bool lock);
		virtual void OnKillFocus();
		virtual void OnSetFocus();
		
		/*bool ButtonDown(MOUSE_BUTTON button) const;
		bool ButtonUp(MOUSE_BUTTON button) const;		
		void GetMouseCoordDelta(long& x, long& y) const;
		void GetMouseCoordAbsolute(long& x, long& y) const;
		long GetWheel() const;*/

	private:
		void ClearDrag();

	private:
		int mButtonsDown;
		int mButtonsDownPrev;
		int mButtonsPressed;
		int mButtonsClicked;
		int mButtonsDoubleClicked;

		// this should be used for picking.
		long mAbsX;
		long mAbsXPrev;
		long mAbsY;
		long mAbsYPrev;

		// this is good to use camera rotation.
		long mLastX;
		long mLastY;

		long mDragStartX;
		long mDragStartY;
		long mDragEndX;
		long mDragEndY;

		std::stack<long> mWheel;

		bool mValid;
		unsigned long mNumLinesWheelScroll;

		double mDoubleClickSpeed;
		bool mLButtonDoubleClicked;
		bool mLockMouse;

		Vec2I mLastDownPos;
		float mLastDownTime;
		float mLastUpTime;
		Vec2I mLastClickPos;
		float mLastClickTime;
	};
}

#endif