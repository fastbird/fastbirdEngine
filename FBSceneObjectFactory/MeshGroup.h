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
#include "FBSceneManager/SpatialSceneObject.h"
#include "MeshAuxiliary.h"
#include "CollisionInfo.h"
#include "FBRenderer/RenderPass.h"
namespace fb
{
	class Color;
	class AnimationData;
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(MeshObject);
	FB_DECLARE_SMART_PTR(MeshGroup);
	class FB_DLL_SCENEOBJECTFACTORY MeshGroup : public SpatialSceneObject
	{
		FB_DECLARE_PIMPL_CLONEABLE(MeshGroup);

	protected:
		MeshGroup();
		~MeshGroup();
		
	public:
		static MeshGroupPtr Create();
		static MeshGroupPtr Create(const MeshGroup& other);

		void PreRender(const RenderParam& param, RenderParamOut* paramOut);
		void Render(const RenderParam& param, RenderParamOut* paramOut);
		void PostRender(const RenderParam& param, RenderParamOut* paramOut);		
		
		void Update(TIME_PRECISION dt);
		//---------------------------------------------------------------------------
		// Own functions
		//---------------------------------------------------------------------------		
		MaterialPtr GetMaterial();
		void SetMaterial(MaterialPtr mat, RENDER_PASS pass);
		void SetMaterial(const char* path, RENDER_PASS pass);

		MeshGroupPtr Clone() const;
		void SetEnableHighlight(bool enable);
		// order of inserting meshes is important. parent first.
		// transformation is in parent space.
		size_t AddMesh(MeshObjectPtr mesh, const Transformation& transform, size_t parent);
		const char* GetNameOfMesh(size_t idx);
		size_t GetNumMeshes() const;
		void AddMeshRotation(size_t idx, const Quat& rot);
		const Quat& GetMeshRotation(size_t idx) const;
		void SetMeshRotation(size_t idx, const Quat& rot);
		const Vec3& GetMeshOffset(size_t idx) const;
		/// Returns auxiliaries data
		/// You do not own the returned pointer.
		const AUXILIARIES* GetAuxiliaries(size_t idx) const;
		void SetAuxiliaries(size_t idx, const AUXILIARIES& aux);
		void AddAuxiliary(const AUXILIARY& aux);
		void AddAuxiliary(size_t idx, const AUXILIARY& v);
		void SetCollisionShapes(COLLISION_INFOS& colInfos);
		void AddCollisionShape(size_t idx, std::pair<ColisionShapeType::Enum, Transformation>& data);						
		// SpatialObject
		void SetLocation(const Transformation& t);
		// force == false
		void UpdateTransform(const RenderParam& param, RenderParamOut* paramOut);
		void UpdateTransform(const RenderParam& param, RenderParamOut* paramOut, bool force);
		void PlayAction(const std::string& name, bool immediate, bool reverse);
		bool IsActionDone(const char* action) const;
		bool IsPlayingAction() const;
		Transformation GetToLocalTransform(unsigned meshIdx);
		Transformation GetToLocalTransform(const char* meshName);
		void SetAlpha(float alpha);
		void SetForceAlphaBlending(bool enable, float alpha, float forceGlow = 0.f, bool disableDepth = false);
		void SetAmbientColor(const Color& color);
		MeshObjectPtr GetMeshObject(unsigned idx);
		unsigned GetNumCollisionShapes() const;
		const FBCollisionShapeConstPtr GetCollisionShape(unsigned idx) const;
		bool HasCollisionShapes() const;
		unsigned GetNumCollisionShapes(unsigned idx) const;
	};
}