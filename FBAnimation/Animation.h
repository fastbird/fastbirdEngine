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
	FB_DECLARE_SMART_PTR(AnimationData);
	FB_DECLARE_SMART_PTR(Animation);
	/** Represents an animation instance using the AnimationData.
	*/
	class FB_DLL_ANIMATION Animation{
		FB_DECLARE_PIMPL_CLONEABLE(Animation);
		Animation();

	public:
		static AnimationPtr Create();
		static AnimationPtr Create(const Animation& other);
		~Animation();

		AnimationPtr Clone() const;
		void PlayAction(const std::string& name, bool immediate, bool reverse);
		bool IsActionDone(const char* action) const;
		bool IsPlaying() const;
		void Update(TIME_PRECISION dt);
		const Transformation& GetResult() const;
		bool Changed() const;
		void SetAnimationData(AnimationDataPtr data);

	};
}