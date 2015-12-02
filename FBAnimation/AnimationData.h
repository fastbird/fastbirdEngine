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
#include "FBCommonHeaders/platform.h"
namespace fb{
	class Transformation;
	class Vec3;
	class Quat;
	FB_DECLARE_SMART_PTR(AnimationData);
	/** Represents an unique animation set.
	*/
	class FB_DLL_ANIMATION AnimationData{
		FB_DECLARE_PIMPL_NON_COPYABLE(AnimationData);
		AnimationData();

	public:
		static AnimationDataPtr Create();
		~AnimationData();

		enum PosComp
		{
			X,
			Y,
			Z
		};
		struct Action
		{			
			std::string mName;
			float mStartTime;
			float mEndTime;
			float mLength;
			bool mLoop;
			const Vec3* mPosStartEnd[2];
			const Quat* mRotStartEnd[2];

			Action();
		};

		void AddPosition(float time, float v, PosComp comp);
		void AddScale(float time, float v, PosComp comp);
		void AddRotEuler(float time, float v, PosComp comp);
		bool HasPosAnimation() const;
		bool HasRotAnimation() const;
		bool HasScaleAnimation() const;
		void SetName(const char* name);
		const char* GetName() const;
		void PickPos(TIME_PRECISION time, bool cycled, const Vec3** prev, const Vec3** next, TIME_PRECISION& interpol);
		void PickRot(TIME_PRECISION time, bool cycled, const Quat** prev, const Quat** next, TIME_PRECISION& interpol);
		void ToLocal(const Transformation& tolocal);
		bool ParseAction(const char* filename);
		const Action* GetAction(const char* name) const;


	};
}