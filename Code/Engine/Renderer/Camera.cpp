#include <Engine/StdAfx.h>
#include <Engine/Renderer/Camera.h>
#include <CommonLib/Math/fbMath.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IMouse.h>
#include <Engine/IKeyboard.h>
#include <CommonLib/Timer.h>
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Debug.h>

using namespace fastbird;

//----------------------------------------------------------------------------
Camera::Camera()
	: mViewPropertyChanged(true)
	, mProjPropertyChanged(true)
	, mOrthogonal(false)
	, mYZSwap(true)
{
	// proj properties
	mFov = Radian(70);
	if (IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer())
	{
		mWidth = (float)pRenderer->GetWidth();
		mHeight = (float)pRenderer->GetHeight();
	}
	else
	{
		Error("No Renderer found while creating the camera.");
	}
	mNear = 1.3f;
	mFar = 2000.0f;
}

Camera::~Camera()
{
}

//----------------------------------------------------------------------------
void Camera::SetOrthogonal(bool ortho)
{
	mOrthogonal = ortho;
	mProjPropertyChanged = true;
}

void Camera::SetPos(const Vec3& pos)
{
	SpatialObject::SetPos(pos);
	mViewPropertyChanged = true;
}

const Vec3& Camera::GetPos() const
{
	return SpatialObject::GetPos();
}

void Camera::SetRot(const Quat& rot)
{
	SpatialObject::SetRot(rot);
	mViewPropertyChanged = true;
}

void Camera::SetDir(const Vec3& dir)
{
	Vec3 forward = dir;
	forward.Normalize();
	Vec3 right;
	if (forward == Vec3::UNIT_Z || forward == -Vec3::UNIT_Z)
	{
		right = Vec3::UNIT_X;
	}
	else
	{
		right = forward.Cross(Vec3::UNIT_Z);
	} 
	Vec3 up = right.Cross(forward);


	right.Normalize();
	up.Normalize();

	Mat33 rot;
	rot.SetColumn(0, right);
	rot.SetColumn(1, forward);
	rot.SetColumn(2, up);
	mTransformation.SetRotation(rot);
	mViewPropertyChanged = true;
}

void Camera::SetTransformation(const Vec3& pos, const Quat& rot)
{
	mTransformation.SetTranslation(pos);
	mTransformation.SetRotation(rot);
	mViewPropertyChanged = true;
}

void Camera::SetTransform(const Transformation& t)
{
	__super::SetTransform(t);
	mViewPropertyChanged = true;
}

const Vec3 Camera::GetDir() const
{
	return mTransformation.GetMatrix().Column(1);
}

//----------------------------------------------------------------------------
void Camera::SetNearFar(float n, float f)
{
	mNear = n;
	mFar = f;
	mProjPropertyChanged = true;
}

//----------------------------------------------------------------------------
void Camera::GetNearFar(float& n, float& f) const
{
	n = mNear;
	f = mFar;
}

//----------------------------------------------------------------------------
void Camera::Update()
{
	// world coordinates (Blender style)
	// x: right
	// y: forward
	// z: up
	bool viewChanged = mViewPropertyChanged;
	if (mViewPropertyChanged)
	{
		mViewPropertyChanged = false;
		Vec3 right = mTransformation.GetMatrix().Column(0);
		Vec3 forward = mTransformation.GetMatrix().Column(1);
		Vec3 up = mTransformation.GetMatrix().Column(2);
		const Vec3& pos = mTransformation.GetTranslation();
		
		mViewMat  = fastbird::MakeViewMatrix(pos, right, forward, up);
		mInvViewMat = mViewMat.InverseAffine();
	}

	bool projChanged = mProjPropertyChanged;
	if (mProjPropertyChanged)
	{
		mProjPropertyChanged = false;
		if (!mOrthogonal)
		{
			mProjMat = MakeProjectionMatrix(mFov, mWidth/mHeight, mNear, mFar);
		}
		else
		{
			mProjMat = MakeOrthogonalMatrix(-mWidth*.5f, mHeight*.5f, mWidth*.5f, -mHeight*.5f, mNear, mFar);
		}
		if (mYZSwap)
		{
			Mat44 swapMat(
				1, 0, 0, 0,
				0, 0, 1, 0,
				0, 1, 0, 0,
				0, 0, 0, 1); 
			mProjMat = mProjMat * swapMat;
		}

		gFBEnv->pRenderer->UpdateRareConstantsBuffer();
	}

	if (projChanged || viewChanged)
	{
		mViewProjMat = mProjMat * mViewMat;
		mInvViewProjMat = mViewProjMat.Inverse();

		UpdateFrustum();
	}
}

//----------------------------------------------------------------------------
void Camera::UpdateFrustum()
{
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.x = mViewProjMat[3][0] + mViewProjMat[0][0];
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.y = mViewProjMat[3][1] + mViewProjMat[0][1];
	mPlanes[FRUSTUM_PLANE_LEFT].mNormal.z = mViewProjMat[3][2] + mViewProjMat[0][2];
	mPlanes[FRUSTUM_PLANE_LEFT].mConstant = -(mViewProjMat[3][3] + mViewProjMat[0][3]);

	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.x = mViewProjMat[3][0] - mViewProjMat[0][0];
	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.y = mViewProjMat[3][1] - mViewProjMat[0][1];
	mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.z = mViewProjMat[3][2] - mViewProjMat[0][2];
	mPlanes[FRUSTUM_PLANE_RIGHT].mConstant = -(mViewProjMat[3][3] - mViewProjMat[0][3]);

	mPlanes[FRUSTUM_PLANE_TOP].mNormal.x = mViewProjMat[3][0] - mViewProjMat[1][0];
	mPlanes[FRUSTUM_PLANE_TOP].mNormal.y = mViewProjMat[3][1] - mViewProjMat[1][1];
	mPlanes[FRUSTUM_PLANE_TOP].mNormal.z = mViewProjMat[3][2] - mViewProjMat[1][2];
	mPlanes[FRUSTUM_PLANE_TOP].mConstant = -(mViewProjMat[3][3] - mViewProjMat[1][3]);

	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.x = mViewProjMat[3][0] + mViewProjMat[1][0];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.y = mViewProjMat[3][1] + mViewProjMat[1][1];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.z = mViewProjMat[3][2] + mViewProjMat[1][2];
	mPlanes[FRUSTUM_PLANE_BOTTOM].mConstant = -(mViewProjMat[3][3] + mViewProjMat[1][3]);

	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.x = mViewProjMat[3][0] + mViewProjMat[2][0];
	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.y = mViewProjMat[3][1] + mViewProjMat[2][1];
	mPlanes[FRUSTUM_PLANE_NEAR].mNormal.z = mViewProjMat[3][2] + mViewProjMat[2][2];
	mPlanes[FRUSTUM_PLANE_NEAR].mConstant = -(mViewProjMat[3][3] + mViewProjMat[2][3]);

	mPlanes[FRUSTUM_PLANE_FAR].mNormal.x = mViewProjMat[3][0] - mViewProjMat[2][0];
	mPlanes[FRUSTUM_PLANE_FAR].mNormal.y = mViewProjMat[3][1] - mViewProjMat[2][1];
	mPlanes[FRUSTUM_PLANE_FAR].mNormal.z = mViewProjMat[3][2] - mViewProjMat[2][2];
	mPlanes[FRUSTUM_PLANE_FAR].mConstant = -(mViewProjMat[3][3] - mViewProjMat[2][3]);

	// Renormalise any normals which were not unit length
	for(int i=0; i<6; i++ ) 
	{
		float length = mPlanes[i].mNormal.Normalize();
		mPlanes[i].mConstant /= length;
	}

	mFrustumMax = Vec3(-FLT_MAX,-FLT_MAX, -FLT_MAX);
	mFrustumMin = Vec3(FLT_MAX,FLT_MAX, FLT_MAX);
	for (int i=0; i<6; i++)
	{
		Vec3 pos = mPlanes[i].mNormal * mPlanes[i].mConstant;

		mFrustumMax = Max(mFrustumMax, pos);
		mFrustumMin = Min(mFrustumMin, pos);
		
	}
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetViewMat()
{
	Update();
	return mViewMat;
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetInvViewMat()
{
	Update();
	return mInvViewMat;
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetProjMat()
{
	Update();
	return mProjMat;

}

//----------------------------------------------------------------------------
const Mat44& Camera::GetViewProjMat()
{
	Update();
	return mViewProjMat;
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetInvViewProjMat()
{
	Update();
	return mInvViewProjMat;
}
//----------------------------------------------------------------------------
bool Camera::IsCulled(BoundingVolume* pBV) const
{
	if (pBV->GetBVType() == BoundingVolume::BV_SPHERE)
	{
		for (int i=0; i<6; i++)
		{
			if (pBV->WhichSide(mPlanes[i])<0)
				return true;
		}
	}
	else if (pBV->GetBVType() == BoundingVolume::BV_AABB)
	{
		if (pBV->WhichSide(mFrustumMin, mFrustumMax)<0)
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------
Ray3 Camera::ScreenPosToRay(long x, long y)
{
	Update();
	float fx = 2.0f * x / mWidth - 1.0f;
	float fy = 1.0f - 2.0f * y / mHeight;
	Vec3 screenPos((float)fx, (float)fy, -1.0f);
	Vec3 screenMidPos((float)fx, (float)fy, 0.0f);
	Vec3 origin = mInvViewProjMat * screenPos;
	Vec3 target = mInvViewProjMat * screenMidPos;
	Vec3 dir = target - origin;
	dir.Normalize();

	Ray3 ray(origin, dir);
	return ray;
}

//----------------------------------------------------------------------------
void Camera::PreRender()
{
}

//----------------------------------------------------------------------------
void Camera::Render()
{
}

//----------------------------------------------------------------------------
void Camera::PostRender()
{
}