#pragma once
#ifndef _fastbird_Scene_header_included_
#define _fastbird_Scene_header_included_

#include <Engine/IScene.h>

namespace fastbird
{
    class Scene : public IScene
    {
    public:
        Scene();
        virtual ~Scene();

        virtual bool AttachObject(SpatialObject* pSpatialObject);
        virtual bool DetachObject(SpatialObject* pSpatialObject);

        virtual bool AttachObject(IObject* pObject);
        virtual bool DetachObject(IObject* pObject);

		virtual void AttachSkyBox(ISkyBox* pSkyBox);
		virtual void AttachSkySphere(ISkySphere* p);
		virtual void DetachSkySphere();
		virtual void ToggleSkyRendering();

		virtual OBJECTS QueryVisibleObjects(const Ray3& ray, unsigned limitObject);

		virtual void PreRender();
		virtual void Render();


	protected:
		typedef std::vector<SpatialObject*> SPATIAL_OBJECTS;
		void MakeVisibleSet();


    private:
        OBJECTS mObjects;
		SPATIAL_OBJECTS mSpatialObjects;
		SPATIAL_OBJECTS mVisibleObjects;
		SmartPtr<ISkyBox> mSkyBox;
		SmartPtr<ISkySphere> mSkySphere;
		bool mSkyRendering;
    };
}

#endif //_fastbird_Scene_header_included_