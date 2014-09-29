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
		virtual Vec2 GetCurrentPos() = 0;
		virtual Color GetCurrentTextColor() = 0;
		virtual Color GetCurrentBackColor() = 0;
		virtual bool HasPosAnim() const = 0;
		virtual bool HasTextColorAnim() const = 0;
		virtual bool HasBackColorAnim() const = 0;
		virtual void LoadFromXML(tinyxml2::XMLElement* elem) = 0;
		virtual bool IsActivated() const = 0;
		virtual void SetActivated(bool activate) = 0;
		virtual const char* GetName() const = 0;
		virtual void SetName(const char* name) = 0;
	};
}