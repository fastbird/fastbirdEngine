#pragma once

#include <UI/WinBase.h>

namespace fastbird
{
class IUIObject;

class TextField : public WinBase
{
public:
	TextField();
	virtual ~TextField();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::TextField; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);
	virtual void SetText(const wchar_t* szText);

	virtual void OnFocusLost();
	virtual void OnFocusGain();

	virtual void SetPasswd(bool passwd);


	// own
	
	

protected:
	const static float LEFT_GAP;
	virtual void OnPosChanged();
	virtual void OnSizeChanged();
	void MoveCursor(int move);

private:
	int mCursorPos;
	bool mPasswd;
};

}