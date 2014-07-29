#pragma once
#include <Engine/IInputListener.h>

class InputHandler : public fastbird::IInputListener
{
public:
	virtual void OnInput(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);
};