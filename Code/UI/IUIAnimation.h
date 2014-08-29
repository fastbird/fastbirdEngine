#pragma once

namespace fastbird
{
	class IUIAnimation
	{
	public:
		virtual ~IUIAnimation() {}
		virtual void SetLength(float seconds) = 0;
		virtual void AddPos(float time, const Vec2& pos) = 0;
		virtual void Update(float deltaTime) = 0;
		// returning relative pos(offset);
		virtual const Vec2 GetCurrentPos() = 0;
	};
}