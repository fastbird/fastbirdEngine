#pragma once

#include <UI/Container.h>
#include <UI/ICheckbox.h>
namespace fastbird
{
class IUIObject;
class ImageBox;
class StaticText;
class CheckBox : public Container, public ICheckbox
{
public:
	CheckBox();
	virtual ~CheckBox();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::CheckBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual void SetText(const wchar_t* szText);
	virtual void OnSizeChanged();

	// ICheckBox
	virtual void SetCheck(bool check);
	virtual bool GetCheck() const;

	// event
	void OnClicked(void* arg);

private:
	void UpdateImage();

private:
	ImageBox* mCheckImageBox;
	StaticText* mStaticText;
	bool mChecked;
};

}