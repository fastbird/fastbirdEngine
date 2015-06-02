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
	virtual bool GetProperty(UIProperty::Enum prop, char val[], unsigned bufsize, bool notDefaultOnly);
	virtual bool SetVisible(bool show);
	virtual void SetAnimScale(const Vec2& scale);
	virtual void RefreshScissorRects();
	
	// own
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	void MouseConsumer(void* arg){}

	virtual bool IsAlwaysOnTop() const{ return mAlwaysOnTop; }
	virtual bool GetCloseByEsc() const { return mCloseByEsc; }

	virtual void StartHighlight(float speed);
	virtual void StopHighlight();

	virtual void SetHwndId(HWND_ID hwndId);
	virtual const char* GetMsgTranslationUnit() const;

protected:
	virtual void OnSizeChanged();
	virtual void OnPosChanged(bool anim);
	void RefreshFrame();
	void OnTitlebarDrag(void *arg);

	ImageBox* CreateBackgroundImage();

private:
	Button* mTitlebar;
	std::string mTitlebarString;
	std::string mStrBackground;
	std::string mStrKeepRatio;
	std::vector<ImageBox*> mFrames;
	bool mUseFrame;
	ImageBox* mBackgroundImage;
	std::string mMsgTranslationUnit;
	bool mAlwaysOnTop;
	bool mCloseByEsc;
	bool mSyncWindowPos;
};

}