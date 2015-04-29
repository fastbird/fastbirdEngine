// not using precompiled header.
// this file will be compiled in the engine project and the game project for inheritance.
#include <CommonLib/Config.h>

#include <CommonLib/CommonLib.h>
#include <assert.h>
#include <vector>
#include <CommonLib/Math/fbMath.h>
#include <Engine/Object.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/IScene.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/IEngine.h>

using namespace fastbird;

//----------------------------------------------------------------------------
Object::Object()
	: mObjFlag(0)
	, mGameType(-1)
	, mGamePtr(0)
{
	mBoundingVolume = BoundingVolume::Create();
	mBoundingVolumeWorld = BoundingVolume::Create();
}

//----------------------------------------------------------------------------
Object::~Object()
{
	DetachFromScene(true);
}


void Object::Clone(IObject* cloned) const
{
	Object* object = (Object*)cloned;
	object->mBoundingVolume = mBoundingVolume;
	object->mBoundingVolumeWorld->SetCenter(mBoundingVolumeWorld->GetCenter());
	object->mBoundingVolumeWorld->SetRadius(mBoundingVolumeWorld->GetRadius());
	object->mObjFlag = mObjFlag;
	object->mGameType = mGameType;
}
//----------------------------------------------------------------------------
void Object::OnAttachedToScene(IScene* pScene)
{
	assert(pScene != 0);
	if (ValueNotExistInVector(mScenes, pScene))
	{
		mScenes.push_back(pScene);
	}
}

//----------------------------------------------------------------------------
void Object::OnDetachedFromScene(IScene* pScene)
{
	assert(pScene != 0);
	DeleteValuesInVector(mScenes, pScene);
}

//----------------------------------------------------------------------------
void Object::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
{
	assert(0 && "Need to implement in the inherited class");
}

void Object::SetMaterial(IMaterial* pMat, int pass/* = RENDER_PASS::PASS_NORMAL*/)
{
	assert(0 && "Need to implement in the inherited class");
}

IMaterial* Object::GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
{
	//"Need to implement in the inherited class"
	//or
	// no material
	assert(0);
	// Even you don't have material, implementing this in the inherited class
	return 0;
}

//----------------------------------------------------------------------------
void Object::SetObjFlag(unsigned flag)
{
	mObjFlag = flag;
}

//----------------------------------------------------------------------------
unsigned Object::GetObjFlag() const
{
	return mObjFlag;
}

//----------------------------------------------------------------------------
void Object::ModifyObjFlag(unsigned flag, bool enable)
{
	if (enable)
	{
		mObjFlag |= flag;
	}
	else
	{
		mObjFlag = mObjFlag  & ~flag;
	}
}

bool Object::HasObjFlag(unsigned flag)
{
	return (mObjFlag & flag) != 0;
}

//----------------------------------------------------------------------------
void Object::SetShow(bool show)
{
	ModifyObjFlag(IObject::OF_HIDE, !show);
}

bool Object::GetShow() const
{
	return mObjFlag & IObject::OF_HIDE ? false : true;
}

//----------------------------------------------------------------------------
void Object::SetGameType(int type)
{
	mGameType = type;
}

int Object::GetGameType() const
{
	return mGameType;
}

void Object::SetGamePtr(void* ptr)
{
	mGamePtr = ptr;
}

void* Object::GetGamePtr() const
{
	return mGamePtr;
}

void Object::SetRadius(float r)
{
	mBoundingVolume->SetRadius(r);
	mBoundingVolumeWorld->SetRadius(r);
}

void Object::AttachToScene()
{
	IScene* pScene = gFBEnv->pEngine->GetScene();
	if (!IsAttached(pScene))
		pScene->AttachObject(this);
}

void Object::DetachFromScene(bool includingRtt)
{
	unsigned limit = mScenes.size();
	unsigned count = 0;
	for (unsigned i = 0; i < mScenes.size() && count < limit;)
	{
		auto scene = mScenes[i];
		if (includingRtt)
		{
			scene->DetachObject(this);
		}
		else
		{
			if (!scene->IsRttScene())
			{
				scene->DetachObject(this);
			}
			else
			{
				++i;
			}
		}
		++count;
	}
}

bool Object::IsAttached(IScene* pScene) const
{
	if (!pScene)
	{
		return !mScenes.empty();
	}
	return !ValueNotExistInVector(mScenes, pScene);
}

//---------------------------------------------------------------------------
void Object::RegisterEventListener(IObjectEventListener* listener)
{
	assert(std::find(mEventListener.begin(), mEventListener.end(), listener) == mEventListener.end());
	mEventListener.push_back(listener);
}

//---------------------------------------------------------------------------
void Object::RemoveEventListener(IObjectEventListener* listener)
{
	mEventListener.erase(std::remove(mEventListener.begin(), mEventListener.end(), listener), mEventListener.end());
}