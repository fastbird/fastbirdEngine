#include <Engine/StdAfx.h>
#include <Engine/Foundation/Object.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IRenderer.h>
#include <Engine/IScene.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/IInputLayout.h>

using namespace fastbird;

//----------------------------------------------------------------------------
Object::Object()
	: mObjFlag(0)
	, mGameType(-1)
	, mDestructing(false)
	, mGamePtr(0)
{
	mBoundingVolume = BoundingVolume::Create();
	mBoundingVolumeWorld = BoundingVolume::Create();
}

//----------------------------------------------------------------------------
Object::~Object()
{
	mDestructing = true;
	for each(auto scene in mScenes)
	{
		scene->DetachObject(this);
	}
}


void Object::Clone(IObject* cloned) const
{
	Object* object = (Object*)cloned;
	object->mBoundingVolume = mBoundingVolume;
	object->mBoundingVolumeWorld->SetCenter(mBoundingVolumeWorld->GetCenter());
	object->mBoundingVolumeWorld->SetRadius(mBoundingVolumeWorld->GetRadius());
	object->mObjFlag = mObjFlag;
	object->mRasterizerState = mRasterizerState;
	object->mBlendState = mBlendState;
	object->mDepthStencilState = mDepthStencilState;
	object->mGameType = mGameType;
	object->mDestructing = mDestructing;
}
//----------------------------------------------------------------------------
void Object::OnAttachedToScene(IScene* pScene)
{
	if (std::find(mScenes.begin(), mScenes.end(), pScene)==mScenes.end())
	{
		mScenes.push_back(pScene);
	}
}

//----------------------------------------------------------------------------
void Object::OnDetachedFromScene(IScene* pScene)
{
	if (!mDestructing)
		mScenes.erase(std::remove(mScenes.begin(), mScenes.end(), pScene), 
			mScenes.end());
}

//----------------------------------------------------------------------------
void Object::SetMaterial(const char* name)
{
	assert(0 && "Need to implement in the inherited class");
}

void Object::SetMaterial(IMaterial* pMat)
{
	assert(0 && "Need to implement in the inherited class");
}

IMaterial* Object::GetMaterial() const
{
	assert(0 && "Need to implement in the inherited class");
	return 0;
}

//----------------------------------------------------------------------------
void Object::SetRasterizerState(const RASTERIZER_DESC& desc)
{
	mRasterizerState = gFBEnv->pRenderer->CreateRasterizerState(desc);
}

void Object::SetBlendState(const BLEND_DESC& desc)
{
	mBlendState = gFBEnv->pRenderer->CreateBlendState(desc);
}

void Object::SetDepthStencilState(const DEPTH_STENCIL_DESC& desc)
{
	if (gFBEnv && gFBEnv->pRenderer)
		mDepthStencilState = gFBEnv->pRenderer->CreateDepthStencilState(desc);
}

//----------------------------------------------------------------------------
IRasterizerState* Object::GetRasterizerState()
{ 
	return mRasterizerState; 
}
//----------------------------------------------------------------------------
IBlendState* Object::GetBlenderState() 
{ 
	return mBlendState; 
}
//----------------------------------------------------------------------------
IDepthStencilState* Object::GetDepthStencilState() 
{ 
	return mDepthStencilState; 
}

//----------------------------------------------------------------------------
void Object::BindRenderStates(unsigned stencilRef/*=0*/)
{
	if (mRasterizerState)
	{
		mRasterizerState->Bind();
	}

	if (mBlendState)
	{
		mBlendState->Bind();
	}

	if (mDepthStencilState)
	{
		mDepthStencilState->Bind(stencilRef);
	}
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

//----------------------------------------------------------------------------
void Object::SetShow(bool show)
{
	ModifyObjFlag(IObject::OF_HIDE, false);
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

void Object::DetachFromScene()
{
	gFBEnv->pEngine->GetScene()->DetachObject(this);
}

bool Object::IsAttached(IScene* pScene) const
{
	FB_FOREACH(it, mScenes)
	{
		if (pScene==(*it))
		{
			return true;
		}
	}
	return false;
}