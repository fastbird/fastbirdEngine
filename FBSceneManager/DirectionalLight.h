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
#include "FBMathLib/Math.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(DirectionalLight);
	//------------------------------------------------------------------------
	class FB_DLL_SCENEMANAGER DirectionalLight
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(DirectionalLight);
		DirectionalLight();
		~DirectionalLight();


	public:
		
		static DirectionalLightPtr Create();

		const Vec3& GetDirection();
		const Vec3& GetDiffuse();
		const Vec3& GetSpecular();
		Real GetIntensity() const;
		Real GetAttenuation() const;
		Real GetExponent() const;

		void AddTheta(Real radian);
		void AddPhi(Real radian);

		// The direciton from an actor to the sun.
		void SetDirection(const Vec3& pos);
		void SetDiffuse(const Vec3& diffuse);
		void SetSpecular(const Vec3& specular);
		void SetIntensity(Real intensity);
		void SetAttenuation(Real attenuation){}
		void SetExponent(Real exponent) {}

		void PrepareInterpolation(Real destTheta, Real destPhi, Real destIntensity, const Vec3& destDiffuse, Real time);
		void AddInterpolTime(Real time);
		Vec3 GetInterpolDir(unsigned target) const;
		Real GetInterpolIntensity(unsigned target) const;
		const Vec3& GetInterpolDiffuse(unsigned target) const;
		Real GetTheta() const;
		Real GetPhi() const;

		void Update(Real dt);

		void PreRender();
		void Render();
		void PostRender();

		void CopyLight(DirectionalLightPtr src);
		
	};
}
