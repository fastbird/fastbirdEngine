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
#include "RendererEnums.h"
#include "RendererStructs.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(IPlatformTexture);
	class IPlatformTexture
	{
	public:
		virtual Vec2ITuple GetSize() const = 0;
		virtual PIXEL_FORMAT GetPixelFormat() const = 0;		
		virtual bool IsReady() const = 0;
		virtual void Bind(BINDING_SHADER shader, int slot) const = 0;
		virtual MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag) const = 0;
		virtual void Unmap(UINT subResource) const = 0;
		virtual void CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, 
			UINT dstX, UINT dstY, UINT dstZ, UINT srcSubresource, Box3D* srcBox) const = 0;
		virtual void SaveToFile(const char* filename) const = 0;
		virtual void GenerateMips() = 0;
		virtual void SetDebugName(const char* name) = 0;

	protected:
		~IPlatformTexture(){}
	};
}