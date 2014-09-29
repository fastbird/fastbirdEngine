#include <Engine/StdAfx.h>
#include <Engine/IRenderToTexture.h>
#include <Engine/ICamera.h>
#include <Engine/RenderObjects/SkySphere.h>

#include <CommonLib/Math/GeomUtils.h>

namespace fastbird
{

fastbird::SmartPtr<fastbird::IRenderToTexture> SkySphere::mRT;

ISkySphere* ISkySphere::CreateSkySphere(bool usingSmartPointer)
{
	ISkySphere* newsky = FB_NEW(SkySphere);
	if (!usingSmartPointer)
	{
		newsky->AddRef();		
	}		
	return newsky;
}

void ISkySphere::Delete() // only when not using smart ptr.
{
	this->Release();
}

SkySphere::SkySphere()
: mCurInterpolationTime(0)
, mInterpolating(false)
, mUseAlphaBlend(false)
, mAlpha(1.0f)
{
	BLEND_DESC bdesc;
	SetBlendState(bdesc);
	bdesc.RenderTarget[0].BlendEnable = true;
	bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
	bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
	mAlphaBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
	SetRasterizerState(RASTERIZER_DESC());
	DEPTH_STENCIL_DESC desc;
	desc.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
	SetDepthStencilState(desc);
	assert(mRT && "Call SkySphere::CreateSharedEnvRT first!");
}

SkySphere::~SkySphere()
{
	// no scene is referencing this object since scenes are using smart pointer to hole the sky.
	mScenes.clear();
}

//static 
void SkySphere::CreateSharedEnvRT()
{
	if (!mRT)
	{
		mRT = gFBEnv->pRenderer->CreateRenderToTexture(false);
		mRT->SetColorTextureDesc(1024, 1024, PIXEL_FORMAT_R8G8B8A8_UNORM, true, true, true);
	}
}
//static 
void SkySphere::DeleteSharedEnvRT()
{
	mRT = 0;
}

void SkySphere::SetMaterial(const char* name, int pass /*= RENDER_PASS::PASS_NORMAL*/)
{
	IMaterial* pMat = IMaterial::CreateMaterial(name);
	if (pMat)
	{
		if (pass == RENDER_PASS::PASS_NORMAL)
			mMaterial = pMat;
		else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
			mMaterialOCC = pMat;
		else
			assert(0);
	}
		
}

void SkySphere::SetMaterial(IMaterial* pMat, int pass /*= RENDER_PASS::PASS_NORMAL*/)
{
	if (pass == RENDER_PASS::PASS_NORMAL)
		mMaterial = pMat;
	else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
		mMaterialOCC = pMat;
	else
		assert(0);
}

IMaterial* SkySphere::GetMaterial(int pass /*= RENDER_PASS::PASS_NORMAL*/) const
{
	if (pass == RENDER_PASS::PASS_NORMAL)
		return mMaterial;
	else if (pass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
		return mMaterialOCC;
	else
		assert(0);

	return 0;
}

void SkySphere::PreRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	if (mInterpolating)
	{
		mCurInterpolationTime += gFBEnv->pTimer->GetDeltaTime();
		float normTime = mCurInterpolationTime / mInterpolationTime;
		if (normTime > 1.0f)
		{
			normTime = 1.0f;
			mInterpolating = false;
			//if (!mAlphaBlend)
				//gFBEnv->pRenderer->UpdateEnvMapInNextFrame(this);
		}
			
		for (int i = 0; i < 4; i++)
		{
			mMaterial->SetMaterialParameters(i, Lerp(mMaterialParamCur[i], mMaterialParamDest[i], normTime));
		}		
	}
}

void SkySphere::Render()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;

	if (!mMaterial)
	{
		assert(0);
		return;
	}

	gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gFBEnv->pRenderer->SetInputLayout(0);
	gFBEnv->pRenderer->SetVertexBuffer(0, 0, 0, 0, 0);
	if (mMaterialOCC && gFBEnv->mRenderPass == RENDER_PASS::PASS_GODRAY_OCC_PRE)
	{
		D3DEventMarker mark("SkySphere_OCC");
		mMaterialOCC->Bind(false);
		BindRenderStates();
		gFBEnv->pRenderer->Draw(3, 0);
	}
	else if (gFBEnv->mRenderPass == RENDER_PASS::PASS_NORMAL)
	{
		D3DEventMarker mark("SkySphere");
		mMaterial->Bind(false);
		BindRenderStates();
		if (mUseAlphaBlend)
			mAlphaBlend->Bind();
		gFBEnv->pRenderer->Draw(3, 0);
	}
}

void SkySphere::PostRender()
{
	if (mObjFlag & IObject::OF_HIDE)
		return;
}

void SkySphere::GenerateSphereMesh()
{
	float n, f;
	gFBEnv->pRenderer->GetCamera()->GetNearFar(n, f);
	float radius =  f * 0.95f;
	std::vector<Vec3> pos;
	std::vector<UINT16> index;
	CreateSphereMesh(radius, 16, 16, pos, index, 0, 0);
	assert(!pos.empty() && !index.empty());

	mVB = gFBEnv->pRenderer->CreateVertexBuffer(&pos[0], sizeof(Vec3), pos.size(), 
		BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
	mIB = gFBEnv->pRenderer->CreateIndexBuffer(&index[0], index.size(), INDEXBUFFER_FORMAT_16BIT);
	assert(mVB && mIB);
}

void SkySphere::UpdateEnvironmentMap(const Vec3& origin)
{
	if (!mRT)
		return;
	ITexture* pTexture = mRT->GetRenderTargetTexture();
	mRT->GetScene()->AttachSkySphere(this);
	mRT->GetCamera()->SetPos(origin);
	mRT->GetCamera()->SetFOV(HALF_PI);
	mRT->GetCamera()->SetAspectRatio(1.0f);
	Vec3 dirs[] = {
		Vec3(1, 0, 0), Vec3(-1, 0, 0),
		Vec3(0, 0, 1), Vec3(0, 0, -1),
		Vec3(0, 1, 0), Vec3(0, -1, 0),
	};
	for (int i = 0; i < 6; i++)
	{
		mRT->GetCamera()->SetDir(dirs[i]);
		mRT->Render(i);
	}

	pTexture->GenerateMips();
	gFBEnv->pRenderer->SetEnvironmentTexture(pTexture);
	mRT->GetScene()->DetachSkySphere();
}

void SkySphere::SetInterpolationData(unsigned index, const Vec4& data)
{
	assert(index < 4);
	mMaterialParamDest[index] = data;
}

void SkySphere::PrepareInterpolation(float time)
{
	for (int i = 0; i < 4; i++)
	{
		mMaterialParamCur[i] = mMaterial->GetMaterialParameters(i);
	}

	mInterpolationTime = time;
	mCurInterpolationTime = 0;
	mInterpolating = true;
}

void SkySphere::DetachFromScene()
{
	if (mScenes.empty())
		return;
	if (mUseAlphaBlend)
	{
		mScenes[0]->DetachSkySphereBlend();
	}
	else
	{
		mScenes[0]->DetachSkySphere();
	}
		
}

void SkySphere::SetAlpha(float alpha)
{
	Vec4 param = mMaterial->GetMaterialParameters(3);
	mAlpha = param.w = alpha;
	mMaterial->SetMaterialParameters(3, param);
}
}