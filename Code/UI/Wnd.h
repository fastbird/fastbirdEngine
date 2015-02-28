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
	virtual bool SetVisible(bool show);
	virtual void SetAnimScale(const Vec2& scale, const Vec2& povot);
	virtual void RefreshScissorRects();
	
	// own
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	void MouseConsumer(void* arg){}

	virtual bool IsAlwaysOnTop() const{ return mAlwaysOnTop; }
	virtual bool GetCloseByEsc() const { return mCloseByEsc; }

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
	bool mAlwaysOnTop;
	bool mCloseByEsc;
};

}