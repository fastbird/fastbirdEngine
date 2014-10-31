#pragma once

#include <UI/WinBase.h>

namespace fastbird
{
class IUIObject;

class StaticText : public WinBase
{
public:
	StaticText();
	virtual ~StaticText();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::StaticText; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	

protected:
	const static float LEFT_GAP;
	virtual void OnPosChanged();
	virtual void OnSizeChanged();

private:
	int mCursorPos;
	bool mPasswd;
};

}