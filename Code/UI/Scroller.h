#pragma once
#include <UI/WinBase.h>
namespace fastbird
{
class Scroller : public WinBase
{
public:
	Scroller();

	// IWinBase
	virtual ComponentType::Enum GetType() const { return ComponentType::Scroller; }
	virtual void GatherVisit(std::vector<IUIObject*>& v);
	virtual bool OnInputFromHandler(IMouse* mouse, IKeyboard* keyboard);

	virtual void OnStartUpdate(float elapsedTime);

	// Own
	void SetMaxOffset(const Vec2& maxOffset) { mMaxOffset = maxOffset;}
	const Vec2& GetMaxOffset() const { return mMaxOffset; }
	const Vec2& GetOffset() const { return mOffset; }
	void SetOwner(IWinBase* p) { mOwner = p; }

private:
	Vec2 mOffset;
	float mScrollAmount;
	Vec2 mMaxOffset;
	IWinBase* mOwner;
	float mMaxScrollSpeed;
	float mCurScrollSpeed;
	float mScrollAcc;
	float mCurOffset;
	float mDestOffset;
};
}