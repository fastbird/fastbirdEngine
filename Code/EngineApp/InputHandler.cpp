#include "StdAfx.h"
#include "InputHandler.h"
#include "CameraMan.h"
using namespace fastbird;
extern CameraMan* gCameraMan;
void InputHandler::OnInput(IMouse* pMouse, IKeyboard* pKeyboard)
{
	gCameraMan->OnInputFromHandler(pMouse, pKeyboard);
}
