#include <Engine/StdAfx.h>
#include <Engine/Misc/ColorRamp.h>
#include <CommonLib/Math/fbMath.h>

using namespace fastbird;

ColorRamp::ColorRamp()
{
}
ColorRamp::~ColorRamp()
{
}

void ColorRamp::InsertBar(float pos, const Color& color)
{
	mBars.push_back(Bar(pos, color));
	std::sort(mBars.begin(), mBars.end());
}

void ColorRamp::GenerateColorRampTextureData(int textureWidth)
{
	if (mBars.empty())
		return;

	mColors.clear();
	mColors.reserve(textureWidth);

	for (int i=0; i<textureWidth; i++)
	{
		float pos = (float)i / (textureWidth-1);
		// find the closest bar
		auto it = mBars.begin(), itEnd = mBars.end();
		BAR_VECTOR::iterator itNext = mBars.end();
		for (; it!=itEnd; it++)
		{
			if (it->position >=  pos)
			{
				itNext = it;
				break;
			}
		}
		if (itNext==mBars.end())
		{
			mColors.push_back(mBars.back().color);
			continue;
		}
		else
		{
			if (itNext==mBars.begin())
			{
				mColors.push_back(mBars[0].color);
				continue;
			}
			else
			{
				BAR_VECTOR::iterator itPrev = itNext-1;
				float posPrev = itPrev->position;
				float posNext = itNext->position;
				float len = posNext - posPrev;
				Color result = Lerp(itPrev->color, itNext->color, (pos - posPrev) / len);
				mColors.push_back(result);
			}
		}
	}
}

const Color& ColorRamp::operator[] (int idx) const
{
	assert(idx < (int)mColors.size());
	return mColors[idx];
}

bool ColorRamp::operator==(const ColorRamp& other) const
{
	if (mBars.size() != other.mBars.size())
		return false;

	size_t numBars = mBars.size();
	for (size_t i=0; i<numBars; i++)
	{
		if (!(mBars[i] == other.mBars[i]))
			return false;
	}
	

	return true;
}