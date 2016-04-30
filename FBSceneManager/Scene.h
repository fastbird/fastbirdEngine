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
#include "SceneManager.h"
#include "ISceneObserver.h"
#include "IScene.h"
#include "FBRenderer/RenderParam.h"
#include "FBCommonHeaders/Observable.h"
#undef AttachObjectFB
#undef DetachObjectFB
namespace fb{
	class Vec3;
	class Color;
	class Ray;
	FB_DECLARE_SMART_PTR(PointLight);
	FB_DECLARE_SMART_PTR(PointLightManager);
	FB_DECLARE_SMART_PTR(ICamera);
	FB_DECLARE_SMART_PTR(SkySphere);
	FB_DECLARE_SMART_PTR(DirectionalLight);
	FB_DECLARE_SMART_PTR(Scene);
	class FB_DLL_SCENEMANAGER Scene : public IScene, public Observable<ISceneObserver>{
		FB_DECLARE_PIMPL_NON_COPYABLE(Scene);		
		Scene(const char* name);
		~Scene();
		
	public:		
		typedef std::vector<SpatialSceneObject*> SPATIAL_OBJECTS_RAW;
		static ScenePtr Create(const char* name);		
		
		
		//---------------------------------------------------------------------------
		// IScene interface
		//---------------------------------------------------------------------------
		void AddSceneObserver(int ISceneObserverEnum, ISceneObserverPtr observer);
		void GetDirectionalLightInfo(DirectionalLightIndex::Enum index, DirectionalLightInfo& data);
		const Vec3& GetMainLightDirection();
		void SetLightDirection(DirectionalLightIndex::Enum idx, const Vec3& dir);
		void PreRender(const RenderParam& prarm, RenderParamOut* paramOut);
		void Render(const RenderParam& prarm, RenderParamOut* paramOut);		
		void PreRenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut);
		void RenderCloudVolumes(const RenderParam& prarm, RenderParamOut* paramOut);
		const Color& GetFogColor() const;	
		void AttachSky(SceneObjectPtr p);
		void DetachSky();
		bool AttachObjectFB(SceneObjectPtr object, SceneObject* rawPointer);
		bool DetachObject(SceneObject* object);
		bool AttachObjectFB(SpatialSceneObjectPtr object, SpatialSceneObject* rawPointer);
		bool DetachObject(SpatialSceneObject* object);
		PointLightPtr CreatePointLight(const Vec3& pos, Real range, const Vec3& color, Real intensity, Real lifeTime,
			bool manualDeletion);
		void GatherPointLightData(const BoundingVolume* aabb, const Transformation& transform, POINT_LIGHT_CONSTANTS* plConst);

		//---------------------------------------------------------------------------
		const char* GetName() const;
		void Update(TIME_PRECISION dt);
		/// Returns currently rendering pass which is RenderParam.mRenderPass when the Render() is called.
		int GetRenderPass() const;

		void SetSkipSpatialObjects(bool skip);
		void ClearEverySpatialObject();
		unsigned GetNumObjects() const;
		unsigned GetNumSpatialObjects() const;
		/** You do not own the returned pointers of this function
		do not keep any pointer. */
		const SPATIAL_OBJECTS_RAW* GetVisibleSpatialList(ICameraPtr cam);
		void PrintSpatialObject();
		
		SceneObjectPtr GetSky();		
		void ToggleSkyRendering();
		void SetSkyRendering(bool render);
		bool GetSkyRendering();		

		const Vec3& GetWindVector() const;

		/** p is a MeshObject*/
		void AddCloudVolume(SpatialSceneObjectPtr p);
		void ClearClouds();
		
		void SetFogColor(const Color& c);
		void SetDrawClouds(bool e);

		/** True if this scene is attached in the render target which is not related swap-chain.*/
		void SetRttScene(bool set);
		bool IsRttScene() const;

		DirectionalLightPtr GetDirectionalLight(unsigned idx);		
		PointLightManagerPtr GetPointLightMan() const;		
		void RefreshPointLight();	
		bool NeedToRefreshPointLight() const;
		const AABB& GetSceneAABB();

		void MakeVisibleSet(ICamera* cam, bool force);
		void MakeVisibleSet(ICamera* cam);
	};
}

#define AttachObjectFB(p) AttachObjectFB((p), (p).get())
#define DetachObjectFB(p) DetachObjectFB((p), (p).get())