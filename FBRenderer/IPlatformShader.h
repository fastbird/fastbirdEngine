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
#include "FBCommonHeaders/Types.h"
#include "ShaderDefines.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(IPlatformShader);
	class IPlatformShader
	{
	public:

		/**Bind all type of platform shaders
		Empty shader will be removed from the pipeline.
		*/
		virtual void Bind() = 0;
		/** Bind vertex shader only.*/
		virtual void BindVS() = 0;
		/** Bind hull shader only.*/
		virtual void BindHS() = 0;
		/** Bind domain shader only.*/
		virtual void BindDS() = 0;
		/** Bind geometry shader only.*/
		virtual void BindGS() = 0;
		/** Bind pixel shader only.	*/
		virtual void BindPS() = 0;	
		
		virtual bool GetCompileFailed() const = 0;
		virtual void* GetVSByteCode(unsigned& size) const = 0;		
		virtual void SetDebugName(const char* name) = 0;
		/** Returns true if \a inc is a related header file.
		*/
		virtual bool CheckIncludes(const char* inc) = 0;

	protected:
		~IPlatformShader(){}
	};
}