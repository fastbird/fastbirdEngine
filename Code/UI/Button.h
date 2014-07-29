#pragma once

#include <UI/WinBase.h>

namespace fastbird
{
class IUIObject;

class Button : public WinBase
{
public:
	Button();
	virtual ~Button();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Button; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	virtual bool SetProperty(Property prop, const char* val);

protected:
	const static float LEFT_GAP;
	const static float BOTTOM_GAP;

	void OnMouseIn(void* arg);
	void OnMouseHover(void* arg);
	void OnMouseOut(void* arg);
	void OnMouseDown(void* arg);

private:
	Color mBackColor;
	Color mBackColorOver;
	Color mBackColorDown;
};

}