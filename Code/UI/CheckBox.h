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
	virtual void OnCreated();
	virtual ComponentType::Enum GetType() const { return ComponentType::CheckBox; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);

	// ICheckBox
	virtual void SetCheck(bool check);
	virtual bool GetCheck() const;

	// event
	void OnClicked(void* arg);

	void OnMouseHover(void* arg);

private:
	void UpdateImage();

private:
	ImageBox* mCheckImageBox;
	bool mChecked;
};

}