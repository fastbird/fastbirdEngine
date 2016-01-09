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
		float mL, mR, mT, mB;		
		float mNear;
		float mFar;
	};
	OrthogonalData mOrthogonalData[MAX_CASCADES];
	bool mMainRt;

	//---------------------------------------------------------------------------
	Impl(unsigned renderTargetId, const Vec2I& renderTargetSize)
		: mRenderTargetId(renderTargetId)
		, mRenderTargetSize(renderTargetSize)
		, mMainRt(false)
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
			mCascadePartitionsFrustum[i] = FLT_MAX;
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
	// Used to compute an intersection of the orthographic projection and the Scene AABB
	//--------------------------------------------------------------------------------------
	struct Triangle
	{
		Vec3 pt[3];
		bool culled;
	};

	//--------------------------------------------------------------------------------------
	// Computing an accurate near and flar plane will decrease surface acne and 
	// Peter-panning.
	// Surface acne is the term for erroneous self shadowing.  Peter-panning is 
	// the effect where shadows disappear near the base of an object.
	// As offsets are generally used with PCF filtering due self shadowing issues,
	// computing the correct near and far planes becomes even more important.
	// This concept is not complicated, but the intersection code is.
	//--------------------------------------------------------------------------------------
	void ComputeNearAndFar(float& fNearPlane,
		float& fFarPlane,
		Vec3 vLightCameraOrthographicMin,
		Vec3 vLightCameraOrthographicMax,
		Vec3* pvPointsInCameraView)
	{

		// Initialize the near and far planes
		fNearPlane = vLightCameraOrthographicMin.y;
		fFarPlane = vLightCameraOrthographicMax.y;

		Triangle triangleList[16];
		INT iTriangleCnt = 1;

		triangleList[0].pt[0] = pvPointsInCameraView[0];
		triangleList[0].pt[1] = pvPointsInCameraView[1];
		triangleList[0].pt[2] = pvPointsInCameraView[2];
		triangleList[0].culled = false;

		// These are the indices used to tesselate an AABB into a list of triangles.
		static const INT iAABBTriIndexes[] =
		{
			0, 1, 2, 1, 2, 3,
			4, 5, 6, 5, 6, 7,
			0, 2, 4, 2, 4, 6,
			1, 3, 5, 3, 5, 7,
			0, 1, 4, 1, 4, 5,
			2, 3, 6, 3, 6, 7
		};

		int iPointPassesCollision[3];

		// At a high level: 
		// 1. Iterate over all 12 triangles of the AABB.  
		// 2. Clip the triangles against each plane. Create new triangles as needed.
		// 3. Find the min and max z values as the near and far plane.

		//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

		float fLightCameraOrthographicMinX = vLightCameraOrthographicMin.x;
		float fLightCameraOrthographicMaxX = vLightCameraOrthographicMax.x;
		float fLightCameraOrthographicMinZ = vLightCameraOrthographicMin.z;
		float fLightCameraOrthographicMaxZ = vLightCameraOrthographicMax.z;

		for (int AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter)
		{

			triangleList[0].pt[0] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 0]];
			triangleList[0].pt[1] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 1]];
			triangleList[0].pt[2] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 2]];
			iTriangleCnt = 1;
			triangleList[0].culled = false;

			// Clip each invidual triangle against the 4 frustums.  When ever a 
			// triangle is clipped into new triangles, add them to the list.
			for (int frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
			{

				float fEdge;
				int iComponent;

				if (frustumPlaneIter == 0)
				{
					fEdge = fLightCameraOrthographicMinX; // todo make float temp
					iComponent = 0;
				}
				else if (frustumPlaneIter == 1)
				{
					fEdge = fLightCameraOrthographicMaxX;
					iComponent = 0;
				}
				else if (frustumPlaneIter == 2)
				{
					fEdge = fLightCameraOrthographicMinZ;
					iComponent = 2;
				}
				else
				{
					fEdge = fLightCameraOrthographicMaxZ;
					iComponent = 2;
				}

				for (int triIter = 0; triIter < iTriangleCnt; ++triIter)
				{
					// We don't delete triangles, so we skip those that have been culled.
					if (!triangleList[triIter].culled)
					{
						int iInsideVertCount = 0;
						Vec3 tempOrder;
						// Test against the correct frustum plane.
						// This could be written more compactly, but it would be harder to understand.
						if (frustumPlaneIter == 0)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (triangleList[triIter].pt[triPtIter].x >
									vLightCameraOrthographicMin.x)
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else if (frustumPlaneIter == 1)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (triangleList[triIter].pt[triPtIter].x <
									vLightCameraOrthographicMax.x)
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else if (frustumPlaneIter == 2)
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (triangleList[triIter].pt[triPtIter].z >
									vLightCameraOrthographicMin.z)
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}
						else
						{
							for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
							{
								if (triangleList[triIter].pt[triPtIter].z <
									vLightCameraOrthographicMax.z)
								{
									iPointPassesCollision[triPtIter] = 1;
								}
								else
								{
									iPointPassesCollision[triPtIter] = 0;
								}
								iInsideVertCount += iPointPassesCollision[triPtIter];
							}
						}

						// Move the points that pass the frustum test to the begining of the array.
						if (iPointPassesCollision[1] && !iPointPassesCollision[0])
						{
							tempOrder = triangleList[triIter].pt[0];
							triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = tempOrder;
							iPointPassesCollision[0] = 1;
							iPointPassesCollision[1] = 0;
						}
						if (iPointPassesCollision[2] && !iPointPassesCollision[1])
						{
							tempOrder = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
							triangleList[triIter].pt[2] = tempOrder;
							iPointPassesCollision[1] = 1;
							iPointPassesCollision[2] = 0;
						}
						if (iPointPassesCollision[1] && !iPointPassesCollision[0])
						{
							tempOrder = triangleList[triIter].pt[0];
							triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
							triangleList[triIter].pt[1] = tempOrder;
							iPointPassesCollision[0] = 1;
							iPointPassesCollision[1] = 0;
						}

						if (iInsideVertCount == 0)
						{ // All points failed. We're done,  
							triangleList[triIter].culled = true;
						}
						else if (iInsideVertCount == 1)
						{// One point passed. Clip the triangle against the Frustum plane
							triangleList[triIter].culled = false;

							// 
							Vec3 vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
							Vec3 vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

							// Find the collision ratio.
							FLOAT fHitPointTimeRatio = fEdge - triangleList[triIter].pt[0][iComponent];
							// Calculate the distance along the vector as ratio of the hit ratio to the component.
							FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / vVert0ToVert1[iComponent];
							FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / vVert0ToVert2[iComponent];
							// Add the point plus a percentage of the vector.
							vVert0ToVert1 *= fDistanceAlongVector01;
							vVert0ToVert1 += triangleList[triIter].pt[0];
							vVert0ToVert2 *= fDistanceAlongVector02;
							vVert0ToVert2 += triangleList[triIter].pt[0];

							triangleList[triIter].pt[1] = vVert0ToVert2;
							triangleList[triIter].pt[2] = vVert0ToVert1;

						}
						else if (iInsideVertCount == 2)
						{ // 2 in  // tesselate into 2 triangles


							// Copy the triangle\(if it exists) after the current triangle out of
							// the way so we can override it with the new triangle we're inserting.
							triangleList[iTriangleCnt] = triangleList[triIter + 1];

							triangleList[triIter].culled = false;
							triangleList[triIter + 1].culled = false;

							// Get the vector from the outside point into the 2 inside points.
							Vec3 vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
							Vec3 vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

							// Get the hit point ratio.
							FLOAT fHitPointTime_2_0 = fEdge - triangleList[triIter].pt[2][iComponent];
							FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / vVert2ToVert0[iComponent];
							// Calcaulte the new vert by adding the percentage of the vector plus point 2.
							vVert2ToVert0 *= fDistanceAlongVector_2_0;
							vVert2ToVert0 += triangleList[triIter].pt[2];

							// Add a new triangle.
							triangleList[triIter + 1].pt[0] = triangleList[triIter].pt[0];
							triangleList[triIter + 1].pt[1] = triangleList[triIter].pt[1];
							triangleList[triIter + 1].pt[2] = vVert2ToVert0;

							//Get the hit point ratio.
							FLOAT fHitPointTime_2_1 = fEdge - triangleList[triIter].pt[2][iComponent];
							FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / vVert2ToVert1[iComponent];
							vVert2ToVert1 *= fDistanceAlongVector_2_1;
							vVert2ToVert1 += triangleList[triIter].pt[2];
							triangleList[triIter].pt[0] = triangleList[triIter + 1].pt[1];
							triangleList[triIter].pt[1] = triangleList[triIter + 1].pt[2];
							triangleList[triIter].pt[2] = vVert2ToVert1;
							// Cncrement triangle count and skip the triangle we just inserted.
							++iTriangleCnt;
							++triIter;


						}
						else
						{ // all in
							triangleList[triIter].culled = false;

						}
					}// end if !culled loop            
				}
			}
			for (INT index = 0; index < iTriangleCnt; ++index)
			{
				if (!triangleList[index].culled)
				{
					// Set the near and far plan and the min and max z values respectivly.
					for (int vertind = 0; vertind < 3; ++vertind)
					{
						float fTriangleCoordY = triangleList[index].pt[vertind].y;
						if (fNearPlane > fTriangleCoordY)
						{
							fNearPlane = fTriangleCoordY;
						}
						if (fFarPlane < fTriangleCoordY)
						{
							fFarPlane = fTriangleCoordY;
						}
					}
				}
			}
		}

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
		pvCornerPointsWorld[2] = Vec3(frustum.mRightSlope * fCascadeIntervalBegin,
			fCascadeIntervalBegin, frustum.mBottomSlope * fCascadeIntervalBegin);// right bottom
		pvCornerPointsWorld[3] = Vec3(frustum.mLeftSlope * fCascadeIntervalBegin,
			fCascadeIntervalBegin, frustum.mBottomSlope * fCascadeIntervalBegin); // left bottom

		pvCornerPointsWorld[4] = Vec3(frustum.mRightSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mTopSlope * fCascadeIntervalEnd); // right top
		pvCornerPointsWorld[5] = Vec3(frustum.mLeftSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mTopSlope * fCascadeIntervalEnd);// left top
		pvCornerPointsWorld[6] = Vec3(frustum.mRightSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mBottomSlope * fCascadeIntervalEnd);// right bottom
		pvCornerPointsWorld[7] = Vec3(frustum.mLeftSlope * fCascadeIntervalEnd,
			fCascadeIntervalEnd, frustum.mBottomSlope * fCascadeIntervalEnd); // left bottom
		for (int i = 0; i < 8; ++i){
			pvCornerPointsWorld[i] = frustum.mOrientation * pvCornerPointsWorld[i];
			pvCornerPointsWorld[i] += frustum.mOrigin;
		}
	}

	void CreateAABBPoints(Vec3 sceneAABBPointsLightSpace[8], const AABB& sceneAABB){
		//This map enables us to use a for loop and do vector math.
		static const Vec3 vExtentsMap[] =
		{
			{ 1.0f, -1.0f, 1.0f },
			{ -1.0f, -1.0f, 1.0f },
			{ 1.0f, -1.0f, -1.0f },
			{ -1.0f, -1.0f, -1.0f },
			{ 1.0f, 1.0f, 1.0f },
			{ -1.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, -1.0f },
			{ -1.0f, 1.0f, -1.0f }
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
		auto overriding = viewerCamera->GetOverridingCamera();
		viewerCamera->SetOverridingCamera(0);
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
		for (int i = 0; i < 8; ++i){
			mCascadePartitionsFrustum[i] = FLT_MAX;
		}
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
			Vec3 vNormalizeByBufferSize(fNormalizeByBufferSize, 1.f, fNormalizeByBufferSize);

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
			vWorldUnitsPerTexel.y = vLightCameraOrthographicMin.y;
			vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMin = Floor(vLightCameraOrthographicMin);
			vLightCameraOrthographicMin *= vWorldUnitsPerTexel;

			vWorldUnitsPerTexel.y = vLightCameraOrthographicMax.y;
			vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
			vLightCameraOrthographicMax = Floor(vLightCameraOrthographicMax);
			vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

			FLOAT fNearPlane = 0.0f;
			FLOAT fFarPlane = 10000.0f;
			ComputeNearAndFar(fNearPlane, fFarPlane, vLightCameraOrthographicMin,
				vLightCameraOrthographicMax, sceneAABBPointsLightSpace);
			//Vec3 vLightSpaceSceneAABBminValue(FLT_MAX, FLT_MAX, FLT_MAX); // world space scene aabb 
			//Vec3 vLightSpaceSceneAABBmaxValue(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			//// We calculate the min and max vectors of the scene in light space. 
			//// The min and max "Z" values of the  light space AABB can be used for 
			//// the near and far plane. This is easier than intersecting the scene with
			//// the AABBand in some cases provides similar results.
			//for (int index = 0; index< 8; ++index)
			//{
			//	vLightSpaceSceneAABBminValue = Min(
			//		sceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue);
			//	vLightSpaceSceneAABBmaxValue = Max(
			//		sceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue);
			//}

			//// The min and max z values are the near and far planes.
			//// y,z is swapped. We need to use y value.
			//fNearPlane = vLightSpaceSceneAABBminValue.y;
			//fFarPlane = vLightSpaceSceneAABBmaxValue.y;
			
			// Craete the orthographic projection for this cascade.
			mOrthogonalData[iCascadeIndex].mL = vLightCameraOrthographicMin.x;
			mOrthogonalData[iCascadeIndex].mR = vLightCameraOrthographicMax.x;
			mOrthogonalData[iCascadeIndex].mT = vLightCameraOrthographicMax.z;
			mOrthogonalData[iCascadeIndex].mB = vLightCameraOrthographicMin.z;
			mOrthogonalData[iCascadeIndex].mNear = fNearPlane;
			mOrthogonalData[iCascadeIndex].mFar = fFarPlane;			

			mCascadePartitionsFrustum[iCascadeIndex] = fFrustumIntervalEnd;
		}
		viewerCamera->SetOverridingCamera(overriding);
	}

	TexturePtr GetShadowMap() const{
		return mShadowMap;
	}

	void DeleteShadowMap(){
		mShadowMap = 0;
	}

	void SetMain(bool main){
		mMainRt = main;
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
		
		Color colors[MAX_CASCADES] = {
			Color::Red,
			Color::Green,
			Color::Blue,
			Color::White,
			Color(0.8, 0.8, 0.8),
			Color(0.6, 0.6, 0.6),
			Color(0.4, 0.4, 0.4),
			Color(0.2, 0.2, 0.2)
		};
		// Iterate over cascades and render shadows.
		for (INT currentCascade = 0;
			currentCascade < options->r_ShadowCascadeLevels;
			++currentCascade)
		{
			// Each cascade has its own viewport because we're storing all the 
			// cascades in one large texture.
			renderer.SetViewports(&mViewports[currentCascade], 1);
			auto& orthogonalData = mOrthogonalData[currentCascade];
			mLightCamera->SetOrthogonalData(orthogonalData.mL, orthogonalData.mT,
				orthogonalData.mR, orthogonalData.mB);			
			mLightCamera->SetNearFar(orthogonalData.mNear, orthogonalData.mFar);
			mShadowProj[currentCascade] = mLightCamera->GetMatrix(ICamera::ProjBeforeSwap);
			renderer.SetCamera(mLightCamera);			
			RenderParam param;
			memset(&param, 0, sizeof(RenderParam));
			param.mCamera = mLightCamera.get();
			//param.mLightCamera = param.mCamera;
			param.mRenderPass = PASS_SHADOW;
			scene->MakeVisibleSet(mLightCamera.get(), true);
			scene->Render(param, 0);	
			if (mMainRt){
				AABB aabb;
				aabb.SetMax(Vec3(orthogonalData.mR, orthogonalData.mFar, orthogonalData.mT));
				aabb.SetMin(Vec3(orthogonalData.mL, orthogonalData.mNear, orthogonalData.mB));
				renderer.QueueDrawAABB(aabb, mLightCamera->GetTransformation(), colors[currentCascade]);
			}
		}
		provider->BindRasterizerState(ResourceTypes::RasterizerStates::Default);

		SHADOW_CONSTANTS rc;				
		rc.gCascadeBlendArea = options->r_ShadowCascadeBlendArea;
		rc.gShadowPartitionSize = 1.0f / (float)options->r_ShadowCascadeLevels;
		rc.gShadowTexelSize = 1.0f / (float)options->r_ShadowMapSize;
		rc.gShadowTexelSizeX = rc.gShadowTexelSize / (float)options->r_ShadowCascadeLevels;
		Mat44 textureScaleTranslMat(
			0.5f, 0, 0, .5,
			0, -0.5, 0, .5,
			0, 0, 1.0f, 0,
			0, 0, 0, 1);

		for (int i = 0; i < options->r_ShadowCascadeLevels; ++i){
			float* dest = &rc.gCascadeFrustumsEyeSpaceDepthsFloat[0].x;
			*(dest+i)  = mCascadePartitionsFrustum[i];
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
CascadedShadowsManager::CascadedShadowsManager(unsigned renderTargetId, 
	const Vec2I& renderTargetSize)
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

void CascadedShadowsManager::SetMain(bool main){
	mImpl->SetMain(main);
}