#include <UI/StdAfx.h>
#include <UI/Scroller.h>

namespace fastbird
{

Scroller::Scroller()
	: mScrollAmount(0.001f)
	, mOffset(0, 0)
	, mMaxOffset(0, 0)
	, mOwner(0)
{
	mUIObject = IUIObject::CreateUIObject();
	mUIObject->mOwnerUI = this;
	mUIObject->mTypeString = ToString(GetType());
}

void Scroller::GatherVisit(std::vector<IUIObject*>& v)
{
	v.push_back(mUIObject);	
}

void Scroller::OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard)
{
	if (!mParent)
		return;

	if (mouse->IsValid() && mOwner->GetFocus(true))
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
}


}