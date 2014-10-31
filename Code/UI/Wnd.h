#pragma once
#include <Engine/IObject.h>
#include <UI/Container.h>
namespace fastbird
{
class IUIObject;
class Button;
class ImageBox;

class Wnd : public Container
{
public:
	Wnd();
	virtual ~Wnd();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Window; }
	virtual void GatherVisit(std::vector<IUIObject*>& v) ;
	virtual bool SetProperty(UIProperty::Enum prop, const char* val);
	virtual void SetVisible(bool show);
	
	// own
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	void MouseConsumer(void* arg){}

protected:
	virtual void OnSizeChanged();
	virtual void OnPosChanged();
	void RefreshFrame();
	void OnTitlebarDrag(void *arg);

private:
	Button* mTitlebar;
	std::vector<ImageBox*> mFrames;
	bool mUseFrame;
	ImageBox* mBackgroundImage;
};

}