#include <UI/StdAfx.h>
#include <UI/UIAnimation.h>
#include <UI/IWinBase.h>
#include <CommonLib/StringUtils.h>

namespace fastbird
{

//---------------------------------------------------------------------------
UIAnimation::UIAnimation()
: mLength(1.f)
, mCurTime(0)
, mLoop(true)
, mEnd(false)
, mCurPos(0, 0)
, mCurScale(1, 1)
, mActivate(false)
, mID(-1)
, mTargetUI(0)
, mCurAlpha(0)
{
}

//---------------------------------------------------------------------------
UIAnimation::~UIAnimation()
{

}

//---------------------------------------------------------------------------
void UIAnimation::SetLength(float seconds)
{
	mLength = seconds;
}

//---------------------------------------------------------------------------
void UIAnimation::SetLoop(bool loop)
{
	mLoop = loop;
}

//---------------------------------------------------------------------------
void UIAnimation::AddPos(float time, const Vec2& pos)
{
	if (mKeyPos.empty())
		mKeyPos[0.0f] = Vec2::ZERO;

	mKeyPos[time] = pos;
}

void UIAnimation::AddScale(float time, const Vec2& scale)
{
	if (mKeyScale.empty())
		mKeyScale[0.f] = Vec2(1, 1);

	mKeyScale[time] = scale;
}

void UIAnimation::AddTextColor(float time, const Color& color)
{
	if (mKeyTextColor.empty())
		mKeyTextColor[0.0f] = Color::White;

	mKeyTextColor[time] = color;
}

void UIAnimation::AddBackColor(float time, const Color& color)
{
	if (mKeyBackColor.empty())
		mKeyBackColor[0.0f] = Color::Black;

	mKeyBackColor[time] = color;
}

void UIAnimation::AddMaterialColor(float time, const Color& color)
{
	if (mKeyMaterialColor.empty())
		mKeyMaterialColor[0.0f] = Color::White;

	mKeyMaterialColor[time] = color;
}

void UIAnimation::AddAlpha(float time, float alpha)
{
	if (mKeyAlpha.empty())
		mKeyAlpha[0.f] = 1.f;
	mKeyAlpha[time] = alpha;
}

//---------------------------------------------------------------------------
void UIAnimation::Update(float deltaTime)
{
	if (!mActivate)
		return;
	if (mEnd)
		return;

	mCurTime += deltaTime;
	if (mCurTime > mLength)
	{
		if (mLoop)
		{
			mCurTime =  mCurTime - mLength;
		}
		else
		{
			mEnd = true;
			mCurTime = mLength;
			if (mTargetUI)
			{
				if (HasPosAnim())
				{
					mCurPos = (mKeyPos.end() - 1)->second;
				}
				if (HasScaleAnim())
				{
					mCurScale = (mKeyScale.end() - 1)->second;
				}
				// textcolor
				if (!mKeyTextColor.empty())
				{
					mCurTextColor = (mKeyTextColor.end() - 1)->second;
				}

				// backcolor
				if (!mKeyBackColor.empty())
				{
					mCurBackColor = (mKeyBackColor.end() - 1)->second;
				}

				if (!mKeyMaterialColor.empty())
				{
					mCurMaterialColor = (mKeyMaterialColor.end() - 1)->second;
				}

				if (!mKeyAlpha.empty())
				{
					mCurAlpha = (mKeyAlpha.end() - 1)->second;
				}
			}
			mActivate = false;
			return;
		}
	}

	float normTime = mCurTime / mLength;

	// TODO : Use template
	// pos
	if (!mKeyPos.empty())
	{
		mCurPos = Animate(mKeyPos, mCurTime, normTime);
	}
	// scale
	if (!mKeyScale.empty())
	{
		mCurScale = Animate(mKeyScale, mCurTime, normTime);
	}
	// textcolor
	if (!mKeyTextColor.empty())
	{
		mCurTextColor = Animate(mKeyTextColor, mCurTime, normTime);
	}

	// backcolor
	if (!mKeyBackColor.empty())
	{
		mCurBackColor = Animate(mKeyBackColor, mCurTime, normTime);
	}

	if (!mKeyMaterialColor.empty())
	{
		mCurMaterialColor = Animate(mKeyMaterialColor, mCurTime, normTime);
	}

	if (!mKeyAlpha.empty())
	{
		mCurAlpha = Animate(mKeyAlpha, mCurTime, normTime);
	}

	
}

//---------------------------------------------------------------------------
const Vec2& UIAnimation::GetCurrentPos() const
{
	return mCurPos;
}

const Vec2& UIAnimation::GetCurrentScale() const
{
	return mCurScale;
}

const Color& UIAnimation::GetCurrentTextColor() const
{
	return mCurTextColor;
}

const Color& UIAnimation::GetCurrentBackColor() const
{
	return mCurBackColor;
}

const Color& UIAnimation::GetCurrentMaterialColor() const
{
	return mCurMaterialColor;
}

float UIAnimation::GetCurrentAlpha() const
{
	return mCurAlpha;
}

//---------------------------------------------------------------------------
bool UIAnimation::HasScaleAnim() const
{
	return !mKeyScale.empty();
}
bool UIAnimation::HasPosAnim() const
{
	return !mKeyPos.empty();
}
bool UIAnimation::HasTextColorAnim() const
{
	return !mKeyTextColor.empty();
}
bool UIAnimation::HasBackColorAnim() const
{
	return !mKeyBackColor.empty();
}
bool UIAnimation::HasMaterialColorAnim() const
{
	return !mKeyMaterialColor.empty();
}

bool UIAnimation::HasAlphaAnim() const
{
	return !mKeyAlpha.empty();
}

void UIAnimation::LoadFromXML(tinyxml2::XMLElement* elem)
{
	if (!elem)
		return;

	const char* sz = elem->Attribute("id");
	if (sz)
		mID = StringConverter::parseInt(sz);

	sz = elem->Attribute("name");
	if (sz)
		mName = sz;

	sz = elem->Attribute("length");
	if (sz)
		mLength = StringConverter::parseReal(sz);

	sz = elem->Attribute("loop");
	if (sz)
		mLoop = StringConverter::parseBool(sz);

	{
		tinyxml2::XMLElement* pC = elem->FirstChildElement("TextColor");
		if (pC)
		{
			tinyxml2::XMLElement* k = pC->FirstChildElement("key");
			while (k)
			{
				float time = 0;
				Color color;
				sz = k->Attribute("time");
				if (sz)
					time = StringConverter::parseReal(sz);
				sz = k->Attribute("color");
				if (sz)
					color = StringConverter::parseColor(sz);
				AddTextColor(time, color);
				k = k->NextSiblingElement("key");
			}
		}
	}

	{
		tinyxml2::XMLElement* pC = elem->FirstChildElement("BackColor");
		if (pC)
		{
			tinyxml2::XMLElement* k = pC->FirstChildElement("key");
			while (k)
			{
				float time = 0;
				Color color;
				sz = k->Attribute("time");
				if (sz)
					time = StringConverter::parseReal(sz);
				sz = k->Attribute("color");
				if (sz)
					color = StringConverter::parseColor(sz);
				AddBackColor(time, color);
				k = k->NextSiblingElement("key");
			}
		}
	}

	{
		tinyxml2::XMLElement* pC = elem->FirstChildElement("MaterialColor");
		if (pC)
		{
			tinyxml2::XMLElement* k = pC->FirstChildElement("key");
			while (k)
			{
				float time = 0;
				Color color;
				sz = k->Attribute("time");
				if (sz)
					time = StringConverter::parseReal(sz);
				sz = k->Attribute("color");
				if (sz)
					color = StringConverter::parseColor(sz);
				AddMaterialColor(time, color);
				k = k->NextSiblingElement("key");
			}
		}
	}

	{
		tinyxml2::XMLElement* pC = elem->FirstChildElement("Pos");
		if (pC)
		{
			tinyxml2::XMLElement* k = pC->FirstChildElement("key");
			while (k)
			{
				float time = 0;
				Vec2 pos;
				sz = k->Attribute("time");
				if (sz)
					time = StringConverter::parseReal(sz);
				sz = k->Attribute("pos");
				if (sz)
					pos = StringConverter::parseVec2(sz);
				AddPos(time, pos);
				k = k->NextSiblingElement("key");
			}
		}
	}
}

void UIAnimation::ParseLua(LuaObject& data)
{
	if (!data.IsValid())
		return;

	mID = data.GetField("id").GetInt(mID);
	mName = data.GetField("name").GetString();
	mLength = data.GetField("length").GetFloat();
	mLoop = data.GetField("loop").GetBoolWithDef(mLoop);
	auto textColor = data.GetField("TextColor");
	if (textColor.IsValid())
	{
		auto keys = textColor.GetField("keys");
		assert(keys.IsValid());
		auto it = keys.GetSequenceIterator();
		LuaObject v;
		while (it.GetNext(v))
		{
			auto time = v.GetField("time").GetFloat();
			auto color = v.GetField("color").GetVec4();
			AddTextColor(time, color);
		}
	}

	auto backColor = data.GetField("BackColor");
	if (backColor.IsValid())
	{
		auto keys = backColor.GetField("keys");
		assert(keys.IsValid());
		auto it = keys.GetSequenceIterator();
		LuaObject v;
		while (it.GetNext(v))
		{
			auto time = v.GetField("time").GetFloat();
			auto color = v.GetField("color").GetVec4();
			AddBackColor(time, color);
		}
	}

	auto posAnim = data.GetField("Pos");
	if (posAnim.IsValid())
	{
		auto keys = posAnim.GetField("keys");
		assert(keys.IsValid());
		auto it = keys.GetSequenceIterator();
		LuaObject v;
		while (it.GetNext(v))
		{
			auto time = v.GetField("time").GetFloat();
			auto pos = v.GetField("pos").GetVec2();
			AddPos(time, pos);
		}
	}
}


void UIAnimation::SetActivated(bool activate)
{
	mActivate = activate;
	mEnd = false;
	mCurTime = 0.f;
	mCurPos = Vec2::ZERO;
	if (mTargetUI)
	{
		if (!mActivate)
		{
			mTargetUI->ClearAnimationResult();
			if (!mKeyBackColor.empty())
			{
				auto backColor = StringConverter::toString(mInitialBackColor);
				mTargetUI->SetProperty(UIProperty::BACK_COLOR, backColor.c_str());
			}
		}
		else if (mActivate)
		{
			if (!mKeyScale.empty())
			{
				mCurScale = mKeyScale.begin()->second;
				mTargetUI->SetAnimScale(mCurScale, mTargetUI->GetPivotWNPos());
			}

			if (!mKeyBackColor.empty())
			{
				mInitialBackColor = mTargetUI->GetBackColor();
			}
		}
	}
	
}

void UIAnimation::SetName(const char* name)
{
	assert(name && strlen(name)>0);
	mName = name;
}

void UIAnimation::ClearData()
{
	mKeyPos.clear();
	mKeyScale.clear();
	mKeyTextColor.clear();
	mKeyBackColor.clear();
	mKeyMaterialColor.clear();
	mKeyAlpha.clear();
}


IUIAnimation* UIAnimation::Clone() const
{
	UIAnimation* newAnim = FB_NEW(UIAnimation);
	newAnim->mID = mID;
	newAnim->mName = mName;
	newAnim->mKeyPos = mKeyPos;
	newAnim->mKeyScale = mKeyScale;
	newAnim->mKeyTextColor = mKeyTextColor;
	newAnim->mKeyBackColor = mKeyBackColor;
	newAnim->mKeyMaterialColor = mKeyMaterialColor;
	newAnim->mKeyAlpha = mKeyAlpha;
	newAnim->mLength = mLength;
	newAnim->mLoop = mLoop;
	newAnim->mCurPos = mCurPos;
	newAnim->mCurScale = mCurScale;
	newAnim->mCurTextColor = mCurTextColor;
	newAnim->mCurBackColor = mCurBackColor;
	newAnim->mCurMaterialColor = mCurMaterialColor;
	newAnim->mCurAlpha = mCurAlpha;
	return newAnim;
}
}