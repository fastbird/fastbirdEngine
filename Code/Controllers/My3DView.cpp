// Controllers.cpp : 기본 프로젝트 파일입니다.

#include "stdafx.h"
#include "My3DView.h"
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <CommonLib/Timer.h>
#include "EngineBridge.h"

namespace Controllers
{
	void My3DView::InitSwapChain()
	{
		//_swapChain = gFBEnv->pRenderer->InitSwapChain((HWND)this->Handle.ToPointer(), this->Width, this->Height);
	}

	void My3DView::OnPaint()
	{
		/*static __int64 lastTick = gFBEnv->pTimer->GetTickCount();

		if (gFBEnv)
			this->Invalidate();

		__int64 currentTick = gFBEnv->pTimer->GetTickCount();
		__int64 elapsed = currentTick - lastTick;
		lastTick = currentTick;*/
	}
}