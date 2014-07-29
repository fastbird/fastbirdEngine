#pragma once

#include <UI/Wnd.h>
#include <UI/IRadioBox.h>
namespace fastbird
{
class IUIObject;
class ImageBox;
class StaticText;
class RadioBox : public Wnd, public IRadioBox
{
public:
	RadioBox();
	virtual ~RadioBox();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::RadioBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void SetText(const wchar_t* szText);

	// IRadioBox
	virtual void SetCheck(bool check);
	virtual bool GetCheck() const;
	virtual void SetGroupID(int id){ mGroupID = id; }
	virtual int GetGroupID() const { return mGroupID; }

	// event
	void OnClicked(void* arg);

	virtual void OnSizeChanged();
	virtual void OnPosChanged();

private:
	void UpdateImage();

private:
	ImageBox* mRadioImageBox;
	StaticText* mStaticText;
	bool mChecked;
	int mGroupID;
};

}