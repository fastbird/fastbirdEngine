#pragma once
#include <CommonLib/LuaObject.h>
namespace fastbird
{
	class IWinBase;
	class IUIAnimation
	{
	public:
		virtual ~IUIAnimation() {}
		virtual IUIAnimation* Clone() const = 0;

		virtual void SetLength(float seconds) = 0;
		virtual float GetLength() const = 0;
		virtual void SetLoop(bool loop) = 0;
		virtual void AddPos(float time, const Vec2& pos) = 0;
		virtual void AddScale(float time, const Vec2& scale) = 0;
		virtual void AddTextColor(float time, const Color& color) = 0;
		virtual void AddBackColor(float time, const Color& color) = 0;
		virtual void AddMaterialColor(float time, const Color& color) = 0;
		virtual void AddAlpha(float time, float alpha) = 0;
		virtual void Update(float deltaTime) = 0;
		
		// returning relative pos(offset);
		virtual const Vec2& GetCurrentPos()  const = 0;
		virtual const Vec2& GetCurrentScale() const = 0;
		virtual const Color& GetCurrentTextColor() const = 0;
		virtual const Color& GetCurrentBackColor() const = 0;
		virtual float GetCurrentAlpha() const=0;
		virtual bool HasPosAnim() const = 0;
		virtual bool HasScaleAnim() const = 0;
		virtual bool HasTextColorAnim() const = 0;
		virtual bool HasBackColorAnim() const = 0;
		virtual bool HasAlphaAnim() const = 0;
		virtual void LoadFromXML(tinyxml2::XMLElement* elem) = 0;
		virtual void ParseLua(LuaObject& data) = 0;
		virtual bool IsActivated() const = 0;
		virtual void SetActivated(bool activate) = 0;
		virtual const char* GetName() const = 0;
		virtual void SetName(const char* name) = 0;
		virtual void SetTargetUI(IWinBase* target) = 0;
		virtual void ClearData() = 0;
	};
}