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
#include "FBRenderer/RenderPass.h"
#include "FBCommonHeaders/GenericNotifier.h"
#include "FBSceneObjectFactory/ISkySphereLIstener.h"
#include "ISkyFacadeListener.h"
namespace fb{
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(SkyFacade);
	class SkySphere;
	class ISkyFacadeListener;	
	class FB_DLL_ENGINEFACADE SkyFacade : public GenericNotifier<ISkyFacadeListener>, public ISkySphereListener{
		FB_DECLARE_PIMPL_NON_COPYABLE(SkyFacade);
		SkyFacade();
		~SkyFacade();

	public:
		static SkyFacadePtr Create();
		static SkyFacadePtr GetMain();

		SkyFacadePtr CreateSkySphere();
		SkyFacadePtr CreateSkyBox(const char* materialPath);

		void SetMaterial(const char* path, RENDER_PASS pass);
		void SetGeometry(const char* path);
		void AttachToScene();
		void AttachToScene(IScenePtr scene);
		void DetachFromScene();
		MaterialPtr GetMaterial() const;
		void UpdateEnvironmentMap(const Vec3& pos);
		/// Attach as a blending sky of the main scene.
		void AttachToBlend();
		void SetAlpha(float alpha);
		void PrepareInterpolation(float time, SkyFacadePtr startFrom);		
		void AttachBlendingSky(SkyFacadePtr blending);
		void SetInterpolationData(unsigned index, const Vec4& data);
		void StartInterpolation(float time);

		// GenericNotifier
		void AddListener(ISkyFacadeListener* listener) OVERRIDE;
		void RemoveListener(ISkyFacadeListener* listener) OVERRIDE;
		// ISkySphereListener
		void OnInterpolationFinished(SkySphere* sky);
	};
}