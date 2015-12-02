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

#include "stdafx.h"
#include "InputManager.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "IInputConsumer.h"
#include "IInputInjector.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBTimer/Timer.h"
using namespace fb;

Timer* fb::gpTimer = 0;

class InputManager::Impl{
public:
	InputManagerWeakPtr mSelf;
	IKeyboardPtr mKeyboard;
	IMousePtr mMouse;
	IInputInjectorPtr mInjector;
	HWindow mMainWindowHandle;
	std::map<int, std::vector<IInputConsumerWeakPtr>> mConsumers;
	int mValid;

	Impl()
		: mValid(0)
		, mMainWindowHandle((HWindow)-1)
		, mKeyboard(IKeyboardPtr(new Keyboard))
		, mMouse(IMousePtr(new Mouse))
	{
		
		gpTimer = Timer::GetMainTimer().get();
		for (int i = 0; i < InputDevice::DeviceNum; ++i){
			mValid += 1 << i;
		}		
		SetInputInjector(InputInjector::Create());
	}

	void SetMainWindowHandle(HWindow window){
		if (mMainWindowHandle == window)
			return;

		mMainWindowHandle = window;	

#ifdef _PLATFORM_WINDOWS_
		const unsigned short HID_USAGE_PAGE_GENERIC = 0x01;
		const unsigned short HID_USAGE_GENERIC_MOUSE = 0x02;
		const unsigned short HID_USAGE_GENERIC_KEYBOARD = 0x06;

		RAWINPUTDEVICE Rid[2];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = 0;
		Rid[0].hwndTarget = 0;

		Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		Rid[1].dwFlags = 0;
		Rid[1].hwndTarget = 0;

		HRESULT hr = RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));
		if (FAILED(hr))
		{
			Logger::Log(FB_ERROR_LOG_ARG, "Registering Raw input devices failed!");
		}
#else
#endif
	}

	HWindow GetMainWindowHandle() const{
		return mMainWindowHandle;
	}

	void RegisterInputConsumer(IInputConsumerPtr consumer, int priority){
		auto& consumers = mConsumers[priority];
		if (!ValueExistsInVector(consumers, consumer)){
			consumers.push_back(consumer);			
		}
	}
	void UnregisterInputConsumer(IInputConsumerPtr consumer, int priority){
		auto& consumers = mConsumers[priority];
		DeleteValuesInVector(consumers, consumer);
	}

	void Update(InputManager* inputManager){
		mValid = 0;
		mValid = InputDevice::AllMask;		
		for (auto& it : mConsumers){
			for (auto weak = it.second.begin(); weak != it.second.end(); /**/){
				IteratingWeakContainer(it.second, weak, consumer);				
				consumer->ConsumeInput(mInjector);
				if (!(mValid & InputDevice::AllMask))
					return;				
			}
		}
	}

	void Invalidate(InputDevice::Enum type, bool includeButtonClicks){
		if (mValid & type)
			mValid -= mValid & type;
		if (type & InputDevice::Keyboard){
			if (mKeyboard){
				mKeyboard->Invalidate(includeButtonClicks);
			}
		}
		if (type & InputDevice::Mouse) {
			if (mMouse){
				mMouse->Invalidate(includeButtonClicks);
			}
		}		
	}

	void InvalidTemporary(InputDevice::Enum type, bool invalidate){
		switch (type){
		case InputDevice::Keyboard:
			if (mKeyboard){
				mKeyboard->InvalidTemporary(invalidate);
			}
			break;
		case InputDevice::Mouse:
			if (mMouse){
				mMouse->InvalidTemporary(invalidate);
			}
			break;
		default:
			assert(0 && "Not implemented");
		}
	}

	bool IsValid(InputDevice::Enum type) const{
		switch (type){
		case InputDevice::Keyboard:
			if (mKeyboard){
				return mKeyboard->IsValid();
			}
			break;
		case InputDevice::Mouse:
			if (mMouse){
				return mMouse->IsValid();
			}
			break;
		default:
			assert(0 && "Not implemented");
		}
		return false;
	}

	void EndFrame(TIME_PRECISION gameTimeInSecond){
		if (mKeyboard){
			mKeyboard->EndFrame(gameTimeInSecond);
		}
		if (mMouse){
			mMouse->EndFrame(gameTimeInSecond);
		}
	}

	void SetInputInjector(IInputInjectorPtr injector){
		mInjector = injector;
		mInjector->SetKeyboard(mKeyboard);
		mInjector->SetMouse(mMouse);
	}

	IInputInjectorPtr GetInputInjector() const{
		return mInjector;
	}

	RenderTargetObservers GetRenderTargetObservers() const{
		RenderTargetObservers ret;
		auto mouse = std::static_pointer_cast<Mouse>(mMouse);
		if (mouse)
			ret.push_back(std::dynamic_pointer_cast<IRenderTargetObserver>(mouse));

		return ret;
	}
};

//---------------------------------------------------------------------------
InputManagerWeakPtr sInputManager;
InputManagerPtr InputManager::Create(){
	if (sInputManager.expired()){
		auto inputManager = InputManagerPtr(new InputManager, [](InputManager* obj){ delete obj; });
		sInputManager = inputManager;
		inputManager->mImpl->mSelf = inputManager;
		return inputManager;
	}
	return sInputManager.lock();
}

bool InputManager::HasInstance(){
	return !sInputManager.expired();
}

InputManager& InputManager::GetInstance(){
	if (sInputManager.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "InputManager is deleted! The program will crash...");
	}
	return *sInputManager.lock();
}

InputManager::InputManager()
: mImpl(new Impl)
{
	
}

InputManager::~InputManager(){	
}

void InputManager::SetMainWindowHandle(HWindow window){
	mImpl->SetMainWindowHandle(window);
}

HWindow InputManager::GetMainWindowHandle() const{
	return mImpl->GetMainWindowHandle();
}

//-------------------------------------------------------------------
// Manager
//-------------------------------------------------------------------
void InputManager::SetKeyboard(IKeyboardPtr keyboard){
	mImpl->mKeyboard = keyboard;
}

void InputManager::SetMouse(IMousePtr mouse){
	mImpl->mMouse = mouse;
}

void InputManager::RegisterInputConsumer(IInputConsumerPtr consumer, int priority){
	mImpl->RegisterInputConsumer(consumer, priority);
}

void InputManager::UnregisterInputConsumer(IInputConsumerPtr consumer, int priority){
	mImpl->UnregisterInputConsumer(consumer, priority);
}

void InputManager::Update(){
	mImpl->Update(this);
}

void InputManager::Invalidate(InputDevice::Enum type, bool includeButtonClicks){
	mImpl->Invalidate(type, includeButtonClicks);
}

void InputManager::InvalidTemporary(InputDevice::Enum type, bool invalidate){
	mImpl->InvalidTemporary(type, invalidate);
}

bool InputManager::IsValid(InputDevice::Enum type) const{
	return mImpl->IsValid(type);
}

void InputManager::EndFrame(TIME_PRECISION gameTimeInSecond){
	mImpl->EndFrame(gameTimeInSecond);
}

//-------------------------------------------------------------------
// Common
//-------------------------------------------------------------------
void InputManager::OnSetFocus(HWindow hWnd){
	if (mImpl->mMouse){
		mImpl->mMouse->OnSetFocus(hWnd);
	}
}

void InputManager::OnKillFocus(){
	if (mImpl->mKeyboard){
		mImpl->mKeyboard->OnKillFocus();
	}
	if (mImpl->mMouse){
		mImpl->mMouse->OnKillFocus();
	}
}

void InputManager::AddHwndInterested(HWindow wnd){
	if (mImpl->mMouse){
		mImpl->mMouse->AddHwndInterested(wnd);
	}
}

void InputManager::SetInputInjector(IInputInjectorPtr injector){
	mImpl->SetInputInjector(injector);
}

IInputInjectorPtr InputManager::GetInputInjector() const{
	return mImpl->GetInputInjector();
}

InputManager::RenderTargetObservers InputManager::GetRenderTargetObservers() const{
	return mImpl->GetRenderTargetObservers();
}

//-------------------------------------------------------------------
// Keyboard
//-------------------------------------------------------------------
void InputManager::PushKeyEvent(HWindow hwnd, const KeyboardEvent& keyboardEvent){
	if (mImpl->mKeyboard){
		mImpl->mKeyboard->PushEvent(hwnd, keyboardEvent);
	}
}

void InputManager::PushChar(HWindow hwnd, unsigned keycode, TIME_PRECISION gameTimeInSec){
	if (mImpl->mKeyboard){
		mImpl->mKeyboard->PushChar(hwnd, keycode, gameTimeInSec);
	}
}

void InputManager::ClearBuffer(){
	if (mImpl->mKeyboard){
		mImpl->mKeyboard->ClearBuffer();
	}
}

//-------------------------------------------------------------------
// Mouse
//-------------------------------------------------------------------
void InputManager::PushMouseEvent(HWindow handle, const MouseEvent& mouseEvent, TIME_PRECISION timeInSec){
	if (mImpl->mMouse){
		mImpl->mMouse->PushEvent(handle, mouseEvent, timeInSec);
	}
}

void InputManager::MouseToCenter(){
	if (mImpl->mMouse){
		mImpl->mMouse->CursorToCenter();
	}
}

void InputManager::SetMousePosition(int x, int y){
	if (mImpl->mMouse){
		mImpl->mMouse->SetCursorPosition(x, y);
	}
}

Real InputManager::GetSensitivity() const{
	if (mImpl->mMouse){
		return mImpl->mMouse->GetSensitivity();
	}
	return 0.;
}

void InputManager::SetSensitivity(Real sens){
	if (mImpl->mMouse){
		mImpl->mMouse->SetSensitivity(sens);
	}
}

Real InputManager::GetWheelSensitivity() const{
	if (mImpl->mMouse){
		return mImpl->mMouse->GetWheelSensitivity();
	}
	return 0;
}

void InputManager::SetWheelSensitivity(Real sens){
	if (mImpl->mMouse){
		mImpl->mMouse->SetWheelSensitivity(sens);
	}
}