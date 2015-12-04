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
#include "FBMathLib/Math.h"
namespace fb
{
	struct POINT_LIGHT_CONSTANTS;
	FB_DECLARE_SMART_PTR(Scene);
	FB_DECLARE_SMART_PTR(PointLight);
	FB_DECLARE_SMART_PTR(PointLightManager);
	class PointLightManager
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(PointLightManager);
		PointLightManager();

	public:
		static PointLightManagerPtr Create();

		void SetScene(ScenePtr scene);
		PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime, bool manualDeletion);		
		void Update(Real dt);
		void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst);
		unsigned GetNumPointLights() const;
	};
}