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
#include "FBRenderer/PixelFormats.h"
#include "FBMathLib/Vec2I.h"
namespace fb{
	struct RenderTargetParamEx
	{
		RenderTargetParamEx()
			: mEveryFrame(false)
			, mSize(200, 200)
			, mPixelFormat(PIXEL_FORMAT_R8G8B8A8_UNORM)
			, mShaderResourceView(false)
			, mMipmap(false)
			, mCubemap(false)
			, mWillCreateDepth(false)
			, mUsePool(false)
		{

		}
		Vec2I mSize;
		PIXEL_FORMAT mPixelFormat;
		bool mEveryFrame;
		bool mShaderResourceView;
		bool mMipmap;
		bool mCubemap;
		bool mWillCreateDepth; // set true, and call IRenderTarget::SetDepthStencilDesc().
		bool mUsePool;
		/// If specified, The render target will owns(strong pointer) the new scene with this name.
		std::string mSceneNameToCreateAndOwn;
		// If specified, environment texture will be overridden.
		std::string mEnvironmentTexture;
	};
}