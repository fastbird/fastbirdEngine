#include "stdafx.h"
#include "InputDisplayer.h"
#include "Renderer.h"
#include "FBInputManager/InputManager.h"
#include "FBTimer/Timer.h"

using namespace fb;
InputDisplayerPtr InputDisplayer::Create() {
	auto p = InputDisplayerPtr(new InputDisplayer, [](InputDisplayer* obj) {delete obj; });
	InputManager::GetInstance().RegisterInputConsumer(p, IInputConsumer::Priority11_Console);
	return p;
}

class fb::InputDisplayer::Impl {
public:	

};

InputDisplayer::InputDisplayer()
	: mImpl(new Impl)
{
}

InputDisplayer::~InputDisplayer() {

}

void InputDisplayer::ConsumeInput(IInputInjectorPtr injector) {
	auto& r = Renderer::GetInstance();
	Vec2I pos(700, 700);
	char buf[512] = { 0 };
	int len = 0;
	// mouse
	if (injector->IsLButtonDown()) {
		const char* str = "[$imgMouseLeft$]";
		sprintf_s(buf, 511, str);
		len += strlen(str);
	}
	if (injector->IsRButtonDown()) {
		if (len == 0) {
			const char* str = "[$imgMouseRight$]";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+[$imgMouseRight$]";
			sprintf_s(buf + len, 511-len, str);
			len += strlen(str);
		}
	}
	if (injector->IsMButtonDown()) {
		if (len == 0) {
			const char* str = "[$imgMouseMiddle$]";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+[$imgMouseMiddle]";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	static float lastMouseMoved = gpTimer->GetTime();
	if (injector->IsMouseMoved() || 
		(gpTimer->GetTime() - lastMouseMoved)< 0.2f) {
		if (len == 0) {
			const char* str = "MouseMove";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+MouseMove";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
		if (injector->IsMouseMoved())
			lastMouseMoved = gpTimer->GetTime();
	}
	if (injector->IsKeyDown(VK_CONTROL)) {
		if (len == 0) {
			const char* str = "CTRL";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+CTRL";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_MENU)) {
		if (len == 0) {
			const char* str = "ALT";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+ALT";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_SHIFT)) {
		if (len == 0) {
			const char* str = "SHIFT";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+SHIFT";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_RETURN)) {
		if (len == 0) {
			const char* str = "ENTER";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+ENTER";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_LEFT)) {
		if (len == 0) {
			const char* str = "LEFT";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+LEFT";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_RIGHT)) {
		if (len == 0) {
			const char* str = "RIGHT";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+RIGHT";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_UP)) {
		if (len == 0) {
			const char* str = "UP";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+UP";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	if (injector->IsKeyDown(VK_DOWN)) {
		if (len == 0) {
			const char* str = "DOWN";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+DOWN";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
	}
	static float lastKeyTime[255] = { 0 };
	if (injector->IsKeyDown(VK_SPACE) || (gpTimer->GetTime() - lastKeyTime[VK_SPACE] < 0.2f)) {
		if (len == 0) {
			const char* str = "SPACE";
			sprintf_s(buf, 511, str);
			len += strlen(str);
		}
		else {
			const char* str = "+SPACE";
			sprintf_s(buf + len, 511 - len, str);
			len += strlen(str);
		}
		if (injector->IsKeyDown(VK_SPACE)) {
			lastKeyTime[VK_SPACE] = gpTimer->GetTime();
		}
	}
	auto& im = InputManager::GetInstance();
	for (unsigned short i = 0; i < 255; ++i) {
		if (i == VK_CONTROL || i == VK_MENU || i == VK_SHIFT || 
			i== VK_LCONTROL || i== VK_LMENU || i== VK_LSHIFT || i==VK_RETURN ||
			i==VK_LEFT || i == VK_RIGHT || i == VK_UP || i == VK_DOWN ||
			i==VK_SPACE)
			continue;
		if (injector->IsKeyDown(i) || (gpTimer->GetTime() - lastKeyTime[i] < 0.2f)) {
			if (len == 0) {				
				sprintf_s(buf, 511, "%c", i);
				len += 1;
			}
			else {				
				sprintf_s(buf + len, 511 - len, "+%c", i);
				len += 2;
			}
			if (injector->IsKeyDown(i))
				lastKeyTime[i] = gpTimer->GetTime();
		}
	}
	if (len > 0) {
		r.QueueDrawText(pos, buf, Color::White);
	}
}