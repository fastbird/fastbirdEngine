#pragma once
#ifndef _fastbird_Scene_header_included_
#define _fastbird_Scene_header_included_

#include <Engine/IScene.h>

namespace fastbird
{
    class Scene : public IScene
    {
		typedef std::vector<SpatialObject*> SPATIAL_OBJECTS;

		OBJECTS mObjects;
		SPATIAL_OBJECTS mSpatialObjects;
		VectorMap<ICamera*, SPATIAL_OBJECTS> mVisibleObjectsMain;
		VectorMap<ICamera*, SPATIAL_OBJECTS> mVisibleObjectsLight;
		VectorMap<ICamera*, SPATIAL_OBJECTS> mPreRenderList;
		VectorMap<ICamera*, SPATIAL_OBJECTS> mVisibleTransparentObjects;
		SmartPtr<ISkyBox> mSkyBox;
		SmartPtr<ISkySphere> mSkySphere;
		SmartPtr<ISkySphere> mSkySphereBlend; // alphablend sky
		
		SmartPtr<ILight> mDirectionalLight[2];

		bool mSkipSpatialObjects;
		bool mSkyRendering;
		FB_CRITICAL_SECTION mSpatialObjectsCS;

		std::vector< SmartPtr<IMeshObject> > mCloudVolumes;

		std::vector<ISceneListener*> mListeners;
		Vec3 mWindDir;
		float mWindVelocity;
		Vec3 mWindVector;
		Color mFogColor;
		VectorMap<ICamera*, unsigned> mLastPreRenderFramePerCam;
		bool mDrawClouds;
		bool mRttScene;

    public:
        Scene();
        virtual ~Scene();

	protected:
		virtual void FinishSmartPtr();

	public:

        virtual bool AttachObject(SpatialObject* pSpatialObject);
        virtual bool DetachObject(SpatialObject* pSpatialObject);

        virtual bool AttachObject(IObject* pObject);
        virtual bool DetachObject(IObject* pObject);

		virtual void AttachSkyBox(ISkyBox* pSkyBox);
		virtual void AttachSkySphere(ISkySphere* p);
		virtual void AttachSkySphereBlend(ISkySphere* p);
		virtual void DetachSkySphere();
		virtual void DetachSkySphereBlend();
		virtual void SwapSkySphereBlendAndDetach();
		virtual ISkySphere* GetSkySphereBlend() const { return mSkySphereBlend; }
		virtual void ToggleSkyRendering();
		virtual void SetSkyRendering(bool render);
		virtual bool GetSkyRendering() const { return mSkyRendering; }
		virtual ISkySphere* GetSkySphere() const {return mSkySphere;}
		virtual void SetSkipSpatialObjects(bool skip);

		virtual OBJECTS QueryVisibleObjects(ICamera* cam, const Ray3& ray, unsigned limitObject, bool narrow = false);
		virtual void ClearEverySpatialObject();

		virtual void MakeVisibleSet(ICamera* cam);
		virtual void PreRender();
		virtual void Render();

		virtual const Vec3& GetWindVector() const;

		virtual void AddCloudVolume(IMeshObject* p);
		virtual void RemoveClouds();
		virtual void RenderCloudVolumes();

		virtual const Color& GetFogColor() const { return mFogColor; }
		virtual void SetFogColor(const Color& c);
		virtual void SetDrawClouds(bool e);

		virtual void AddListener(ISceneListener* listener);
		virtual void RemoveListener(ISceneListener* listener);

		virtual const std::vector<SpatialObject*>& GetVisibleSpatialList(ICamera* cam);

		virtual unsigned GetNumSpatialObjects() const;

		virtual void PrintSpatialObject();

		virtual void SetRttScene(bool set);
		virtual bool IsRttScene() const{ return mRttScene; }

		virtual ILight* GetLight(unsigned idx);
		
		// internal use
		void UpdateLightCamera();
		void SetLightToRenderer();

        
    };
}

#endif //_fastbird_Scene_header_included_