#include "Engine/StdAfx.h"
#include <Engine/Foundation/Mouse.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <CommonLib/Timer.h>
#include <CommonLib/Debug.h>

namespace fastbird
{
	void Mouse::GetCurrentMousePos(long& x, long& y)
	{
#ifdef _FBENGINE_FOR_WINDOWS_
		POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(gFBEnv->pEngine->GetWindowHandle(), &cursor);
		x = cursor.x;
		y = cursor.y;
#else
		assert(0);
#endif _FBENGINE_FOR_WINDOWS_
	}

	void Mouse::SetCurrentMousePos(long x, long y)
	{
#ifdef _FBENGINE_FOR_WINDOWS_
		POINT cursor;
		cursor.x = x;
		cursor.y = y;
		ClientToScreen(gFBEnv->pEngine->GetWindowHandle(), &cursor);
		SetCursorPos(cursor.x,cursor.y);
#else
		assert(0);
#endif _FBENGINE_FOR_WINDOWS_
	}



	Mouse::Mouse()
		: mLastDownPos(0, 0)
		, mLastDownTime(0)
		, mLastClickTime(0)
		, mLastClickPos(0, 0)
		, mLockMouse(false)
	{
		mLButtonDoubleClicked = false;
		mButtonsDown = 0;
		mButtonsDownPrev = 0;
		mButtonsClicked = 0;
		mButtonsDoubleClicked = 0;
		GetCurrentMousePos(mAbsX, mAbsY);
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

	void Mouse::PushEvent(const MouseEvent& mouseEvent)
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
			GetCurrentMousePos(mAbsX, mAbsY);
		mLastX = mouseEvent.lLastX;
		mLastY = mouseEvent.lLastY;
		mButtonsDownPrev = mButtonsDown;
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_LEFT_BUTTON_DOWN)
		{
			if ((mButtonsDown & MOUSE_BUTTON_LEFT)==0)
			{
				// drag
				mDragStartX = mAbsX;
				mDragStartY = mAbsY;
			}
			mButtonsDown |= MOUSE_BUTTON_LEFT;
			mButtonsPressed |= MOUSE_BUTTON_LEFT;

			mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_RIGHT_BUTTON_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_RIGHT;
			mButtonsPressed |= MOUSE_BUTTON_RIGHT;

			mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_MIDDLE_BUTTON_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_MIDDLE;
			mButtonsPressed |= MOUSE_BUTTON_MIDDLE;

			mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_4_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_4;
			mButtonsPressed |= MOUSE_BUTTON_4;

			mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_BUTTON_5_DOWN)
		{
			mButtonsDown |= MOUSE_BUTTON_5;
			mButtonsPressed |= MOUSE_BUTTON_5;

			mLastDownTime = gFBEnv->pTimer->GetTime();
			mLastDownPos.x = mAbsX;
			mLastDownPos.y = mAbsY;
		}


		bool mouseNotMoved = abs((mLastDownPos.x - mAbsX)) < 2 && abs((mLastDownPos.y - mAbsY)) < 2;
		float curTime = gFBEnv->pTimer->GetTime();
		float elapsedTime = (curTime - mLastDownTime);
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_LEFT_BUTTON_UP)
		{
			if (mButtonsDown & MOUSE_BUTTON_LEFT)
			{
				mDragEndX = mAbsX;
				mDragEndY = mAbsY;
				if (mDragStartX == mDragEndX && mDragStartY == mDragEndY)
				{
					ClearDrag();
				}
			}
			mButtonsDown &= ~MOUSE_BUTTON_LEFT;			

			float doubleClickElapsedTime = curTime - mLastClickTime;
			bool doubleClickMouseNotMoved = abs((mLastClickPos.x - mAbsX)) < 2 && abs((mLastClickPos.y - mAbsY)) < 2;
			if (doubleClickElapsedTime< mDoubleClickSpeed && doubleClickMouseNotMoved)
			{
				mLButtonDoubleClicked = true;
			}
			
			if (mouseNotMoved && !mLButtonDoubleClicked && elapsedTime < 0.15f)
			{
				mButtonsClicked |= MOUSE_BUTTON_LEFT;
				mLastClickTime = gFBEnv->pTimer->GetTime();
				mLastClickPos = Vec2I(mAbsX, mAbsY);
			}
		}
		if (mouseEvent.usButtonFlags & MOUSE_BUTTON_FLAG_RIGHT_BUTTON_UP)
		{
			mButtonsDown &= ~MOUSE_BUTTON_RIGHT;			
			if (mouseNotMoved && elapsedTime < 0.15f)
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
		}
	}

	void Mouse::EndFrame()
	{
		mButtonsClicked = 0;
		mButtonsPressed = 0;
		mLastX = 0;
		mLastY = 0;
		if (mLockMouse)
		{
			SetCurrentMousePos(mAbsXPrev, mAbsYPrev);
		}
		else
		{
			mAbsXPrev = mAbsX;
			mAbsYPrev = mAbsY;
		}
		while(!mWheel.empty())
			mWheel.pop();
		mValid = true;
		mLButtonDoubleClicked = false;
	}

	void Mouse::Invalidate()
	{
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

	void Mouse::GetPos(long &x, long &y) const
	{
		x = mAbsX;
		y = mAbsY;
	}

	void Mouse::GetPrevPos(long &x, long &y) const
	{
		x = mAbsXPrev;
		y = mAbsYPrev;
	}

	void Mouse::GetNPos(float &x, float &y) const
	{
		x = (float)mAbsX / (float)gFBEnv->pRenderer->GetWidth();
		y = (float)mAbsY / (float)gFBEnv->pRenderer->GetHeight();
	}

	void Mouse::GetDragStart(long &x, long &y) const
	{
		x = mAbsX;
		y = mAbsY;
	}

	Vec2 Mouse::GetNPos() const
	{
		return Vec2((float)mAbsX / (float)gFBEnv->pRenderer->GetWidth(),
			(float)mAbsY / (float)gFBEnv->pRenderer->GetHeight());
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
			*time = mLastDownTime;
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
			*time = mLastDownTime;
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

	long Mouse::GetWheel() const
	{
		if (mWheel.empty())
			return 0;
		return mWheel.top();
	}

	unsigned long Mouse::GetNumLinesWheelScroll() const
	{
		return mNumLinesWheelScroll;
	}

	void Mouse::LockMousePos(bool lock)
	{
		if (lock)
		{
			if (GetFocus() == gFBEnv->pEngine->GetWindowHandle())
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

	void Mouse::OnSetFocus()
	{
		GetCurrentMousePos(mAbsX, mAbsY);
		mAbsXPrev = mAbsX;
		mAbsYPrev = mAbsY;
		LockMousePos(false);
	}

	void Mouse::OnKillFocus()
	{		
		LockMousePos(false);
		mButtonsDown = 0;
		EndFrame();
	}
}