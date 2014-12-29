#pragma once
#ifndef _fastbird_IScene_header_included_
#define _fastbird_IScene_header_included_

#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Ray3.h>

namespace fastbird
{
	class ISceneListener;
    class IObject;
    class SpatialObject;
	class ISkyBox;
	class ISkySphere;
	class IMeshObject;
	class CLASS_DECLSPEC_ENGINE IScene : public ReferenceCounter
	{
	public:
		typedef std::vector<IObject*> OBJECTS;

		static IScene* CreateScene();

		virtual ~IScene() {}
		virtual bool AttachObject(SpatialObject* pSpatialObject) = 0;
        virtual bool DetachObject(SpatialObject* pSpatialObject) = 0;

        virtual bool AttachObject(IObject* pObject) = 0;
        virtual bool DetachObject(IObject* pObject) = 0;

		virtual void AttachSkyBox(ISkyBox* pSkyBox) = 0;
		virtual void AttachSkySphere(ISkySphere* p) = 0;
		virtual void AttachSkySphereBlend(ISkySphere* p) = 0;
		virtual void DetachSkySphere() = 0;
		virtual void DetachSkySphereBlend() = 0;
		virtual void SwapSkySphereBlendAndDetach() = 0;
		virtual ISkySphere* GetSkySphereBlend() const = 0;
		virtual void ToggleSkyRendering() = 0;
		virtual void SetSkyRendering(bool render) = 0;
		virtual bool GetSkyRendering() const = 0;
		virtual ISkySphere* GetSkySphere() const = 0;

		// narrow : do narrow phase collision check
		virtual OBJECTS QueryVisibleObjects(const Ray3& ray, unsigned limitObject, bool narrow = false) = 0;
		virtual void SetSkipSpatialObjects(bool skip) = 0;

		virtual void ClearEverySpatialObject() = 0;

		virtual void MakeVisibleSet() = 0;
		virtual void PreRender() = 0;
		virtual void Render() = 0;

		virtual const Vec3& GetWindVector() const = 0;
		virtual void RenderCloudVolumes() = 0;
		virtual void AddCloudVolume(IMeshObject* p)= 0;
		virtual void RemoveClouds() = 0;
		virtual const Color& GetFogColor() const = 0;
		virtual void SetFogColor(const Color& c) = 0;
		virtual void SetDrawClouds(bool e) = 0;

		virtual void AddListener(ISceneListener* listener) = 0;
		virtual void RemoveListener(ISceneListener* listener) = 0;

		virtual const std::vector<SpatialObject*>& GetVisibleSpatialList() const = 0;
	};
}

#endif //_fastbird_IScene_header_included_