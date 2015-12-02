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

/**
\defgroup FBInputManager
Handles use inputs such as keyboard and mouse.

Required libraries: \b FBDebugLib \b FBTimerLib
*/
#pragma once
#include <memory>
#include "FBCommonHeaders/Types.h"
#include "FBTimer/Timer.h"
namespace fb
{
	class IInputDevice;
	typedef std::shared_ptr<IInputDevice> IInputDevicePtr;
	typedef std::weak_ptr<IInputDevice> IInputDeviceWeakPtr;
	class IInputDevice
	{
	public:
		virtual void EndFrame(TIME_PRECISION gameTimeInSecond) = 0;
		virtual bool IsValid() const = 0;
		virtual void Invalidate(bool buttonClicked = false) = 0;
		virtual void InvalidTemporary(bool invalidate) = 0;
	};
}