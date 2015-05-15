#pragma once

#include <UI/WinBase.h>
#include <Engine/ITextManipulatorListener.h>

namespace fastbird
{
class IUIObject;

class TextField : public WinBase, public ITextManipulatorListener
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

	// ITextManipulatorListener
	virtual void OnCursorPosChanged(TextManipulator* mani);
	virtual void OnTextChanged(TextManipulator* mani);
	
	

protected:
	const static float LEFT_GAP;
	virtual void SetUseBorder(bool use);
	virtual void OnPosChanged();

private:
	bool mPasswd;
};

}