#include <UI/StdAfx.h>
#include <UI/Scroller.h>
#include <UI/Container.h>

namespace fastbird
{

Scroller::Scroller()
	: mScrollAmount(0.001f)
	, mOffset(0, 0)
	, mMaxOffset(0, 0)
	, mOwner(0)
{
	mUIObject = IUIObject::CreateUIObject(false);
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ComponentType::ConvertToString(GetType());
}

void Scroller::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
}

bool Scroller::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mParent)
		return false;

	bool isIn = __super::OnInputFromHandler(mouse, keyboard);
	Vec2 npos = mouse->GetNPos();
	if (mouse->IsValid() && mParent->IsIn(npos))
	{
		long wheel = mouse->GetWheel();
		if (wheel)
		{			
			mOffset.y += wheel * mScrollAmount;
			mOffset.y = std::min(0.f, mOffset.y);
			mOffset.y = std::max(-mMaxOffset.y, mOffset.y);
			mOwner->Scrolled();
			mouse->Invalidate();
		}
	}
	return isIn;
}


}