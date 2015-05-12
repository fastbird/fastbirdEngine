#include <Engine/StdAfx.h>
#include <Engine/Scene.h>
#include <Engine/IObject.h>
#include <Engine/SpatialObject.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/ICamera.h>
#include <Engine/ISkyBox.h>
#include <Engine/ISkySphere.h>
#include <Engine/IMeshObject.h>
#include <Engine/ISceneListener.h>
#include <Engine/ILight.h>
#include <Engine/FBCollisionShape.h>
#include <Engine/Renderer.h>
#include <Engine/ParticleRenderObject.h>
#include <Engine/IRenderTarget.h>
#include <Engine/Camera.h>
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
, mRttScene(false)
, mLastPreRenderFrame(-1)
{
	mVisibleObjectsMain.reserve(1000);
	mVisibleObjectsLight.reserve(1000);
	mPreRenderList.reserve(1000);
	mVisibleTransparentObjects.reserve(1000);
	mSkyBox = 0;

	mWindDir = Vec3(1, 0, 0);
	mWindVelocity = 0.0f;
	mWindVector = mWindDir * mWindVelocity;

	// Light
	for (int i = 0; i < 2; ++i)
	{
		mDirectionalLight[i] = ILight::CreateLight(ILight::LIGHT_TYPE_DIRECTIONAL);
		mDirectionalLight[i]->SetIntensity(1.0f);
	}

	mDirectionalLight[0]->SetPosition(Vec3(-3, 1, 1));
	mDirectionalLight[0]->SetDiffuse(Vec3(1, 1, 1));
	mDirectionalLight[0]->SetSpecular(Vec3(1, 1, 1));

	mDirectionalLight[1]->SetPosition(Vec3(3, 1, -1));
	mDirectionalLight[1]->SetDiffuse(Vec3(0.8f, 0.4f, 0.1f));
	mDirectionalLight[1]->SetSpecular(Vec3(0, 0, 0));
}

//----------------------------------------------------------------------------
Scene::~Scene()
{
	DetachSkySphere();
	DetachSkySphereBlend();
	
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
	if (ValueNotExistInVector(mSpatialObjects, pSpatialObject))
	{
		mSpatialObjects.push_back(pSpatialObject);
		pSpatialObject->OnAttachedToScene(this);
		return true;
	}   

    return false;
}

//----------------------------------------------------------------------------
bool Scene::DetachObject(SpatialObject* pSpatialObject)
{
	LOCK_CRITICAL_SECTION lock(mSpatialObjectsCS);
    auto it = std::find(mSpatialObjects.begin(), mSpatialObjects.end(), pSpatialObject);
    if (it != mSpatialObjects.end())
    {
		mSpatialObjects.erase(it);
        pSpatialObject->OnDetachedFromScene(this);        
        return true;
    }
    
    return false;
}

//----------------------------------------------------------------------------
bool Scene::AttachObject(IObject* pObject)
{
	auto spatial = dynamic_cast<SpatialObject*>(pObject);
	if (spatial)
	{
		return AttachObject(spatial);
	}
	if (ValueNotExistInVector(mObjects, pObject))
	{
		mObjects.push_back(pObject);
		pObject->OnAttachedToScene(this);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
bool Scene::DetachObject(IObject* pObject)
{
	auto spatial = dynamic_cast<SpatialObject*>(pObject);
	if (spatial)
	{
		return DetachObject(spatial);
	}
    auto it = std::find(mObjects.begin(), mObjects.end(), pObject);
    if (it!= mObjects.end())
    {
		mObjects.erase(it);
        pObject->OnDetachedFromScene(this);        
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
				const FBCollisionShape* cs = pObj->GetCollisionShape(i);
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
	auto const renderer = gFBEnv->pRenderer;	
	auto mainCam = renderer->GetCamera();
	auto curTarget = renderer->GetCurRendrTarget();
	assert(curTarget);
	auto lightCamera = curTarget->GetLightCamera();
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
		
		if (lightCamera && !lightCamera->IsCulled((*it)->GetBoundingVolumeWorld()))
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

	for (auto l : mListeners)
	{
		l->OnAfterMakeVisibleSet(this);
	}
}

//----------------------------------------------------------------------------
void Scene::PreRender()
{
	bool runnedAlready = mLastPreRenderFrame == gFBEnv->mFrameCounter;		

	if (!mSkipSpatialObjects)
	{
		auto cam = gFBEnv->pRenderer->GetCamera();
		assert(cam);
		
		auto it = mLastPreRenderFramePerCam.Find(cam);
		if (it != mLastPreRenderFramePerCam.end() && it->second == gFBEnv->mFrameCounter)
			return;
		mLastPreRenderFramePerCam[cam] = gFBEnv->mFrameCounter;

		MakeVisibleSet();

		if (!runnedAlready)
		{
			auto objIt = mPreRenderList.begin(), objItEnd = mPreRenderList.end();
			for (; objIt != objItEnd; objIt++)
			{
				(*objIt)->PreRender();
			}
		}
	}	
	if (!runnedAlready)
	{
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
					if (mSkySphereBlend && mSkySphereBlend->GetAlpha() != 0.f)
						mSkySphereBlend->PreRender();
				}

			}
			if (mSkyBox)
			{
				mSkyBox->PreRender();
			}
		}

		FB_FOREACH(it, mObjects)
		{
			(*it)->PreRender();
		}

		mLastPreRenderFrame = gFBEnv->mFrameCounter;
	}
}

//----------------------------------------------------------------------------
void Scene::Render()
{
	auto const renderer = (Renderer*)gFBEnv->pRenderer;
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
				l->OnBeforeRenderingOpaques(this);
			}

			for (auto& obj : mVisibleObjectsMain)
			{
				obj->Render();
			}
		}
	}

	if (!(gFBEnv->mRenderPass == RENDER_PASS::PASS_SHADOW || gFBEnv->mRenderPass == RENDER_PASS::PASS_DEPTH))
	{
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
					mSkySphereBlend->Render();
				else
				{
					mSkySphere->Render();
					if (mSkySphereBlend && mSkySphereBlend->GetAlpha() != 0.f)
						mSkySphereBlend->Render();
				}
			}

		}

		for (const auto& l : mListeners)
			l->OnBeforeRenderingTransparents(this);

		bool mainRt = renderer->IsMainRenderTarget();
		if (mainRt && gFBEnv->mRenderPass == RENDER_PASS::PASS_NORMAL)
		{
			renderer->RenderGeoms();
		}

		renderer->GetCurRendrTarget()->BindDepthTexture(true);
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
}

void Scene::RemoveClouds()
{
	mCloudVolumes.clear();
}

void Scene::RenderCloudVolumes()
{	
	if (!mDrawClouds)
		return;

	auto const renderer = gFBEnv->_pInternalRenderer;
	// render obstacles
	renderer->LockDepthStencilState();
	renderer->SetBlueMask();
	Render();
	renderer->UnlockDepthStencilState();
	renderer->SetDepthWriteShaderCloud();
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
	gFBEnv->pRenderer->RestoreRasterizerState();
	gFBEnv->pRenderer->RestoreBlendState();
	gFBEnv->pRenderer->RestoreDepthStencilState();
}

void Scene::SetFogColor(const Color& c)
{
	mFogColor = c;
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

const std::vector<SpatialObject*>& Scene::GetVisibleSpatialList() const
{
	return mVisibleObjectsMain;
}

unsigned Scene::GetNumSpatialObjects() const
{
	return mSpatialObjects.size();
}

void Scene::PrintSpatialObject()
{
	Log("void Scene::PrintSpatialObject(): num:  %u", mSpatialObjects.size());
	for (auto it : mSpatialObjects)
	{
		auto name = it->GetName();
		if (name && strlen(name) != 0)
		{
			Log("Spatial Object Name = %s", name);
		}
		else
		{
			int gameType = it->GetGameType();
			Log("GameType = %d");
			auto mat = it->GetMaterial();
			if (mat && strlen(mat->GetName()) != 0)
				Log("		Material = %s", mat->GetName());
			ParticleRenderObject* pro = dynamic_cast<ParticleRenderObject*>(it);
			if (pro)
			{
				Log("		It's particle object.");
			}			
		}
	}
}

void Scene::SetRttScene(bool set)
{
	mRttScene = set;
}



void Scene::SetLightToRenderer()
{
	auto const renderer = gFBEnv->_pInternalRenderer;
	for (int i = 0; i < 2; i++)
	{
		if (mDirectionalLight[i])
			renderer->SetDirectionalLight(mDirectionalLight[i], i);
	}
}

ILight* Scene::GetLight(unsigned idx)
{
	assert(idx < 2);
	if (!mDirectionalLight[idx])
	{
		mDirectionalLight[idx] = ILight::CreateLight(ILight::LIGHT_TYPE_DIRECTIONAL);
		mDirectionalLight[idx]->SetPosition(Vec3(1, 1, 1));
		mDirectionalLight[idx]->SetDiffuse(Vec3(1, 1, 1));
		mDirectionalLight[idx]->SetSpecular(Vec3(1, 1, 1));
		mDirectionalLight[idx]->SetIntensity(1.0f);
	}

	return mDirectionalLight[idx];
}