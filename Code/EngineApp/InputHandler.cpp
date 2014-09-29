#include <EngineApp/StdAfx.h>
#include <EngineApp/InputHandler.h>
#include <EngineApp/CameraMan.h>
#include <EngineApp/UI.h>
using namespace fastbird;
extern CameraMan* gCameraMan;
extern UIs* gUI;
void InputHandler::OnInput(IMouse* pMouse, IKeyboard* pKeyboard)
{
	gUI->OnInputFromHandler(pMouse, pKeyboard);
	gCameraMan->OnInputFromHandler(pMouse, pKeyboard);

	if (pKeyboard->IsKeyPressed('L'))
	{
		gUI->SetVisible(true, UIS_FLEET_UI);
	}
}
