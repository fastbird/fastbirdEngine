#pragma once
#ifndef fastbird_Mouse_header_included_
#define fastbird_Mouse_header_included_

#include <Engine/IMouse.h>

namespace fastbird
{
	class Mouse : public IMouse
	{
	public:
		static void GetCurrentMousePos(HWND_ID hwndId, long& x, long& y, long& physicalX, long& physicalY);
		static void GetCurrentMousePos(HWND hwnd, long& x, long& y, long& physicalX, long& physicalY);
		static void SetCurrentMousePos(HWND_ID hwndId, long x, long y);
		static void SetCurrentMousePos(HWND hwnd, long x, long y);

		Mouse();

	protected:
		virtual void FinishSmartPtr();

	public:

		virtual void PushEvent(HWND handle, const MouseEvent& mouseEvent);

		virtual void EndFrame();
		virtual bool IsValid() const { return mValid && !mInvalidatedTemporary; }
		virtual void Invalidate(bool buttonClicked = false);
		virtual void InvalidTemporary(bool invalidate);
		virtual void GetHDDeltaXY(long &x, long &y) const;
		virtual void GetDeltaXY(long &x, long &y) const;
		virtual Vec2I GetDeltaXY() const;
		virtual void GetPos(long &x, long &y) const;
		virtual Vec2I GetPos() const;
		virtual void GetPrevPos(long &x, long &y) const;
		virtual void GetNPos(float &x, float &y) const;
		virtual Vec2 GetNPos() const;
		virtual void GetDragStart(long &x, long &y) const;
		virtual Vec2I GetDragStartedPos() const;
		virtual bool IsDragStartIn(const RECT& region) const;
		virtual bool IsDragStarted(Vec2I& outStartPos) const;
		virtual bool IsDragEnded() const;
		virtual void PopDragEvent();
		virtual void PopRDragEvent();

		virtual bool IsRDragStarted(Vec2I& outStartPos) const;
		virtual bool IsRDragEnded(Vec2I& outStartPos) const;

		virtual bool IsLButtonDownPrev() const;
		virtual bool IsLButtonDown(float* time = 0) const;
		virtual bool IsLButtonClicked() const;
		virtual bool IsLButtonDoubleClicked() const;
		virtual bool IsLButtonPressed() const;
		virtual bool IsRButtonDown(float* time = 0) const;
		virtual bool IsRButtonDownPrev() const;
		virtual bool IsRButtonClicked() const;
		virtual bool IsRButtonPressed() const;
		virtual bool IsMButtonDown() const;
		virtual bool IsMoved() const;

		virtual long GetWheel() const;
		virtual void PopWheel();
		virtual void ClearWheel();
		virtual void ClearButton();
		virtual unsigned long GetNumLinesWheelScroll() const;

		virtual void LockMousePos(bool lock, void* key);
		virtual void OnKillFocus();
		virtual void OnSetFocus(HWND hWnd);

		virtual const Ray3& GetWorldRay();

		virtual bool IsIn(const RECT& r);
		
		virtual void CursorToCenter();

		virtual void SetCursorPosition(const Vec2I& cursorPos);

		virtual void NoClickOnce();

		/*bool ButtonDown(MOUSE_BUTTON button) const;
		bool ButtonUp(MOUSE_BUTTON button) const;		
		void GetMouseCoordDelta(long& x, long& y) const;
		void GetMouseCoordAbsolute(long& x, long& y) const;
		long GetWheel() const;*/

	private:
		void ClearDrag();
		void ClearRDrag();

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

		long mPhysicalX;
		long mPhysicalY;

		float mNPosX;
		float mNPosY;

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
		float mLastLeftDownTime;
		float mLastRightDownTime;
		float mLastUpTime;
		Vec2I mLastClickPos;
		float mLastClickTime;

		Ray3 mWorldRay;
		bool mWorldRayCalculated;
		float mLastWheelPush;
		bool mDragStarted;
		bool mDragEnd;

		bool mRDragStarted;
		bool mRDragEnd;
		long mRDragStartX;
		long mRDragStartY;
		long mRDragEndX;
		long mRDragEndY;

		void* mLockMouseKey;
		bool mInvalidatedTemporary;
		bool mNoClickOnce;
	};
}

#endif