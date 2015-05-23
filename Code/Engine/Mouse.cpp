#include "Engine/StdAfx.h"
#include <Engine/Mouse.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/ICamera.h>
#include <CommonLib/Timer.h>
#include <CommonLib/Debug.h>

namespace fastbird
{
	void Mouse::GetCurrentMousePos(HWND_ID hwndId, long& x, long& y, long& physicalX, long& physicalY)
	{
		auto hWnd = gFBEnv->pEngine->GetWindowHandle(hwndId);
		GetCurrentMousePos(hWnd, x, y, physicalX, physicalY);

	}

	void Mouse::GetCurrentMousePos(HWND hWnd, long& x, long& y, long& physicalX, long& physicalY)
	{
#ifdef _FBENGINE_FOR_WINDOWS_
		POINT cursor;
		GetCursorPos(&cursor);
		physicalX = cursor.x;
		physicalY = cursor.y;
		assert(hWnd);
		ScreenToClient(hWnd, &cursor);
		x = cursor.x;
		y = cursor.y;
#else
		assert(0);
#endif _FBENGINE_FOR_WINDOWS_
	}

	void Mouse::SetCurrentMousePos(HWND_ID hwndId, long x, long y)
	{
		auto hWnd = gFBEnv->pEngine->GetWindowHandle(hwndId);
		SetCurrentMousePos(hWnd, x, y);
	}

	void Mouse::SetCurrentMousePos(HWND hWnd, long x, long y)
	{
#ifdef _FBENGINE_FOR_WINDOWS_
		POINT cursor;
		cursor.x = x;
		cursor.y = y;
		ClientToScreen(hWnd, &cursor);
		SetCursorPos(cursor.x, cursor.y);
#else
		assert(0);
#endif _FBENGINE_FOR_WINDOWS_
	}



	Mouse::Mouse()
		: mLastDownPos(0, 0)
		, mLastLeftDownTime(0)
		, mLastRightDownTime(0)
		, mLastClickTime(0)
		, mLastClickPos(0, 0)
		, mLockMouse(false)
		, mNPosX(0)
		, mNPosY(0)
		, mPhysicalX(0)
		, mPhysicalY(0)
		, mWorldRayCalculated(false)
		, mLastWheelPush(0)
		, mDragStarted(false)
		, mDragEnd(false)
	{
		mLButtonDoubleClicked = false;
		mButtonsDown = 0;
		mButtonsDownPrev = 0;
		mButtonsClicked = 0;
		mButtonsDoubleClicked = 0;
		GetCurrentMousePos(gFBEnv->pEngine->GetMainWndHandleId(), mAbsX, mAbsY, mPhysicalX, mPhysicalY);
		auto hWnd = gFBEnv->pEngine->GetMainWndHandle();
		Vec2I size = gFBEnv->pEngine->GetRequestedWndSize(hWnd);

		mNPosX = (float)mAbsX / (float)size.x;
		mNPosY = (float)mAbsY / (float)size.y;

		mAbsXPrev = mAbsX;
		mAbsYPrev = mAbsY;
		mLastX = 0;
		mLastY = 0;
		mValid = true;
		mNumLinesWheelScroll = 0;
		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &mNumLinesWheelScroll, 0);
		ClearDrag();
		mDoubleClickSpeed = (float)GetDoubleClickTime() / 1000.0f;
	}

	void Mouse::PushEvent(HWND handle, const MouseEvent& mouseEvent)
	{
		/*DebugOutput("usFlags = %x, usButtonFlags = %x, usButtonData = %x, ulRawButtons = %x, lLastX = %d, lLastY = %d, ulExtraInformation = %d",
						mouseEvent.usFlags,
						mouseEvent.usButtonFlags,
						mouseEvent.usButtonData,
						mouseEvent.ulRawButtons,
						mouseEvent.lLastX,
						mouseEvent.lLastY,
						mouseEvent.ulExtraInformation);*/

		if (!mLockMouse)
		{
			GetCurrentMousePos(handle, mAbsX, mAbsY, mPhysicalX, mPhysicalY);
			const auto& size = gFBEnv->pEngine->GetRequestedWndSize(handle);
			mNPosX = (float)mAbsX / (float)size.x;
			mNPosY = (float)mAbsY / (float)size.y;
			auto hwnd = WindowFromPoint({ mPhysicalX, mPhysicalY });
			if (hwnd != handle)
			{
				auto hwndId = gFBEnv->pEngine->GetWindowHandleId(hwnd);
				if (hwndId != INVALID_HWND_ID)
				{
					gFBEnv->pEngine->GetKeyboard()->ClearBuffer();
					SetForegroundWindow(hwnd);
					return;
				}
			}
		}

		mLastX = mouseEvent.lLastX;
		mLastY = mouseEvent.lLastY;


		mButtonsDownPrev = mButtonsDown;
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_LEFT_BUTTON_DOWN)
		{
			mDragEnd = false;
			if ((mButtonsDown & MOUSE_BUTTON_LEFT)==0)
			{
				// drag
				mDragStartX = mAbsX;
				mDragStartY = mAbsY;
			}
			mButtonsDown |= MOUSE_BUTTON_LEFT;
			mButtonsPressed |= MOUSE_BUTTON_LEFT;

			mLastLeftDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (IsMoved() && IsLButtonDown())
		{
			mDragStarted = true;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_RIGHT_BUTTON_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_RIGHT;
			mButtonsPressed |= MOUSE_BUTTON_RIGHT;

			mLastRightDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_MIDDLE;
			mButtonsPressed |= MOUSE_BUTTON_MIDDLE;

			//mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_4_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_4;
			mButtonsPressed |= MOUSE_BUTTON_4;

			//mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_5_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_5;
			mButtonsPressed |= MOUSE_BUTTON_5;

			//mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}


		bool mouseNotMoved = abs((mLastDownPos.x - mAbsX)) < 4 && abs((mLastDownPos.y - mAbsY)) < 4;
		float curTime = gFBEnv->pTimer->GetTime();
		float leftElapsedTime = (curTime - mLastLeftDownTime);
		float rightElapsedTime = (curTime - mLastRightDownTime);
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_LEFT_BUTTON_UP)
		{
			if (mButtonsDown & MOUSE_BUTTON_LEFT)
			{
				mDragEndX = mAbsX;
				mDragEndY = mAbsY;
				if (mDragStarted){
					mDragStarted = false;
				}
				mDragEnd = true;
				if (mDragStartX == mDragEndX && mDragStartY == mDragEndY)
				{
					ClearDrag();
				}
			}
			mButtonsDown &= ~MOUSE_BUTTON_LEFT;			

			float doubleClickElapsedTime = curTime - mLastClickTime;
			bool doubleClickMouseNotMoved = abs((mLastClickPos.x - mAbsX)) < 3 && abs((mLastClickPos.y - mAbsY)) < 3;
			if (doubleClickElapsedTime< mDoubleClickSpeed && doubleClickMouseNotMoved)
			{
				mLButtonDoubleClicked = true;
			}
			
			if (mouseNotMoved && !mLButtonDoubleClicked && leftElapsedTime < 0.25f)
			{
				mButtonsClicked |= MOUSE_BUTTON_LEFT;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_RIGHT_BUTTON_UP)
		{
			mButtonsDown &= ~MOUSE_BUTTON_RIGHT;			
			if (mouseNotMoved && rightElapsedTime < 0.15f)
			{
				mButtonsClicked |= MOUSE_BUTTON_RIGHT;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_UP)
		{
			mButtonsDown &= ~MOUSE_BUTTON_MIDDLE;
			if (mouseNotMoved)
			{
				mButtonsClicked |= MOUSE_BUTTON_MIDDLE;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_4_UP)
		{
			mButtonsDown &= ~MOUSE_BUTTON_4;
			if (mouseNotMoved)
			{
				mButtonsClicked |= MOUSE_BUTTON_4;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_5_UP)
		{
			mButtonsDown &= ~MOUSE_BUTTON_5;
			if (mouseNotMoved)
			{
				mButtonsClicked |= MOUSE_BUTTON_5;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}

		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_MOUSE_WHEEL)
		{
			mWheel.push((short)mouseEvent.usButtonData);
			mLastWheelPush = gpTimer->GetTime();
		}
	}

	void Mouse::EndFrame()
	{
		mWorldRayCalculated = false;
		mButtonsClicked = 0;
		mButtonsPressed = 0;
		mLastX = 0;
		mLastY = 0;
		if (mLockMouse)
		{
			SetCurrentMousePos(gFBEnv->pEngine->GetForegroundWindow(),
				mAbsXPrev, mAbsYPrev);
		}
		else
		{
			mAbsXPrev = mAbsX;
			mAbsYPrev = mAbsY;
		}
		mValid = true;
		mLButtonDoubleClicked = false;
		if (gpTimer->GetTime() - mLastWheelPush > 0.5f){
			while (!mWheel.empty()){
				mWheel.pop();
			}
		}
	}

	void Mouse::Invalidate(bool buttonClicked)
	{
		if (buttonClicked)
			mLastClickTime = 0;
		mValid = false;
	}

	//-------------------------------------------------------------------------
	void Mouse::GetHDDeltaXY(long &x, long &y) const
	{
		x = mLastX;
		y = mLastY;
	}

	void Mouse::GetDeltaXY(long &x, long &y) const
	{
		x = mAbsX - mAbsXPrev;
		y = mAbsY - mAbsYPrev;
	}

	Vec2I Mouse::GetDeltaXY() const
	{
		return Vec2I(mAbsX - mAbsXPrev, mAbsY - mAbsYPrev);
	}

	void Mouse::GetPos(long &x, long &y) const
	{
		x = mAbsX;
		y = mAbsY;
	}

	Vec2I Mouse::GetPos() const
	{
		return Vec2I(mAbsX, mAbsY);
	}

	void Mouse::GetPrevPos(long &x, long &y) const
	{
		x = mAbsXPrev;
		y = mAbsYPrev;
	}

	void Mouse::GetNPos(float &x, float &y) const
	{
		x = mNPosX;
		y = mNPosY;
	}

	void Mouse::GetDragStart(long &x, long &y) const
	{
		x = mDragStartX;
		y = mDragStartY;
	}

	Vec2 Mouse::GetNPos() const
	{
		auto hWnd = gFBEnv->pEngine->GetForegroundWindow();
		const auto& size = gFBEnv->pEngine->GetRequestedWndSize(hWnd);
		return Vec2((float)mAbsX / (float)size.x,
			(float)mAbsY / (float)size.y);
	}

	//-------------------------------------------------------------------------
	// LButton
	//-------------------------------------------------------------------------

	bool Mouse::IsLButtonDownPrev() const
	{
		return (mButtonsDownPrev&MOUSE_BUTTON_LEFT) != 0;
	}

	bool Mouse::IsLButtonDown(float* time) const
	{
		if (time)
			*time = mLastLeftDownTime;
		return (mButtonsDown & MOUSE_BUTTON_LEFT) !=0;
	}
	
	bool Mouse::IsLButtonClicked() const
	{
		return (mButtonsClicked & MOUSE_BUTTON_LEFT) != 0 && !mLButtonDoubleClicked;
	}
	bool Mouse::IsLButtonDoubleClicked() const
	{
		return mLButtonDoubleClicked;
	}

	bool Mouse::IsLButtonPressed() const
	{
		return (mButtonsPressed&MOUSE_BUTTON_LEFT) != 0;
	}

	bool Mouse::IsMButtonDown() const
	{
		return (mButtonsDown&MOUSE_BUTTON_MIDDLE) != 0;
	}

	//-------------------------------------------------------------------------
	// RButton
	//-------------------------------------------------------------------------
	bool Mouse::IsRButtonDown(float* time) const
	{
		if (time)
			*time = mLastRightDownTime;
		return (mButtonsDown & MOUSE_BUTTON_RIGHT) !=0;
	}

	bool Mouse::IsRButtonClicked() const
	{
		return (mButtonsClicked & MOUSE_BUTTON_RIGHT) != 0;
	}

	bool Mouse::IsRButtonPressed() const
	{
		return (mButtonsPressed&MOUSE_BUTTON_RIGHT) !=0;
	}

	bool Mouse::IsMoved() const
	{
		return mLastX!=0 || mLastY!=0;
	}

	void Mouse::ClearDrag()
	{
		mDragStartX = mDragEndX = -1;
		mDragStartY = mDragEndY = -1;
		mDragStarted = false;
		mDragEnd = false;
	}

	bool Mouse::IsDragStartIn(const RECT& region) const
	{
		if (mDragStartX < region.left ||
			mDragStartX > region.right ||
			mDragStartY < region.top ||
			mDragStartY > region.bottom)
			return false;

		return true;
	}

	bool Mouse::IsDragStarted(Vec2I& outStartPos) const{
		if (mDragStarted)
		{
			outStartPos = Vec2I(mDragStartX, mDragStartY);
		}
		return mDragStarted;
	}

	bool Mouse::IsDragEnded() const{
		return mDragEnd;
	}

	void Mouse::PopDragEvent(){
		mDragStarted = false;
		mDragEnd = false;
	}

	long Mouse::GetWheel() const
	{
		if (mWheel.empty())
			return 0;
		return mWheel.top();		
	}
	
	void Mouse::PopWheel(){
		mWheel.pop();
	}

	void Mouse::ClearWheel()
	{
		while (!mWheel.empty())
			mWheel.pop();
	}

	void Mouse::ClearButton()
	{
		mButtonsClicked = 0;
		mButtonsPressed = 0;
	}

	unsigned long Mouse::GetNumLinesWheelScroll() const
	{
		return mNumLinesWheelScroll;
	}

	void Mouse::LockMousePos(bool lock)
	{
		if (mLockMouse == lock)
			return;

		if (lock)
		{
			if (GetFocus() == gFBEnv->pEngine->GetMainWndHandle())
			{
				if (!mLockMouse)
				{
					mAbsXPrev = mAbsX;
					mAbsYPrev = mAbsY;
				}
				mLockMouse = true;
				int displayCounter = ShowCursor(false);
				while(displayCounter>=0)
				{
					displayCounter = ShowCursor(false);
				}
				return;
			}
		}

		int displayCounter = ShowCursor(true);
		while(displayCounter<0)
		{
			displayCounter = ShowCursor(true);
		}
		
		mLockMouse = false;
	}

	void Mouse::OnSetFocus(HWND hWnd)
	{
		GetCurrentMousePos(hWnd, mAbsX, mAbsY, mPhysicalX, mPhysicalY);
		const auto& size = gFBEnv->pEngine->GetRequestedWndSize(hWnd);
		mNPosX = (float)mAbsX / (float)size.x;
		mNPosY = (float)mAbsY / (float)size.y;
		mAbsXPrev = mAbsX;
		mAbsYPrev = mAbsY;

		LockMousePos(false);
		if (mDragStarted)
		{
			mDragStarted = false;
			mDragEnd = true;
		}
	}

	void Mouse::OnKillFocus()
	{		
		LockMousePos(false);
		mButtonsDown = 0;
		EndFrame();
	}

	const Ray3& Mouse::GetWorldRay()
	{
		if (!mWorldRayCalculated)
		{
			ICamera* pCam = gFBEnv->pRenderer->GetCamera();
			if (pCam)
			{
				mWorldRayCalculated = true;
				mWorldRay = pCam->ScreenPosToRay(mAbsX, mAbsY);
			}		
			else
			{
				Log("World ray cannot be calculated.");
			}
		}

		return mWorldRay;
	}
}