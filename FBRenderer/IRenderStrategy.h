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
#include <memory>
#include "FBCommonHeaders/Types.h"
namespace fb{
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(RenderTarget);	
	FB_DECLARE_SMART_PTR(RendererOptions);
	FB_DECLARE_SMART_PTR(IRenderStrategy);
	class FB_DLL_RENDERER IRenderStrategy{
	protected:
		virtual ~IRenderStrategy(){}

	public:
		virtual void SetScene(IScenePtr scene) = 0;
		virtual void SetRenderTarget(RenderTargetPtr renderTarget) = 0;
		virtual void UpdateLightCamera() = 0;
		virtual void Render(size_t face) = 0;
		virtual bool IsHDR() const = 0;
		virtual bool IsGlowSupported() = 0;		
		virtual CameraPtr GetLightCamera() const = 0;
		virtual bool SetHDRTarget() = 0;
		virtual bool SetSmallSilouetteBuffer() = 0;
		virtual bool SetBigSilouetteBuffer() = 0;	
		virtual void GlowRenderTarget(bool bind) = 0;
		virtual void DepthTexture(bool bind) = 0;
		virtual void OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName) = 0;

		// debugging feature
		virtual TexturePtr GetShadowMap() = 0;
	};
}