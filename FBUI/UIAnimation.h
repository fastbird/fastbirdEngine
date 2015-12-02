/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#pragma once

namespace fb
{
	FB_DECLARE_SMART_PTR(WinBase);
	FB_DECLARE_SMART_PTR(UIAnimation);
	class UIAnimation
	{
	protected:
		UIAnimation();
		~UIAnimation();

	public:

		static UIAnimationPtr Create();		

		virtual UIAnimationPtr Clone() const;
		virtual void SetGlobalAnim(bool global);
		virtual bool IsGlobalAnim() const { return mGlobalAnim; }

		virtual void SetLength(float seconds);
		virtual float GetLength() const { return mLength; }
		virtual void SetLoop(bool loop);
		virtual void AddPos(float time, const Vec2& pos);
		virtual void AddScale(float time, const Vec2& scale);
		virtual void AddTextColor(float time, const Color& color);
		virtual void AddBackColor(float time, const Color& color);
		virtual void AddMaterialColor(float time, const Color& color);
		virtual void AddAlpha(float time, float alpha);
		virtual void Update(float deltaTime);
		virtual const Vec2& GetCurrentPos() const;
		virtual const Vec2& GetCurrentScale() const;
		virtual const Color& GetCurrentTextColor() const;
		virtual const Color& GetCurrentBackColor() const;
		virtual const Color& GetCurrentMaterialColor() const;
		virtual float GetCurrentAlpha() const;
		virtual bool HasScaleAnim() const;
		virtual bool HasPosAnim() const;
		virtual bool HasTextColorAnim() const;
		virtual bool HasBackColorAnim() const;
		virtual bool HasMaterialColorAnim() const;
		virtual bool HasAlphaAnim() const;
		virtual void LoadFromXML(tinyxml2::XMLElement* elem);
		virtual void Save(tinyxml2::XMLElement& elem);
		virtual void ParseLua(LuaObject& data);
		virtual bool IsActivated() const { return mActivate; }
		virtual void SetActivated(bool activate);
		const char* GetName() const { return mName.c_str(); }
		virtual void SetName(const char* name);
		virtual void SetTargetUI(WinBasePtr target) { mTargetUI = target; }
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
		WinBaseWeakPtr mTargetUI;
		int mID;
		std::string mName;
		VectorMap<float, Vec2> mKeyPos;
		VectorMap<float, Vec2> mKeyScale;
		VectorMap<float, Color> mKeyTextColor;
		VectorMap<float, Color> mKeyBackColor;
		VectorMap<float, Color> mKeyMaterialColor;
		VectorMap<float, float> mKeyAlpha;
		float mLength;
		float mCurTime;
		bool mLoop;
		bool mEnd;
		Vec2 mCurPos;
		Vec2 mCurScale;
		Color mCurTextColor;
		Color mCurBackColor;
		Color mInitialBackColor;
		Color mCurMaterialColor;
		float mCurAlpha;
		bool mGlobalAnim;
		
	};
}