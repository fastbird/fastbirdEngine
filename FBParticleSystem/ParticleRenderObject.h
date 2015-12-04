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
#include "FBSceneManager/SpatialSceneObject.h"
namespace fb
{
	struct ParticleRenderKey;
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(ParticleRenderObject);
	class ParticleRenderObject : public SpatialSceneObject
	{
		FB_DECLARE_PIMPL_NON_COPYABLE(ParticleRenderObject);
		ParticleRenderObject();
		~ParticleRenderObject();
		
	public:
		static const int MAX_SHARED_VERTICES;
		static ParticleRenderObjectPtr GetRenderObject(IScenePtr scene, ParticleRenderKey& key, bool& created);
		static void ClearParticles();
		static void EndUpdateParticles();
		static void FinalizeRenderObjects();
		static size_t GetNumRenderObject();
		static size_t GetNumDrawCalls();
		static size_t GetNumPrimitives();

		//---------------------------------------------------------------------------
		// SceneObject Interfaces
		//---------------------------------------------------------------------------
		void PreRender(const RenderParam& param, RenderParamOut* paramOut);
		void Render(const RenderParam& param, RenderParamOut* paramOut);
		void PostRender(const RenderParam& param, RenderParamOut* paramOut);

		//---------------------------------------------------------------------------
		// OWN
		//---------------------------------------------------------------------------
		MaterialPtr GetMaterial() const;
		void Clear();
		void EndUpdate();
		void SetDoubleSided(bool set);
		void SetTexture(const char* texturePath);

		struct Vertex
		{
			Vec3f mPos;
			Vec4f mUDirection_Intensity;
			Vec3f mVDirection;
			Vec4f mPivot_Size;
			Vec4f mRot_Alpha_uv;
			Vec2f mUVStep;
			DWORD mColor;
		};
		Vertex* Map(UINT numVertices, unsigned& canWrite);
	};
}