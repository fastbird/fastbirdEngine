#include <Engine/StdAfx.h>
#include <Engine/IRenderTarget.h>
#include <Engine/ICamera.h>
#include <Engine/SkySphere.h>
#include <Engine/Renderer.h>
#include <Engine/ILight.h>

#include <CommonLib/Math/GeomUtils.h>

namespace fastbird
{

fastbird::SmartPtr<fastbird::IRenderTarget> SkySphere::mRT;

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
	bdesc.RenderTarget[0].BlendEnable = true;
	bdesc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
	bdesc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
	mAlphaBlend = gFBEnv->pRenderer->CreateBlendState(bdesc);
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
		RenderTargetParam param;
		param.mEveryFrame = false;
		param.mSize = Vec2I(ENV_SIZE, ENV_SIZE);
		param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
		param.mShaderResourceView = true;
		param.mMipmap = true;
		param.mCubemap = true;
		param.mWillCreateDepth = false;
		param.mUsePool = true;
		mRT = gFBEnv->pRenderer->CreateRenderTarget(param);
		mRT->CreateScene();
		mRT->GetRenderPipeline().SetMinimum();
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
			if (!mAlphaBlend)
				gFBEnv->pRenderer->UpdateEnvMapInNextFrame(this);
		}
			
		for (int i = 0; i < 5; i++)
		{
			mMaterial->SetMaterialParameters(i, Lerp(mMaterialParamCur[i], mMaterialParamDest[i], normTime));
			if (mMaterialOCC)
			{
				mMaterialOCC->SetMaterialParameters(i, Lerp(mMaterialParamCur[i], mMaterialParamDest[i], normTime));
			}
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
		gFBEnv->pRenderer->Draw(3, 0);
	}
	else if (gFBEnv->mRenderPass == RENDER_PASS::PASS_NORMAL)
	{
		D3DEventMarker mark("SkySphere");
		mMaterial->Bind(false);
		if (mUseAlphaBlend)
		{
			mAlphaBlend->Bind();
			/*Log("Material0 = %s", StringConverter::toString(mMaterial->GetMaterialParameters(0)).c_str());
			Log("Material1 = %s", StringConverter::toString(mMaterial->GetMaterialParameters(1)).c_str());
			Log("Material2 = %s", StringConverter::toString(mMaterial->GetMaterialParameters(2)).c_str());
			Log("Material3 = %s", StringConverter::toString(mMaterial->GetMaterialParameters(3)).c_str());
			Log("Material4 = %s", StringConverter::toString(mMaterial->GetMaterialParameters(4)).c_str());*/
		}
			
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
	{
		Error("void SkySphere::UpdateEnvironmentMap(const Vec3& origin) : No mRT");
		return;
	}

	auto const renderer = gFBEnv->_pInternalRenderer;

	ITexture* pTexture = mRT->GetRenderTargetTexture();
	auto dest = mRT->GetScene()->GetLight(0);
	auto src = renderer->GetMainRenderTarget()->GetScene()->GetLight(0);
	dest->CopyLight(src);

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
	// this is for unbind the environment map from the output slot.
	renderer->GetMainRenderTarget()->BindTargetOnly(false);

	pTexture->GenerateMips();
	pTexture->SaveToFile("environment.dds");
	// for bight test.
	//ITexture* textureFile = gFBEnv->pRenderer->CreateTexture("data/textures/brightEnv.jpg");
	GenerateRadianceCoef(pTexture);
	renderer->SetEnvironmentTexture(pTexture);
	renderer->UpdateRadConstantsBuffer(mIrradCoeff);
	mRT->GetScene()->DetachSkySphere();
}

static Vec3 NormalFromCubePixelCoord(int face, int w, int h, float halfWidth)
{
	Vec3 n;
	switch (face)
	{
	case 0:
		n.x = halfWidth;
		n.y = halfWidth - w;
		n.z = halfWidth - h;
		break;
	case 1:
		n.x = -halfWidth;
		n.y = w - halfWidth;
		n.z = halfWidth - h;
		break;
	case 2: // up
		n.x = w - halfWidth;
		n.y = h - halfWidth;
		n.z = halfWidth;
		break;
	case 3: // down
		n.x = w - halfWidth;
		n.y = halfWidth - h;
		n.z = -halfWidth;
		break;
	case 4: // front
		n.x = w - halfWidth;
		n.y = halfWidth;
		n.z = halfWidth - h;
		break;
	case 5: // back
		n.x = halfWidth - w;
		n.y = -halfWidth;
		n.z = halfWidth - h;
		break;
	}
	return n.NormalizeCopy();
}
void SkySphere::GenerateRadianceCoef(ITexture* pTex)
{
	int maxLod = (int)log2((float)ENV_SIZE);
	
	const float basisCoef[5] = { 0.282095f,
		0.488603f,
		1.092548f,
		0.315392f,
		0.546274f };

	const int usingLod = 2; // if ENV_SIZE == 1024, we are using 256 size.
	const int width = (int)(ENV_SIZE / pow(2, usingLod));
	float halfWidth = width / 2.f;
	SmartPtr<ITexture> pStaging = gFBEnv->pRenderer->CreateTexture(0, width, width, PIXEL_FORMAT_R8G8B8A8_UNORM,
		BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_CUBE_MAP);
	for (int i = 0; i < 6; i++)
	{
		unsigned subResource = i * (maxLod+1) + usingLod;
		pTex->CopyToStaging(pStaging, i, 0, 0, 0, subResource, 0);
	}
	pStaging->SaveToFile("envSub.dds");

	// prefiltering
	float lightCoef[3][9];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			lightCoef[i][j] = 0;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		unsigned subResource = i;
		MapData data = pStaging->Map(subResource, MAP_TYPE_READ, MAP_FLAG_NONE);
		if (data.pData)
		{
			DWORD* colors = (DWORD*)data.pData;
			DWORD* row, *pixel;
			for (int h = 0; h < width; h++)
			{
				row = colors + width * h;
				for (int w = 0; w < width; w++)
				{
					
					Vec3 n = NormalFromCubePixelCoord(i, w, h, halfWidth);
					
					pixel = row + w;
					Color::RGBA* rgba256 = (Color::RGBA*)pixel;
					Vec3 rgb(rgba256->r / 255.0f, rgba256->g / 255.0f, rgba256->b / 255.0f);
					// 00
					lightCoef[0][0] += rgb.x * basisCoef[0];
					lightCoef[1][0] += rgb.y * basisCoef[0];
					lightCoef[2][0] += rgb.z * basisCoef[0];

					// 1-1
					float yC = (basisCoef[1] * n.y);
					lightCoef[0][1] += rgb.x * yC;
					lightCoef[1][1] += rgb.y * yC;
					lightCoef[2][1] += rgb.z * yC;

					// 10
					yC = (basisCoef[1] * n.z);
					lightCoef[0][2] += rgb.x * yC;
					lightCoef[1][2] += rgb.y * yC;
					lightCoef[2][2] += rgb.z * yC;

					// 11
					yC = (basisCoef[1] * n.x);
					lightCoef[0][3] += rgb.x * yC;
					lightCoef[1][3] += rgb.y * yC;
					lightCoef[2][3] += rgb.z * yC;

					// 2-2
					yC = (basisCoef[2] * n.x*n.y);
					lightCoef[0][4] += rgb.x * yC;
					lightCoef[1][4] += rgb.y * yC;
					lightCoef[2][4] += rgb.z * yC;

					// 2-1
					yC = (basisCoef[2] * n.y*n.z);
					lightCoef[0][5] += rgb.x * yC;
					lightCoef[1][5] += rgb.y * yC;
					lightCoef[2][5] += rgb.z * yC;

					// 20
					yC = (basisCoef[3] * (3.0f*n.z*n.z - 1.f));
					lightCoef[0][6] += rgb.x * yC;
					lightCoef[1][6] += rgb.y * yC;
					lightCoef[2][6] += rgb.z * yC;

					// 21
					yC = (basisCoef[2] * (n.x*n.z));
					lightCoef[0][7] += rgb.x * yC;
					lightCoef[1][7] += rgb.y * yC;
					lightCoef[2][7] += rgb.z * yC;

					// 22
					yC = (basisCoef[4] * (n.x*n.x - n.y*n.y));
					lightCoef[0][8] += rgb.x * yC;
					lightCoef[1][8] += rgb.y * yC;
					lightCoef[2][8] += rgb.z * yC;
				}
			}
		}
		pStaging->Unmap(subResource);
	}

	float avg = 1.0f / (width*width * 6);	
	for (int i = 0; i < 9; i++)
	{
		lightCoef[0][i] *= avg;
		lightCoef[1][i] *= avg;
		lightCoef[2][i] *= avg;		
		mIrradCoeff[i] = Vec4(lightCoef[0][i], lightCoef[1][i], lightCoef[2][i], 1);
	}

	//const float sqrtPi = sqrtf(PI);
	//const float fC[5] = { 1.f / (2.f*sqrtPi),
	//	sqrt(3.0f) / (3.f*sqrtPi),
	//	sqrt(15.f) / (8.f*sqrtPi),
	//	sqrt(5.f) / (16.0f*sqrtPi),
	//	0.5f * (sqrt(15.f) / (8.f*sqrtPi)),
	//};

	//Vec4 vLightYCoeff[3];
	//for (int ic = 0; ic < 3; ic++)
	//{
	//	vLightYCoeff[ic].x = -fC[1] * lightCoef[ic][3];
	//	vLightYCoeff[ic].y = -fC[1] * lightCoef[ic][1];
	//	vLightYCoeff[ic].z = fC[1] * lightCoef[ic][2];
	//	vLightYCoeff[ic].w = fC[0] * lightCoef[ic][0] - fC[3] * lightCoef[ic][6];
	//}

	//mIrradConsts[0] = vLightYCoeff[0]; // Ar
	//mIrradConsts[1] = vLightYCoeff[1]; // Ag
	//mIrradConsts[2] = vLightYCoeff[2]; // Ab

	//for (int ic = 0; ic < 3; ic++)
	//{
	//	vLightYCoeff[ic].x = fC[2] * lightCoef[ic][4];
	//	vLightYCoeff[ic].y = -fC[2] * lightCoef[ic][5];
	//	vLightYCoeff[ic].z = 3.0f * fC[3] * lightCoef[ic][6];
	//	vLightYCoeff[ic].w = -fC[2] * lightCoef[ic][7];
	//}

	//mIrradConsts[3] = vLightYCoeff[0]; // Br
	//mIrradConsts[4] = vLightYCoeff[1]; // Bg
	//mIrradConsts[5] = vLightYCoeff[2]; // Bb

	//mIrradConsts[6].x = fC[4] * lightCoef[0][8];
	//mIrradConsts[6].y = fC[4] * lightCoef[1][8];
	//mIrradConsts[6].z = fC[4] * lightCoef[2][8];
	//mIrradConsts[6].w = 1.0f;
}

void SkySphere::SetInterpolationData(unsigned index, const Vec4& data)
{
	assert(index < 5);
	if (index == 2)
	{
		mMaterialParamDest[index] = data;
		mMaterialParamDest[index].w = mAlpha;
	}
	else
	{
		mMaterialParamDest[index] = data;
	}
}

void SkySphere::PrepareInterpolation(float time, ISkySphere* startFrom)
{
	IMaterial* srcMaterial = mMaterial;
	if (startFrom)
		srcMaterial = startFrom->GetMaterial();
	for (int i = 0; i < 5; i++)
	{
		mMaterialParamCur[i] = srcMaterial->GetMaterialParameters(i);
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
	mMaterialParamDest[3].w = alpha;
}
}