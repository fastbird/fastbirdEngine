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
#include <Engine/ISceneListener.h>
#include <Engine/ILight.h>

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
	mVisibleObjectsMain.reserve(1000);
	mVisibleObjectsLight.reserve(1000);
	mPreRenderList.reserve(1000);
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
IScene::OBJECTS Scene::QueryVisibleObjects(const Ray3& ray, unsigned limitObject, bool narrow/* = false*/)
{
	if (mSkipSpatialObjects)
		return IScene::OBJECTS();
	OBJECTS objects;
	// find object from visible list;
	SPATIAL_OBJECTS::iterator it = mVisibleObjectsMain.begin(),
		itEnd = mVisibleObjectsMain.end();
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
	if (!narrow)
	{
		return objects;
	}

	OBJECTS objects2;
	for (auto var : objects)
	{
		SpatialObject* pObj = (SpatialObject*)var;
		unsigned num = pObj->GetNumCollisionShapes();
		if (num == 0)
		{
			objects2.push_back(pObj);
		}
		else
		{
			for (unsigned i = 0; i < num; ++i)
			{
				const CollisionShape* cs = pObj->GetCollisionShape(i);
				Ray3::IResult ret = cs->intersects(ray, pObj->GetTransform());
				if (ret.first)
				{
					objects2.push_back(pObj);
					continue;
				}
			}
		}
		
	}
	return objects2;
}

//----------------------------------------------------------------------------
void Scene::MakeVisibleSet()
{
	if (mSkipSpatialObjects)
		return;
	mVisibleObjectsMain.clear();
	mVisibleObjectsLight.clear();
	mVisibleTransparentObjects.clear();
	mPreRenderList.clear();
	auto mainCam = gFBEnv->pRenderer->GetCamera();
	auto lightCamera = gFBEnv->pRenderer->GetDirectionalLight(0)->GetCamera();
	auto it = mSpatialObjects.begin(), itEnd = mSpatialObjects.end();
	for (; it!=itEnd; it++)
	{
		if ((*it)->GetObjFlag() & IObject::OF_IGNORE_ME)
			continue;
		bool inserted = false;
		if (!mainCam->IsCulled((*it)->GetBoundingVolumeWorld()))
		{
			IMaterial* pmat = (*it)->GetMaterial();
			if (pmat && pmat->IsTransparent())
			{
				mVisibleTransparentObjects.push_back(*it);
			}
			else
			{
				mVisibleObjectsMain.push_back(*it);
			}
			inserted = true;
		}
		
		if (!lightCamera->IsCulled((*it)->GetBoundingVolumeWorld()))
		{
			mVisibleObjectsLight.push_back((*it));
			inserted = true;
		}
		if (inserted)
			mPreRenderList.push_back((*it));
		
	}

	const fastbird::Vec3& camPos = gFBEnv->pRenderer->GetCamera()->GetPos();
	for (const auto& obj : mPreRenderList)
	{
		const Vec3& objPos = obj->GetPos();
		float dist = (camPos - objPos).Length();
		obj->SetDistToCam(dist);
	}

	std::sort(mVisibleObjectsMain.begin(), mVisibleObjectsMain.end(), 
				[](SpatialObject* a, SpatialObject* b) -> bool
				{
					return a->GetDistToCam() < b->GetDistToCam();
				}
			);

	std::sort(mVisibleObjectsLight.begin(), mVisibleObjectsLight.end(),
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
		auto it = mPreRenderList.begin(), itEnd = mPreRenderList.end();
		for (; it != itEnd; it++)
		{
			(*it)->PreRender();
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
		if (gFBEnv->mRenderPass == RENDER_PASS::PASS_SHADOW)
		{
			for (auto& obj : mVisibleObjectsLight)
			{
				obj->Render();
			}
		}
		else
		{
			for (const auto& l : mListeners)
			{
				l->OnBeforeRenderingOpaques();
			}

			for (auto& obj : mVisibleObjectsMain)
			{
				obj->Render();
			}
		}
	}

	if (mSkyRendering && gFBEnv->mRenderPass != RENDER_PASS::PASS_SHADOW)
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

	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_SHADOW)
	{
		for (const auto& l : mListeners)
		{
			l->OnBeforeRenderingTransparents();
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
	for (auto var : mCloudVolumes)
	{
		var->PreRender();
		var->Render();
	}

	// near side	
	gFBEnv->pRenderer->RestoreRasterizerState();
	gFBEnv->pRenderer->SetRedAlphaMaskAddAddBlend();
	for (auto var : mCloudVolumes)
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

void Scene::AddListener(ISceneListener* listener)
{
	if (ValueNotExistInVector(mListeners, listener))
		mListeners.push_back(listener);
}

void Scene::RemoveListener(ISceneListener* listener)
{
	DeleteValuesInVector(mListeners, listener);
}