#pragma once
#include <UI/IUIAnimation.h>
namespace fastbird
{
	class UIAnimation : public IUIAnimation
	{
	public:
		UIAnimation();
		~UIAnimation();

		virtual void SetLength(float seconds);
		virtual void AddPos(float time, const Vec2& pos);
		virtual void Update(float deltaTime);
		virtual const Vec2 GetCurrentPos();

	private:
		VectorMap<float, Vec2> mKeyPos;
		float mLength;
		float mCurTime;
		bool mLoop;
		bool mEnd;
		Vec2 mCurPos;
	};
}