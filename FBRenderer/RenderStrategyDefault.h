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
#include "IRenderStrategy.h"
#include "FBSceneManager/ISceneObserver.h"
namespace fb{
	FB_DECLARE_SMART_PTR(RenderTarget);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(RenderStrategyDefault);
	class RenderStrategyDefault : public IRenderStrategy, public ISceneObserver{
		FB_DECLARE_PIMPL_NON_COPYABLE(RenderStrategyDefault);
		RenderStrategyDefault();

	public:	
		static RenderStrategyDefaultPtr Create();
		~RenderStrategyDefault();

		//-------------------------------------------------------------------
		// IRenderStrategy
		//-------------------------------------------------------------------
		void SetScene(IScenePtr scene);
		void SetRenderTarget(RenderTargetPtr renderTarget);		
		void Render(size_t face);
		bool IsHDR() const;
		bool IsGlowSupported();
		bool SetHDRTarget();
		bool SetSmallSilouetteBuffer();
		bool SetBigSilouetteBuffer();
		void GlowRenderTarget(bool bind);
		void DepthTexture(bool bind);
		void OnRendererOptionChanged(RendererOptionsPtr options, const char* optionName);
		void OnRenderTargetSizeChanged(const Vec2I& size);			

		//ISceneObserver
		void OnAfterMakeVisibleSet(IScene* scene) OVERRIDE;
		void OnBeforeRenderingOpaques(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) OVERRIDE;
		void OnBeforeRenderingOpaquesRenderStates(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) OVERRIDE;
		void OnAfterRenderingOpaquesRenderStates(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) OVERRIDE;
		void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut) OVERRIDE;
	};
}