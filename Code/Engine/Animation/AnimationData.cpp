#include <Engine/StdAfx.h>
#include <Engine/Animation/AnimationData.h>

namespace fastbird
{
	void AnimationData::AddPosition(float time, float v, PosComp comp)
	{
		auto itFind = mPos.Find(time);
		if (itFind == mPos.end())
		{
			auto prevPos = FindPos(time);
			Vec3 def = Vec3::ZERO;
			if (prevPos)
				def = *prevPos;
			mPos[time] = def;
		}
		switch (comp)
		{
		case PosComp::X:
			mPos[time].x = v;
			break;
		case PosComp::Y:
			mPos[time].y = v;
			break;
		case PosComp::Z:
			mPos[time].z = v;
			break;
		}
	}
	void AnimationData::AddScale(float time, float v, PosComp comp)
	{
		auto itFind = mScale.Find(time);
		if (itFind == mScale.end())
		{
			auto prev = FindScale(time);
			Vec3 def = Vec3::ZERO;
			if (prev)
				def = *prev;
			mScale[time] = def;
		}

		switch (comp)
		{
		case PosComp::X:
			mScale[time].x = v;
			break;
		case PosComp::Y:
			mScale[time].y = v;
			break;
		case PosComp::Z:
			mScale[time].z = v;
			break;
		}
	}
	void AnimationData::AddRotEuler(float time, float v, PosComp comp)
	{
		auto itFind = mEuler.Find(time);
		if (itFind == mEuler.end())
		{
			auto prev = FindRotEuler(time);
			Vec3 def = Vec3::ZERO;
			if (prev)
				def = *prev;
			mEuler[time] = def;
		}

		switch (comp)
		{
		case PosComp::X:
			mEuler[time].x = v;
			break;
		case PosComp::Y:
			mEuler[time].y = v;
			break;
		case PosComp::Z:
		{
						   mEuler[time].z = v;
						   break;
		}
		}
	}

	void AnimationData::GenerateQuatFromEuler()
	{
		for (const auto& it : mEuler)
		{
			mRot[it.first] = Quat(it.second);
		}
		mEuler.clear();
	}

	bool AnimationData::ParseAction(const char* filename)
	{
		GenerateQuatFromEuler();

		tinyxml2::XMLDocument doc;
		int errId = doc.LoadFile(filename);
		if (errId)
		{
			Error("AnimationData::ParseAction err : %s", doc.GetErrorStr1());
			Error("	%s", doc.GetErrorStr2());
			return false;
		}
		auto actions = doc.FirstChildElement("Actions");
		if (!actions)
		{
			Error("Cannot find Actions tag!");
			return false;
		}

		auto action = actions->FirstChildElement("Action");
		while (action)
		{
			const char* sz = action->Attribute("name");
			if (!sz)
			{
				Error("name attribute is not found in the action tag!");
				return false;
			}
			auto& newAction = mActions[sz];
			newAction.mName = sz;
			sz = action->Attribute("start");
			unsigned startFrame = 0;
			unsigned endFrame = 0;
			if (sz)
			{
				startFrame = StringConverter::parseUnsignedInt(sz);
			}

			sz = action->Attribute("end");
			if (sz)
			{
				endFrame = StringConverter::parseUnsignedInt(sz);
			}

			newAction.mStartTime = startFrame / 24.0f;
			newAction.mEndTime = endFrame / 24.0f;
			newAction.mLength = newAction.mEndTime - newAction.mStartTime;
			newAction.mPosStartEnd[0] = FindPos(newAction.mStartTime);
			newAction.mPosStartEnd[1] = FindPos(newAction.mEndTime);
			newAction.mRotStartEnd[0] = FindRot(newAction.mStartTime);
			newAction.mRotStartEnd[1] = FindRot(newAction.mEndTime);

			sz = action->Attribute("loop");
			if (sz)
				newAction.mLoop = StringConverter::parseBool(sz);
			action = action->NextSiblingElement("Action");
		}
		return true;
	}

	const Vec3* AnimationData::FindPos(float time)
	{
		const Vec3* mostClose = 0;
		for (const auto& it : mPos)
		{
			if (IsEqual(it.first, time, 0.01f))
			{
				return &it.second;
			}
			else
			{
				if (it.first < time)
					mostClose = &it.second;
			}
		}
		return mostClose;
	}

	const Vec3* AnimationData::FindScale(float time)
	{
		const Vec3* mostClose = 0;
		for (const auto& it : mScale)
		{
			if (IsEqual(it.first, time, 0.01f))
			{
				return &it.second;
			}
			else
			{
				if (it.first < time)
					mostClose = &it.second;
			}
		}
		return mostClose;
	}

	const Vec3* AnimationData::FindRotEuler(float time)
	{
		const Vec3* mostClose = 0;
		for (const auto& it : mEuler)
		{
			if (IsEqual(it.first, time, 0.01f))
			{
				return &it.second;
			}
			else
			{
				if (it.first < time)
					mostClose = &it.second;
			}
		}
		return mostClose;
	}


	const Quat* AnimationData::FindRot(float time)
	{
		const Quat* mostClose = 0;
		for (const auto& it : mRot)
		{
			if (IsEqual(it.first, time, 0.01f))
			{
				return &it.second;
			}
			else
			{
				if (it.first < time)
					mostClose = &it.second;
			}
		}
		return mostClose;
	}

	void AnimationData::PickRot(float time, bool cycled, const Quat** prev, const Quat** next, float& interpol)
	{
		int i = 0;
		*prev = &mRot.begin()->second;			
		*next = *prev;
		float prevTime = mRot.begin()->first;
		for (const auto& it : mRot)
		{
			if (it.first <= time)
			{
				prevTime = it.first;
				*prev = &it.second;
			}
			if (it.first >= time)
			{
				*next = &it.second;
				float length = it.first - prevTime;
				if (length!=0.f)
					interpol = (time - prevTime) / length;
				return;
			}
			i++;
		}
	}

}