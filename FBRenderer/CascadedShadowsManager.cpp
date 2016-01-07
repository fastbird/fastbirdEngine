#include "stdafx.h"
#include "FBMathLib/Frustum.h"
#include "CascadedShadowsManager.h"
#include "Camera.h"
#include "Texture.h"
#include "Renderer.h"
#include "RendererOptions.h"
#include "ResourceProvider.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;
#define MAX_CASCADES 8
class CascadedShadowsManager::Impl{
public:
	unsigned mRenderTargetId;
	Vec2I mRenderTargetSize;
	CameraPtr mLightCamera;
	TexturePtr mShadowMap;
	std::unique_ptr<Viewport[]> mViewports;
	float mCascadePartitionsZeroToOne[MAX_CASCADES]; // Values are 0 to 100 and represent a percent of the frstum	
	Mat44 mShadowProj[MAX_CASCADES];
	float mCascadePartitionsFrustum[MAX_CASCADES]; // End position
	struct OrthogonalData{
		float mWidth;
		float mHeight;
		float mNear;
		float mFar;
	};
	OrthogonalData mOrthogonalData[MAX_CASCADES];

	//---------------------------------------------------------------------------
	Impl(unsigned renderTargetId, const Vec2I& renderTargetSize)
		: mRenderTargetId(renderTargetId)
		, mRenderTargetSize(renderTargetSize)
	{
		mCascadePartitionsZeroToOne[0] = 0.05f;
		mCascadePartitionsZeroToOne[1] = 0.15f;
		mCascadePartitionsZeroToOne[2] = 0.60f;
		mCascadePartitionsZeroToOne[3] = 1.f;
		mCascadePartitionsZeroToOne[4] = 1.f;
		mCascadePartitionsZeroToOne[5] = 1.f;
		mCascadePartitionsZeroToOne[6] = 1.f;
		mCascadePartitionsZeroToOne[7] = 1.f;

		for (int i = 0; i < MAX_CASCADES; ++i){
			mCascadePartitionsFrustum[i] = 0.0f;
		}

		auto& renderer = Renderer::GetInstance();
		auto cmd = renderer.GetRendererOptions();
		mLightCamera = Camera::Create();
		mLightCamera->SetOrthogonal(true);
	}

	void CreateViewports(){
		auto options = Renderer::GetInstance().GetRendererOptions();
		mViewports.reset();
		mViewports = std::unique_ptr<Viewport[]>(new Viewport[options->r_ShadowCascadeLevels]);
		for (int i = 0; i < options->r_ShadowCascadeLevels; ++i){
			mViewports[i].mWidth = (float)options->r_ShadowMapSize;
			mViewports[i].mHeight = (float)options->r_ShadowMapSize;
			mViewports[i].mMaxDepth = 1.0f;
			mViewports[i].mMinDepth = 0.f;
			mViewports[i].mTopLeftX = (float)(options->r_ShadowMapSize * i);
			mViewports[i].mTopLeftY = 0;
		}
	}

	void CreateShadowMap(){
		auto& renderer = Renderer::GetInstance();
		auto cmd = renderer.GetRendererOptions();

		mShadowMap = renderer.CreateTexture(0,
			cmd->r_ShadowMapSize * cmd->r_ShadowCascadeLevels, cmd->r_ShadowMapSize,
			PIXEL_FORMAT_D32_FLOAT, BUFFER_USAGE_DEFAULT,
			BUFFER_CPU_ACCESS_NONE, TEXTURE_TYPE_DEPTH_STENCIL_SRV);
		assert(mShadowMap);		
		mShadowMap->SetDebugName(FormatString("rt%u_%u_%u_ShadowMap", mRenderTargetId,
			cmd->r_ShadowMapSize* cmd->r_ShadowCascadeLevels, cmd->r_ShadowMapSize).c_str());
	}

	//--------------------------------------------------------------------------------------
	// This function takes the camera's projection matrix and returns the 8
	// points that make up a view frustum.
	// The frustum is scaled to fit within the Begin and End interval paramaters.
	//--------------------------------------------------------------------------------------
	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin,
		float fCascadeIntervalEnd,
		const Frustum& frustum,
		Vec3 pvCornerPointsWorld[8])
	{
		pvCornerPointsWorld[0] = Vec3(frustum.mRightSlope * fCascadeIntervalBegin, 
			fCascadeIntervalBegin, frustum.mTopSlope * fCascadeIntervalBegin); // right top
		pvCornerPointsWorld[1] = Vec3(frustum.mLeftSlope * fCascadeIntervalBegin,
			fCascadeIntervalBegin, frustum.mTopSlope * fCascadeIntervalBegin);// left top
		pvCornerPointsWorld[2] = Vec3(frustum.mLeftSlope * fCascadeIntervalBegin,
			fCascadeIntervalBegin, frustum.mBottomSlope * fCascadeIntervalBegin);// left bottom
		pvCornerPointsWorld[3] = Vec3(frustum.mRightSlope * fCascadeIntervalBegin,
			fCascadeIntervalBegin, frustum.mBottomSlope * fCascadeIntervalBegin); // right bottom

		pvCornerPointsWorld[4] = Vec3(frustum.mRightSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mTopSlope * fCascadeIntervalEnd); // right top
		pvCornerPointsWorld[5] = Vec3(frustum.mLeftSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mTopSlope * fCascadeIntervalEnd);// left top
		pvCornerPointsWorld[6] = Vec3(frustum.mLeftSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mBottomSlope * fCascadeIntervalEnd);// left bottom
		pvCornerPointsWorld[7] = Vec3(frustum.mRightSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mBottomSlope * fCascadeIntervalEnd); // right bottom
		for (int i = 0; i < 8; ++i){
			pvCornerPointsWorld[i] = frustum.mOrientation * pvCornerPointsWorld[i];
			pvCornerPointsWorld[i] += frustum.mOrigin;
		}
	}

	void CreateAABBPoints(Vec3 sceneAABBPointsLightSpace[8], const AABB& sceneAABB){
		//This map enables us to use a for loop and do vector math.
		static const Vec3 vExtentsMap[] =
		{
			{ 1.0f, 1.0f, -1.0f },
			{ -1.0f, 1.0f, -1.0f },
			{ 1.0f, -1.0f, -1.0f },
			{ -1.0f, -1.0f, -1.0f },
			{ 1.0f, 1.0f, 1.0f },
			{ -1.0f, 1.0f, 1.0f },
			{ 1.0f, -1.0f, 1.0f },
			{ -1.0f, -1.0f, 1.0f }
		};

		auto extents = sceneAABB.GetExtents();
		auto center = sceneAABB.GetCenter();
		for (INT index = 0; index < 8; ++index)
		{
			sceneAABBPointsLightSpace[index] = vExtentsMap[index] * extents + center;
		}
	}

	void UpdateFrame(CameraPtr viewerCamera, const Vec3& lightDir,
		const AABB& sceneAABB){
		mLightCamera->SetPosition(lightDir * 400.f);
		mLightCamera->SetDirection(-lightDir);


		auto viewerCameraInvView = viewerCamera->GetMatrix(ICamera::InverseView);
		auto lightView = mLightCamera->GetMatrix(ICamera::View);	

		Vec3 sceneAABBPointsLightSpace[8];
		// This function simply converts the center and extents of an AABB into 8 points
		CreateAABBPoints(sceneAABBPointsLightSpace, sceneAABB);
		// Transform the scene AABB to Light space.
		for (int index = 0; index < 8; ++index)
		{
			sceneAABBPointsLightSpace[index] =
				lightView * sceneAABBPointsLightSpace[index];
		}

		FLOAT fFrustumIntervalBegin, fFrustumIntervalEnd;
		float viewerNear, viewerFar;
		viewerCamera->GetNearFar(viewerNear, viewerFar);
		auto fCameraNearFarRange = viewerFar - viewerNear;

		Vec3 vWorldUnitsPerTexel(0, 0, 0);
		auto& renderer = Renderer::GetInstance();
		auto options = renderer.GetRendererOptions();
		// Loop over the cascades to calculate the orthographic projection for each cascade.
		for (INT iCascadeIndex = 0; 
			iCascadeIndex < options->r_ShadowCascadeLevels; 
			++iCascadeIndex)
		{
			// Calculate the interval of the View Frustum that this cascade covers. 
			// We measure the interval the cascade covers as a Min and Max distance 
			// along the Z Axis.
			
			// Because we want to fit the orthogrpahic projection tightly around the 
			// Cascade, we set the Mimiumum cascade value to the previous Frustum 
			// end Interval
			if (iCascadeIndex == 0) 
				fFrustumIntervalBegin = 0.0f;
			else 
				fFrustumIntervalBegin = mCascadePartitionsZeroToOne[iCascadeIndex - 1];
			

			// Scale the intervals between 0 and 1. They are now percentages that we can scale with.
			fFrustumIntervalEnd = (FLOAT)mCascadePartitionsZeroToOne[iCascadeIndex];			
			fFrustumIntervalBegin = fFrustumIntervalBegin * fCameraNearFarRange;
			fFrustumIntervalEnd = fFrustumIntervalEnd * fCameraNearFarRange;
			Vec3 vFrustumPoints[8];
			// This function takes the began and end intervals along with the projection matrix and returns the 8
			// points that repreresent the cascade Interval
			CreateFrustumPointsFromCascadeInterval(fFrustumIntervalBegin, fFrustumIntervalEnd,
				viewerCamera->GetFrustum(), vFrustumPoints);

			Vec3 vLightCameraOrthographicMin(FLT_MAX, FLT_MAX, FLT_MAX);  // light space frustrum aabb 
			Vec3 vLightCameraOrthographicMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			Vec3 vTempTranslatedCornerPoint;
			// This next section of code calculates the min and max values for the orthographic projection.
			for (int icpIndex = 0; icpIndex < 8; ++icpIndex)
			{
				// Transform the point from world space to Light Camera Space.
				vTempTranslatedCornerPoint = lightView * vFrustumPoints[icpIndex];
				
				// Find the closest point.
				vLightCameraOrthographicMin = Min(
					vTempTranslatedCornerPoint , vLightCameraOrthographicMin);
				vLightCameraOrthographicMax = Max(
					vTempTranslatedCornerPoint, vLightCameraOrthographicMax);
			}
			
			// This code removes the shimmering effect along the edges of shadows due 
			// to the light changing to fit the camera.
			// We calculate a looser bound based on the size of the PCF blur.  
			// This ensures us that we're sampling within the correct map.
			float fScaleDuetoBlureAMT = (options->r_ShadowMapPCFBlurSize * 2.f + 1.f)
				/ (float)options->r_ShadowMapSize;
			Vec3 vScaleDuetoBlureAMT(fScaleDuetoBlureAMT, 0.f, fScaleDuetoBlureAMT);

			float fNormalizeByBufferSize = (1.0f / (float)options->r_ShadowMapSize);
			Vec3 vNormalizeByBufferSize(fNormalizeByBufferSize, 0.f, fNormalizeByBufferSize);

			// We calculate the offsets as a percentage of the bound.
			Vec3 vBoarderOffset = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			vBoarderOffset *= 0.5f;
			vBoarderOffset *= vScaleDuetoBlureAMT;
			vLightCameraOrthographicMax += vBoarderOffset;
			vLightCameraOrthographicMin -= vBoarderOffset;

			// The world units per texel are used to snap  the orthographic projection
			// to texel sized increments.  
			// Because we're fitting tighly to the cascades, the shimmering shadow 
			// edges will still be present when the camera rotates.  However, when 
			// zooming in or strafing the shadow edge will not shimmer.
			vWorldUnitsPerTexel = vLightCameraOrthographicMax - vLightCameraOrthographicMin;
			vWorldUnitsPerTexel *= vNormalizeByBufferSize;

			
			float fLightCameraOrthographicMinY = vLightCameraOrthographicMin.y;


			// We snape the camera to 1 pixel increments so that moving the camera 
			// does not cause the shadows to jitter. This is a matter of integer 
			// dividing by the world space size of a texel
			vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMin = Floor(vLightCameraOrthographicMin);
			vLightCameraOrthographicMin *= vWorldUnitsPerTexel;

			vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMax = Floor(vLightCameraOrthographicMax);
			vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

			//These are the unconfigured near and far plane values.  They are purposly
			// awful to show how important calculating accurate near and far planes is.
			FLOAT fNearPlane = 0.0f;
			FLOAT fFarPlane = 10000.0f;

			Vec3 vLightSpaceSceneAABBminValue(FLT_MAX, FLT_MAX, FLT_MAX); // world space scene aabb 
			Vec3 vLightSpaceSceneAABBmaxValue(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			// We calculate the min and max vectors of the scene in light space. 
			// The min and max "Z" values of the  light space AABB can be used for 
			// the near and far plane. This is easier than intersecting the scene with
			// the AABBand in some cases provides similar results.
			for (int index = 0; index< 8; ++index)
			{
				vLightSpaceSceneAABBminValue = Min(
					sceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue);
				vLightSpaceSceneAABBmaxValue = Max(
					sceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue);
			}

			// The min and max z values are the near and far planes.
			// y,z is swapped. We need to use y value.
			fNearPlane = vLightSpaceSceneAABBminValue.y;
			fFarPlane = vLightSpaceSceneAABBmaxValue.y;
			
			// Craete the orthographic projection for this cascade.
			mOrthogonalData[iCascadeIndex].mWidth = 
				vLightCameraOrthographicMax.x - vLightCameraOrthographicMin.x;
			mOrthogonalData[iCascadeIndex].mHeight = 
				vLightCameraOrthographicMax.z - vLightCameraOrthographicMin.z;
			mOrthogonalData[iCascadeIndex].mNear = fNearPlane;
			mOrthogonalData[iCascadeIndex].mFar = fFarPlane;			

			mCascadePartitionsFrustum[iCascadeIndex] = fFrustumIntervalEnd;
		}
	}

	TexturePtr GetShadowMap() const{
		return mShadowMap;
	}

	void DeleteShadowMap(){
		mShadowMap = 0;
	}

	void RenderShadows(IScenePtr scene){
		auto& renderer = Renderer::GetInstance();
		if (!mShadowMap)
		{
			CreateShadowMap();
		}
		TexturePtr rts[] = { 0 };
		size_t index[] = { 0 };
		renderer.SetRenderTarget(rts, index, 1, mShadowMap, 0);
		auto provider = renderer.GetResourceProvider();
		provider->BindShader(ResourceTypes::Shaders::ShadowMapShader);		
		//the full extent of the resource view is always cleared. Viewport and scissor settings are not applied.
		renderer.Clear(0, 0, 0, 0, 1.0f, 0);

		provider->BindRasterizerState(ResourceTypes::RasterizerStates::ShadowMapRS);
		auto options = renderer.GetRendererOptions();
		// Iterate over cascades and render shadows.
		for (INT currentCascade = 0;
			currentCascade < options->r_ShadowCascadeLevels;
			++currentCascade)
		{
			// Each cascade has its own viewport because we're storing all the 
			// cascades in one large texture.
			renderer.SetViewports(&mViewports[currentCascade], 1);
			auto& orthogonalData = mOrthogonalData[currentCascade];
			mLightCamera->SetWidth(orthogonalData.mWidth);
			mLightCamera->SetHeight(orthogonalData.mHeight);
			mLightCamera->SetNearFar(orthogonalData.mNear, orthogonalData.mFar);
			mShadowProj[currentCascade] = mLightCamera->GetMatrix(ICamera::ProjBeforeSwap);
			renderer.SetCamera(mLightCamera);			
			RenderParam param;
			memset(&param, 0, sizeof(RenderParam));
			param.mCamera = mLightCamera.get();
			//param.mLightCamera = param.mCamera;
			param.mRenderPass = PASS_SHADOW;
			scene->PreRender(param, 0);
			scene->Render(param, 0);	
		}
		provider->BindRasterizerState(ResourceTypes::RasterizerStates::Default);

		SHADOW_CONSTANTS rc;				
		rc.gCascadeBlendArea = options->r_ShadowCascadeBlendArea;
		rc.gShadowPartitionSize = 1.0f / options->r_ShadowCascadeLevels;
		rc.gShadowTexelSize = 1.0f / options->r_ShadowMapSize;
		Mat44 textureScaleTranslMat(
			0.5f, 0, 0, .5,
			0, -0.5, 0, .5,
			0, 0, 1.0f, 0,
			0, 0, 0, 1);

		for (int i = 0; i < options->r_ShadowCascadeLevels; ++i){
			rc.gCascadeFrustumsEyeSpaceDepthsFloat[i] = mCascadePartitionsFrustum[i];
			auto shadowTextureMat = textureScaleTranslMat * mShadowProj[i];
			rc.gCascadeScale[i].x = shadowTextureMat[0][0];
			rc.gCascadeScale[i].y = shadowTextureMat[1][1];
			rc.gCascadeScale[i].z = shadowTextureMat[2][2];
			rc.gCascadeScale[i].w = 1;

			rc.gCascadeOffset[i].x = shadowTextureMat[0][3];
			rc.gCascadeOffset[i].y = shadowTextureMat[1][3];
			rc.gCascadeOffset[i].z = shadowTextureMat[2][3];
			rc.gCascadeOffset[i].w = 0;
		}
		renderer.UpdateShadowConstantsBuffer(&rc);


	}
};

//---------------------------------------------------------------------------
CascadedShadowsManagerPtr CascadedShadowsManager::Create(
	unsigned renderTargetId, const Vec2I& renderTargetSize){
	return CascadedShadowsManagerPtr(
		new CascadedShadowsManager(renderTargetId, renderTargetSize),
		[](CascadedShadowsManager* obj){ delete obj; });
}
CascadedShadowsManager::CascadedShadowsManager(unsigned renderTargetId, const Vec2I& renderTargetSize)
	: mImpl(new Impl(renderTargetId, renderTargetSize))
{

}

CascadedShadowsManager::~CascadedShadowsManager(){

}

void CascadedShadowsManager::CreateViewports() {
	mImpl->CreateViewports();
}

void CascadedShadowsManager::CreateShadowMap() {
	mImpl->CreateShadowMap();
}

void CascadedShadowsManager::RenderShadows(IScenePtr scene) {
	mImpl->RenderShadows(scene);
}

void CascadedShadowsManager::UpdateFrame(CameraPtr viewerCamera, const Vec3& lightDir,	const AABB& sceneAABB) {
	mImpl->UpdateFrame(viewerCamera, lightDir, sceneAABB);
}

TexturePtr CascadedShadowsManager::GetShadowMap() const{
	return mImpl->GetShadowMap();
}

void CascadedShadowsManager::DeleteShadowMap(){
	mImpl->DeleteShadowMap();
}

CameraPtr CascadedShadowsManager::GetLightCamera() const{
	return mImpl->mLightCamera;
}