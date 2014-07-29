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

using namespace fastbird;

IScene* IScene::CreateScene()
{
	return new Scene();
}

//----------------------------------------------------------------------------
Scene::Scene()
{
	mVisibleObjects.reserve(1000);
	mSkyBox = 0;
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
	mSkyBox = pSkyBox;
}

void Scene::AttachSkySphere(ISkySphere* p)
{
	mSkySphere = p;
}

void Scene::DetachSkySphere()
{
	mSkySphere = 0;
}

//----------------------------------------------------------------------------
void Scene::ToggleSkyRendering()
{
	mSkyRendering = !mSkyRendering;
}

//----------------------------------------------------------------------------
IScene::OBJECTS Scene::QueryVisibleObjects(const Ray3& ray, unsigned limitObject)
{
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
	mVisibleObjects.clear();
	auto it = mSpatialObjects.begin(), itEnd = mSpatialObjects.end();
	for (; it!=itEnd; it++)
	{
		if ( gFBEnv->pRenderer->GetCamera()->IsCulled((*it)->GetBoundingVolumeWorld()) )
			continue;
		mVisibleObjects.push_back(*it);
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

	std::sort(mVisibleObjects.begin(), mVisibleObjects.end(), 
				[](SpatialObject* a, SpatialObject* b) -> bool
				{
					return a->GetDistToCam() < b->GetDistToCam();
				}
			);
}

//----------------------------------------------------------------------------
void Scene::PreRender()
{
	MakeVisibleSet();
	auto it = mVisibleObjects.begin(), itEnd = mVisibleObjects.end();
	for (; it!=itEnd; it++)
	{
		(*it)->PreRender();
	}
}

//----------------------------------------------------------------------------
void Scene::Render()
{
	auto it = mVisibleObjects.begin(), itEnd = mVisibleObjects.end();
	for (; it!=itEnd; it++)
	{
		(*it)->Render();
	}

	if (mSkyRendering)
	{
		if (mSkyBox)
			mSkyBox->Render();
		else if (mSkySphere)
			mSkySphere->Render();
	}
}