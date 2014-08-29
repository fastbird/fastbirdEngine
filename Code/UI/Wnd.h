#pragma once
#include <Engine/IObject.h>
#include <UI/Container.h>
namespace fastbird
{
class IUIObject;

class Wnd : public Container
{
public:
	Wnd();
	virtual ~Wnd();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Window; }
	virtual void GatherVisit(std::vector<IUIObject*>& v) ;
	
	// own
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
};

}