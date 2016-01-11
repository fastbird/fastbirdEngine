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
#include "RendererEnums.h"
#include "RendererStructs.h"
#include "FBMathLib/Color.h"
#include "FBCommonHeaders/Observable.h"
#include "IRenderTargetObserver.h"
namespace fb
{
	struct RenderTargetParam;
	typedef unsigned RenderTargetId;
	FB_DECLARE_SMART_PTR(IRenderTargetObserver);
	FB_DECLARE_SMART_PTR(IRenderStrategy);
	FB_DECLARE_SMART_PTR(IInputInjector);
	FB_DECLARE_SMART_PTR(GaussianDist);
	FB_DECLARE_SMART_PTR(RenderPipeline);	
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(Mouse);
	FB_DECLARE_SMART_PTR(Keyboard);
	FB_DECLARE_SMART_PTR(RendererOptions);
	FB_DECLARE_SMART_PTR(Renderer);
	FB_DECLARE_SMART_PTR(RenderTarget);
	class FB_DLL_RENDERER RenderTarget : public Observable<IRenderTargetObserver>
	{
		static const int FB_NUM_BLOOM_TEXTURES = 3;
		static const int FB_NUM_STAR_TEXTURES = 12;
		
		FB_DECLARE_PIMPL_NON_COPYABLE(RenderTarget);		
		RenderTarget();

	public:
		static RenderTargetPtr Create();
		~RenderTarget();

		//-------------------------------------------------------------------
		// Observable<IRenderTargetObserver>
		//-------------------------------------------------------------------
		void OnObserverAdded(IRenderTargetObserverPtr observer);

		//-------------------------------------------------------------------
		// InputConsumer From Renderer
		//-------------------------------------------------------------------
		void ConsumeInput(IInputInjectorPtr injector);

		//-------------------------------------------------------------------		
		RenderTargetId GetId() const;
		void SetAssociatedWindowId(HWindowId id);
		HWindowId GetAssociatedWindowId() const;
		bool CheckOptions(const RenderTargetParam& param);
		
		IRenderStrategyPtr SetRenderStrategy(IRenderStrategyPtr strategy);
		IScenePtr RegisterScene(IScenePtr scene);
		void TakeOwnershipScene(IScenePtr scene);
		IScenePtr GetScene() const;
		void SetCamera(CameraPtr cam);
		/// Returns previous camera
		CameraPtr ReplaceCamera(CameraPtr cam);
		CameraPtr GetCamera() const;

		TexturePtr GetRenderTargetTexture() const;
		TexturePtr GetDepthStencilTexture() const;
		const Vec2I& GetSize() const;
		void DeleteBuffers();

		void SetColorTextureDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool miplevel, bool cubeMap);
		void SetDepthStencilDesc(int width, int height, PIXEL_FORMAT format, bool srv, bool cubeMap);
		void SetClearValues(const Color& color, Real z, UINT8 stencil);
		void SetClearColor(const Color& color);
		void SetClearDepthStencil(Real z, UINT8 stencil);
		void SetColorTexture(TexturePtr pTexture);
		void SetDepthTexture(TexturePtr pTexture);
		void RemoveTextures();
		void SetEnvTexture(TexturePtr texture);
		void SetEnable(bool enable);
		bool GetEnable() const;
		bool GetUsePool() const;
		void SetUsePool(bool usePool);
		const Viewport& GetViewport() const;
		/// true, if underyling render strategy support glow rendering
		bool IsGlowSupported() const;

		void Bind(size_t face = 0);
		void BindTargetOnly(bool hdr);
		void BindDepthTexture(bool bind);
		bool Render(size_t face = 0);
		void Unbind();
		void GlowRenderTarget(bool bind);

		void SetLightCamWidth(Real width);
		void SetLightCamHeight(Real height);
		void SetLightCamNear(Real n);
		void SetLightCamFar(Real f);

		void DrawOnEvent(bool set);
		void TriggerDrawEvent();

		bool SetSmallSilouetteBuffer();
		bool SetBigSilouetteBuffer();		

		void OnRendererOptionChanged(RendererOptionsPtr options, const char* name);
		
	};
}