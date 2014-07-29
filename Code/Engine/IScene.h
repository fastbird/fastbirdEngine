#pragma once
#ifndef _fastbird_IScene_header_included_
#define _fastbird_IScene_header_included_

#include <CommonLib/SmartPtr.h>
#include <CommonLib/Math/Ray3.h>

namespace fastbird
{
    class IObject;
    class SpatialObject;
	class ISkyBox;
	class ISkySphere;
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
		virtual void DetachSkySphere() = 0;
		virtual void ToggleSkyRendering() = 0;

		virtual OBJECTS QueryVisibleObjects(const Ray3& ray, unsigned limitObject) = 0;

		virtual void PreRender() = 0;
		virtual void Render() = 0;
	};
}

#endif //_fastbird_IScene_header_included_