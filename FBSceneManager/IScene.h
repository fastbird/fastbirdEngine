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
#include "FBMathLib/Vec4.h"
#include "DirectionalLightIndex.h"

namespace fb{
	struct DirectionalLightInfo{
		Vec4 mDirection_Intensiy;
		Vec4 mDiffuse;
		Vec4 mSpecular;
	};
	struct RenderParam;
	struct RenderParamOut;
	class Color;
	FB_DECLARE_SMART_PTR(SpatialSceneObject);
	FB_DECLARE_SMART_PTR(SceneObject);
	FB_DECLARE_SMART_PTR(SkySphere);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(ISceneObserver);
	class IScene{
	public:
		virtual void AddSceneObserver(int ISceneObserverEnum, ISceneObserverPtr observer) = 0;
		virtual void GetDirectionalLightInfo(DirectionalLightIndex::Enum idx, DirectionalLightInfo& data) = 0;
		virtual const Vec3& GetMainLightDirection() = 0;
		virtual void SetLightDirection(DirectionalLightIndex::Enum idx, const Vec3& dir) = 0;
		virtual void PreRender(const RenderParam& prarm, RenderParamOut* paramOut) = 0;
		virtual void Render(const RenderParam& prarm, RenderParamOut* paramOut) = 0;
		virtual void PreRenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut) = 0;
		virtual void RenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut) = 0;			
		virtual const Color& GetFogColor() const = 0;
		virtual void AttachSky(SceneObjectPtr p) = 0;
		virtual void DetachSky() = 0;
		virtual bool AttachObjectFB(SceneObjectPtr object, SceneObject* rawPointer) = 0;
		virtual bool DetachObject(SceneObject* object) = 0;
		virtual bool AttachObjectFB(SpatialSceneObjectPtr object, SpatialSceneObject* rawPointer) = 0;
		virtual bool DetachObject(SpatialSceneObject* object) = 0;
	};
}
#define AttachObjectFB(p) AttachObjectFB((p), (p).get())
#define DetachObjectFB(p) DetachObjectFB((p), (p).get())