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
		virtual float GetLength() const { return mLength; }
		virtual void SetLoop(bool loop);
		virtual void AddPos(float time, const Vec2& pos);
		virtual void AddScale(float time, const Vec2& scale);
		virtual void AddTextColor(float time, const Color& color);
		virtual void AddBackColor(float time, const Color& color);
		virtual void AddMaterialColor(float time, const Color& color);
		virtual void Update(float deltaTime);
		virtual const Vec2& GetCurrentPos() const;
		virtual const Vec2& GetCurrentScale() const;
		virtual const Color& GetCurrentTextColor() const;
		virtual const Color& GetCurrentBackColor() const;
		virtual const Color& GetCurrentMaterialColor() const;
		virtual bool HasScaleAnim() const;
		virtual bool HasPosAnim() const;
		virtual bool HasTextColorAnim() const;
		virtual bool HasBackColorAnim() const;
		virtual bool HasMaterialColorAnim() const;
		virtual void LoadFromXML(tinyxml2::XMLElement* elem);
		virtual void ParseLua(LuaObject& data);
		virtual bool IsActivated() const { return mActivate; }
		virtual void SetActivated(bool activate);
		const char* GetName() const { return mName.c_str(); }
		virtual void SetName(const char* name);
		virtual void SetTargetUI(IWinBase* target) { mTargetUI = target; }
		virtual void ClearData();

		template <class T>
		T Animate(const VectorMap<float, T>& data, float curTime, float normTime)
		{
			assert(!data.empty());
			auto it = data.begin();
			auto itEnd = data.end();
			for (; it != itEnd; ++it)
			{
				if (curTime <= it->first)
				{
					if (it == data.begin())
					{
						return it->second;
					}
					else
					{
						auto prevIt = it - 1;
						float l = SmoothStep(prevIt->first, it->first, curTime);
						return Lerp(prevIt->second, it->second, l);
					}
				}
			}
			assert(0);
			return T();
		}

	private:
		bool mActivate;
		IWinBase* mTargetUI;
		int mID;
		std::string mName;
		VectorMap<float, Vec2> mKeyPos;
		VectorMap<float, Vec2> mKeyScale;
		VectorMap<float, Color> mKeyTextColor;
		VectorMap<float, Color> mKeyBackColor;
		VectorMap<float, Color> mKeyMaterialColor;
		float mLength;
		float mCurTime;
		bool mLoop;
		bool mEnd;
		Vec2 mCurPos;
		Vec2 mCurScale;
		Color mCurTextColor;
		Color mCurBackColor;
		Color mCurMaterialColor;
		
	};
}