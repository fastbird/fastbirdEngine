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
		//_swapChain = gEnv->pRenderer->InitSwapChain((HWND)this->Handle.ToPointer(), this->Width, this->Height);
	}

	void My3DView::OnPaint()
	{
		/*static __int64 lastTick = gEnv->pTimer->GetTickCount();

		if (gEnv)
			this->Invalidate();

		__int64 currentTick = gEnv->pTimer->GetTickCount();
		__int64 elapsed = currentTick - lastTick;
		lastTick = currentTick;*/
	}
}