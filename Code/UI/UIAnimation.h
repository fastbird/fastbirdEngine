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
		virtual void AddTextColor(float time, const Color& color);
		virtual void AddBackColor(float time, const Color& color);
		virtual void Update(float deltaTime);
		virtual Vec2 GetCurrentPos();
		virtual Color GetCurrentTextColor();
		virtual Color GetCurrentBackColor();
		virtual bool HasPosAnim() const;
		virtual bool HasTextColorAnim() const;
		virtual bool HasBackColorAnim() const;
		virtual void LoadFromXML(tinyxml2::XMLElement* elem);
		virtual bool IsActivated() const { return mActivate; }
		virtual void SetActivated(bool activate);
		const char* GetName() const { return mName.c_str(); }
		virtual void SetName(const char* name);
		

	private:
		bool mActivate;
		int mID;
		std::string mName;
		VectorMap<float, Vec2> mKeyPos;
		VectorMap<float, Color> mKeyTextColor;
		VectorMap<float, Color> mKeyBackColor;
		float mLength;
		float mCurTime;
		bool mLoop;
		bool mEnd;
		Vec2 mCurPos;
		Color mCurTextColor;
		Color mCurBackColor;
		
	};
}