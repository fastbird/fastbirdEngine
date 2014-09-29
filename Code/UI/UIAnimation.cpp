#include <UI/StdAfx.h>
#include <UI/UIAnimation.h>
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
, mActivate(false)
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
void UIAnimation::AddPos(float time, const Vec2& pos)
{
	if (mKeyPos.empty())
		mKeyPos[0.0f] = Vec2::ZERO;

	mKeyPos[time] = pos;
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
			mActivate = false;
		}
	}

	float normTime = mCurTime / mLength;
	// TODO : Use template
	// pos
	if (!mKeyPos.empty())
	{
		auto it = mKeyPos.begin();
		auto itEnd = mKeyPos.end();
		for (; it != itEnd; ++it)
		{
			if (normTime <= it->first)
			{
				if (it == mKeyPos.begin())
				{
					mCurPos = it->second;
				}
				auto prevIt = it - 1;
				float l = SmoothStep(prevIt->first, it->first, normTime);
				mCurPos = Lerp(prevIt->second, it->second, l);
				break;
			}
		}
	}
	// textcolor
	if (!mKeyTextColor.empty())
	{
		auto it = mKeyTextColor.begin();
		auto itEnd = mKeyTextColor.end();
		for (; it != itEnd; ++it)
		{
			if (normTime <= it->first)
			{
				if (it == mKeyTextColor.begin())
				{
					mCurTextColor = it->second;
				}
				auto prevIt = it - 1;
				float l = SmoothStep(prevIt->first, it->first, normTime);
				mCurTextColor = Lerp(prevIt->second, it->second, l);
				break;
			}
		}
	}

	// backcolor
	if (!mKeyBackColor.empty())
	{
		auto it = mKeyBackColor.begin();
		auto itEnd = mKeyBackColor.end();
		for (; it != itEnd; ++it)
		{
			if (normTime <= it->first)
			{
				if (it == mKeyBackColor.begin())
				{
					mCurBackColor = it->second;
				}
				auto prevIt = it - 1;
				float l = SmoothStep(prevIt->first, it->first, normTime);
				mCurBackColor = Lerp(prevIt->second, it->second, l);
				break;
			}
		}
	}

	
}

//---------------------------------------------------------------------------
Vec2 UIAnimation::GetCurrentPos()
{
	return mCurPos;
}

Color UIAnimation::GetCurrentTextColor()
{
	return mCurTextColor;
}

Color UIAnimation::GetCurrentBackColor()
{
	return mCurBackColor;
}

//---------------------------------------------------------------------------
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


void UIAnimation::SetActivated(bool activate)
{
	mActivate = activate;
	mEnd = false;
	mCurTime = 0.f;
}

void UIAnimation::SetName(const char* name)
{
	assert(name && strlen(name)>0);
	mName = name;
}

}