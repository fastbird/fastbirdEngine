#include <Engine/StdAfx.h>
#include <Engine/SceneGraph/Scene.h>
#include <Engine/IObject.h>
#include <Engine/SceneGraph/SpatialObject.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/ICamera.h>
#include <Engine/ISkyBox.h>
#include <Engine/ISkySphere.h>
#include <Engine/IMeshObject.h>

using namespace fastbird;

IScene* IScene::CreateScene()
{
	return FB_NEW(Scene);
}

//----------------------------------------------------------------------------
Scene::Scene()
: mSkyRendering(true)
, mSkipSpatialObjects(false)
, mDrawClouds(true)
{
	mVisibleObjects.reserve(1000);
	mVisibleTransparentObjects.reserve(1000);
	mSkyBox = 0;

	mWindDir = Vec3(1, 0, 0);
	mWindVelocity = 0.0f;
	mWindVector = mWindDir * mWindVelocity;
}

//----------------------------------------------------------------------------
Scene::~Scene()
{
	while(!mObjects.empty())
	{
		this->DetachObject(mObjects[0]);
	}

	while (!mSpatialObjects.empty())
	{
		this->DetachObject(mSpatialObjects[0]);
	}
}

//----------------------------------------------------------------------------
bool Scene::AttachObject(SpatialObject* pSpatialObject)
{
	LOCK_CRITICAL_SECTION lock(mSpatialObjectsCS);
    auto it = std::find(mSpatialObjects.begin(), mSpatialObjects.end(), pSpatialObject);
    if (it!=mSpatialObjects.end())
    {
        return false;
    }

    mSpatialObjects.push_back(pSpatialObject);
    pSpatialObject->OnAttachedToScene(this);

    return true;
}

//----------------------------------------------------------------------------
bool Scene::DetachObject(SpatialObject* pSpatialObject)
{
	LOCK_CRITICAL_SECTION lock(mSpatialObjectsCS);
    auto it = std::find(mSpatialObjects.begin(), mSpatialObjects.end(), pSpatialObject);
    if (it != mSpatialObjects.end())
    {
        pSpatialObject->OnDetachedFromScene(this);
        mSpatialObjects.erase(it);
        return true;
    }
    
    return false;
}

//----------------------------------------------------------------------------
bool Scene::AttachObject(IObject* pObject)
{
    auto it = std::find(mObjects.begin(), mObjects.end(), pObject);
    if (it!= mObjects.end())
    {
        return false;
    }

    mObjects.push_back(pObject);
    pObject->OnAttachedToScene(this);

    return true;
}

//----------------------------------------------------------------------------
bool Scene::DetachObject(IObject* pObject)
{
    auto it = std::find(mObjects.begin(), mObjects.end(), pObject);
    if (it!= mObjects.end())
    {
        pObject->OnDetachedFromScene(this);
        mObjects.erase(it);
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
void Scene::AttachSkyBox(ISkyBox* pSkyBox)
{
	if (mSkyBox)
		mSkyBox->OnDetachedFromScene(this);
	mSkyBox = pSkyBox;

	if (mSkyBox)
		mSkyBox->OnAttachedToScene(this);
}

void Scene::AttachSkySphere(ISkySphere* p)
{
	if (mSkySphere)
		mSkySphere->OnDetachedFromScene(this);
	mSkySphere = p;

	if (mSkySphere)
	{
		mSkySphere->SetUseAlphaBlend(false);
		mSkySphere->OnAttachedToScene(this);
	}
		
	
}

void Scene::AttachSkySphereBlend(ISkySphere* p)
{
	if (mSkySphereBlend)
		mSkySphereBlend->OnDetachedFromScene(this);
	mSkySphereBlend = p;

	if (mSkySphereBlend)
	{
		mSkySphereBlend->OnAttachedToScene(this);
		mSkySphereBlend->SetUseAlphaBlend(true);
	}
		
}

void Scene::DetachSkySphere()
{
	if (mSkySphere)
		mSkySphere->OnDetachedFromScene(this);
	mSkySphere = 0;
}

void Scene::DetachSkySphereBlend()
{
	if (mSkySphereBlend)
		mSkySphereBlend->OnDetachedFromScene(this);
	mSkySphereBlend = 0;
}

void Scene::SwapSkySphereBlendAndDetach()
{
	if (mSkySphere)
		mSkySphere->OnDetachedFromScene(this);
	mSkySphere = mSkySphereBlend;
	if (mSkySphere)
		mSkySphere->SetUseAlphaBlend(false);
	mSkySphereBlend = 0;
}

//----------------------------------------------------------------------------
void Scene::ToggleSkyRendering()
{
	mSkyRendering = !mSkyRendering;
}

//----------------------------------------------------------------------------
void Scene::SetSkyRendering(bool render)
{
	mSkyRendering = render;
}

//----------------------------------------------------------------------------
void Scene::SetSkipSpatialObjects(bool skip)
{
	mSkipSpatialObjects = skip;
}

//----------------------------------------------------------------------------
IScene::OBJECTS Scene::QueryVisibleObjects(const Ray3& ray, unsigned limitObject)
{
	if (mSkipSpatialObjects)
		return IScene::OBJECTS();
	OBJECTS objects;
	// find object from visible list;
	SPATIAL_OBJECTS::iterator it = mVisibleObjects.begin(),
		itEnd = mVisibleObjects.end();
	for (; it!=itEnd && objects.size() < limitObject; it++)
	{
		SpatialObject* pObj = (*it);
		if (pObj->GetObjFlag() & IObject::OF_QUERYABLE)
		{
			Ray3::IResult ret = ray.intersects(pObj->GetBoundingVolumeWorld());
			if (ret.first)
			{
				objects.push_back(pObj);
			}
		}
	}

	return objects;
}

//----------------------------------------------------------------------------
void Scene::MakeVisibleSet()
{
	if (mSkipSpatialObjects)
		return;
	mVisibleObjects.clear();
	mVisibleTransparentObjects.clear();
	auto it = mSpatialObjects.begin(), itEnd = mSpatialObjects.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetObjFlag() & IObject::OF_IGNORE_ME)
			continue;
		if ( gFBEnv->pRenderer->GetCamera()->IsCulled((*it)->GetBoundingVolumeWorld()) )
			continue;
		IMaterial* pmat = (*it)->GetMaterial();
		if (pmat && pmat->IsTransparent())
		{
			mVisibleTransparentObjects.push_back(*it);
		}
		else
		{
			mVisibleObjects.push_back(*it);
		}
		
	}

	const fastbird::Vec3& camPos = gFBEnv->pRenderer->GetCamera()->GetPos();
	std::for_each(mVisibleObjects.begin(), mVisibleObjects.end(),
		[&](SpatialObject* obj)
		{
			const Vec3& objPos = obj->GetPos();
			float dist = (camPos - objPos).Length();
			obj->SetDistToCam(dist);
		}
	);

	std::for_each(mVisibleTransparentObjects.begin(), mVisibleTransparentObjects.end(),
		[&](SpatialObject* obj)
	{
		const Vec3& objPos = obj->GetPos();
		float dist = (camPos - objPos).Length();
		obj->SetDistToCam(dist);
	}
	);

	std::sort(mVisibleObjects.begin(), mVisibleObjects.end(), 
				[](SpatialObject* a, SpatialObject* b) -> bool
				{
					return a->GetDistToCam() < b->GetDistToCam();
				}
			);

	std::sort(mVisibleTransparentObjects.begin(), mVisibleTransparentObjects.end(),
				[](SpatialObject* a, SpatialObject* b) -> bool
			{
				return a->GetDistToCam() > b->GetDistToCam();
			}
			);
}

//----------------------------------------------------------------------------
void Scene::PreRender()
{
	if (!mSkipSpatialObjects)
	{
		MakeVisibleSet();
		auto it = mVisibleObjects.begin(), itEnd = mVisibleObjects.end();
		for (; it != itEnd; it++)
		{
			(*it)->PreRender();
		}

		{
			auto it = mVisibleTransparentObjects.begin(), itEnd = mVisibleTransparentObjects.end();
			for (; it != itEnd; it++)
			{
				(*it)->PreRender();
			}
		}
	}
	if (mSkyRendering)
	{
		if (mSkySphere)
		{
			if (mSkySphereBlend && mSkySphereBlend->GetAlpha() == 1.0f)
			{
				mSkySphereBlend->PreRender();
			}
			else
			{
				mSkySphere->PreRender();
				if (mSkySphereBlend && mSkySphereBlend->GetAlpha()!=0.f)
					mSkySphereBlend->PreRender();
			}
			
		}
		if (mSkyBox)
		{
			mSkyBox->PreRender();
		}			
	}
	

	{
		FB_FOREACH(it, mObjects)
		{
			(*it)->PreRender();
		}
	}
	
}

//----------------------------------------------------------------------------
void Scene::Render()
{
	if (!mSkipSpatialObjects)
	{
		D3DEventMarker mark("VisibleObjects - Opaque");
		auto it = mVisibleObjects.begin(), itEnd = mVisibleObjects.end();
		for (; it != itEnd; it++)
		{
			(*it)->Render();
		}
	}

	if (mSkyRendering)
	{
		D3DEventMarker mark("SkyRendering");
		if (mSkyBox)
		{
			mSkyBox->Render();
			if (mSkySphereBlend)
				mSkySphereBlend->Render();
		}
			
		else if (mSkySphere)
		{
			if (mSkySphereBlend && mSkySphereBlend->GetAlpha() == 1.0f)
			{
				mSkySphereBlend->Render();
			}
			else
			{
				mSkySphere->Render();
				if (mSkySphereBlend && mSkySphereBlend->GetAlpha() != 0.f)
					mSkySphereBlend->Render();
			}
		}
			
	}

	if (!mSkipSpatialObjects)
	{
		{
			D3DEventMarker mark("VisibleObjects - Transparent");
			auto it = mVisibleTransparentObjects.begin(), itEnd = mVisibleTransparentObjects.end();
			for (; it != itEnd; it++)
			{
				(*it)->Render();
			}
		}
	}	

	{
		D3DEventMarker mark("Non-Spatial objects");
		FB_FOREACH(it, mObjects)
		{
			(*it)->Render();
		}
	}
	
}

void Scene::ClearEverySpatialObject()
{
	while (!mSpatialObjects.empty())
	{
		this->DetachObject(mSpatialObjects[0]);
	}
}

//---------------------------------------------------------------------------
const Vec3& Scene::GetWindVector() const
{
	return mWindVector;
}

void Scene::AddCloudVolume(IMeshObject* p)
{
	mCloudVolumes.push_back(p);
	p->ClearRenderStates();
}

void Scene::RemoveClouds()
{
	mCloudVolumes.clear();
}

void Scene::RenderCloudVolumes()
{	
	if (!mDrawClouds)
		return;
	// render obstacles
	gFBEnv->pRenderer->LockDepthStencilState();
	gFBEnv->pRenderer->SetBlueMask();
	Render();
	gFBEnv->pRenderer->UnlockDepthStencilState();

	gFBEnv->pRenderer->SetCloudRendering(true);
	gFBEnv->pRenderer->SetDepthWriteShader();
	// far side
	gFBEnv->pRenderer->SetFrontFaceCullRS();
	gFBEnv->pRenderer->SetNoDepthWriteLessEqual();
	gFBEnv->pRenderer->SetGreenAlphaMaskAddMinusBlend(); // replace very first far volume face, and next will be added if it is not culled.
	for each (IMeshObject* var in mCloudVolumes)
	{
		var->PreRender();
		var->Render();
	}

	// near side	
	gFBEnv->pRenderer->RestoreRasterizerState();
	gFBEnv->pRenderer->SetRedAlphaMaskAddAddBlend();
	for each (IMeshObject* var in mCloudVolumes)
	{
		var->Render();
	}	
	gFBEnv->pRenderer->SetCloudRendering(false);
	gFBEnv->pRenderer->RestoreRasterizerState();
	gFBEnv->pRenderer->RestoreBlendState();
	gFBEnv->pRenderer->RestoreDepthStencilState();
}

void Scene::SetFogColor(const Color& c)
{
	mFogColor = c;
	gFBEnv->pRenderer->UpdateRareConstantsBuffer();
}

void Scene::SetDrawClouds(bool e)
{
	mDrawClouds = e;
}