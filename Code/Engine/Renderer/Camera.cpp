#include <Engine/StdAfx.h>
#include <Engine/Renderer/Camera.h>
#include <CommonLib/Math/fbMath.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IEngine.h>
#include <Engine/IRenderer.h>
#include <Engine/IMouse.h>
#include <Engine/IKeyboard.h>
#include <Engine/Misc/EngineCommand.h>
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
	, mTarget(0)
	, mCurrentCamera(false)
	, mCamIndex(-1)
	, mProcessInput(false)
	, mPrevTargetPos(0, 0, 0)
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
	if (mTarget)
	{
		mTarget->RemoveCameraTargetingMe(this);
		mTarget = 0;
	}
}

//----------------------------------------------------------------------------
void Camera::SetOrthogonal(bool ortho)
{
	mOrthogonal = ortho;
	mProjPropertyChanged = true;
}

void Camera::SetPos(const Vec3& pos)
{
	WRITE_LOCK lock(mCamPosRWCS);
	SpatialObject::SetPos(pos);
	mViewPropertyChanged = true;
}

const Vec3& Camera::GetPos() const
{
	READ_LOCK lock(*(FB_READ_WRITE_CS*)&mCamPosRWCS);
	return SpatialObject::GetPos();
}

void Camera::SetRot(const Quat& rot)
{
	SpatialObject::SetRot(rot);
	mViewPropertyChanged = true;
}

void Camera::SetDir(const Vec3& dir)
{
	SpatialObject::SetDir(dir);
	mViewPropertyChanged = true;
}

void Camera::SetDirAndRight(const Vec3& dir, const Vec3& right)
{
	SpatialObject::SetDirAndRight(dir, right);
	mViewPropertyChanged = true;
}

void Camera::SetCamTransform(const Vec3& pos, const Quat& rot)
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
void Camera::ProcessInputData()
{
	if (!mProcessInput || !mCurrentCamera)
		return;
	if (mUserParams.Changed() || mPrevTargetPos != mTarget->GetPos())
	{
		mInternalParams.dist += mUserParams.dDist;
		mInternalParams.dist = std::max(2.0f, mInternalParams.dist);
		if (mInternalParams.dist > 300.0f)
			mInternalParams.dist = 300.0f;

		mInternalParams.pitch += mUserParams.dPitch;
		if (mInternalParams.pitch > fastbird::HALF_PI - fastbird::Radian(5.f))
		{
			mInternalParams.pitch = fastbird::HALF_PI - fastbird::Radian(5.f);
		}
		else if (mInternalParams.pitch <  -fastbird::HALF_PI + fastbird::Radian(5.f))
		{
			mInternalParams.pitch = -fastbird::HALF_PI + fastbird::Radian(5.f);
		}

		mInternalParams.yaw += mUserParams.dYaw;
		if (mInternalParams.yaw > fastbird::TWO_PI)
		{
			mInternalParams.yaw -= fastbird::TWO_PI;
		}
		else if (mInternalParams.yaw < -fastbird::TWO_PI)
		{
			mInternalParams.yaw += fastbird::TWO_PI;
		}

		Vec3 defaultDir = -Vec3::UNIT_Y;
		Quat qPitch(mInternalParams.pitch, Vec3::UNIT_X);
		Quat qYaw(-mInternalParams.yaw, Vec3::UNIT_Z);
		Vec3 toCam = qPitch * defaultDir;
		toCam = qYaw * toCam;
		Vec3 forward = -toCam;
		Vec3 right = forward.Cross(Vec3::UNIT_Z);
		right.Normalize();
		Vec3 up = right.Cross(forward);
		forward = up.Cross(right);

		Mat33 rot(right.x, forward.x, up.x,
			right.y, forward.y, up.y,
			right.z, forward.z, up.z);
		Vec3 pos = mTarget->GetPos() + toCam * mInternalParams.dist;
		SetCamTransform(pos, rot);
		mUserParams.Clear();
		mPrevTargetPos = mTarget->GetPos();
	}
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
		mViewMat = fastbird::MakeViewMatrix(pos, right, forward, up);
		// The same code
		//Transformation temp;
		//mTransformation.Inverse(temp);
		//temp.GetHomogeneous(mViewMat);

		//mInvViewMat = 
		mTransformation.GetHomogeneous(mInvViewMat);
	}

	bool projChanged = mProjPropertyChanged;
	if (mProjPropertyChanged)
	{
		mAspectRatio = mWidth / mHeight;
		mProjPropertyChanged = false;
		if (!mOrthogonal)
		{
			mProjMat = MakeProjectionMatrix(mFov, mAspectRatio, mNear, mFar);
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

		if (viewChanged)
		{
			for (auto var : mCamListener)
			{
				var->OnViewMatChanged();
			}
		}
		if (projChanged)
		{
			for (auto var : mCamListener)
			{
				var->OnProjMatChanged();
			}
		}
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
	for (int i=0; i<6; i++)
	{
		if (pBV->WhichSide(mPlanes[i])<0)
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

Vec2I Camera::WorldToScreen(const Vec3& worldPos) const
{
	auto projPos = mViewProjMat * Vec4(worldPos, 1);
	float x = projPos.x / projPos.w;
	float y = projPos.y / projPos.w;
	int ix = Round((x * .5f + .5f) * mWidth);
	int iy = Round((-y*.5f + .5f) * mHeight);
	return Vec2I(ix, iy);
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

//----------------------------------------------------------------------------
void Camera::SetTarget(SpatialObject* pObj)
{
	if (mTarget == pObj)
		return;

	if (mTarget)
		mTarget->RemoveCameraTargetingMe(this);

	mTarget = pObj;
	if (mTarget)
		mTarget->AddCameraTargetingMe(this);
}

//---------------------------------------------------------------------------
void Camera::OnInputFromEngine(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	if (!mCurrentCamera)
		return;
	if (!mProcessInput || !mTarget)
		return;

	if (pMouse && pMouse->IsValid() && !pKeyboard->IsKeyDown(VK_CONTROL))
	{
		const Vec3 camPos = GetPos();
		Vec3 toCam = camPos - mTarget->GetPos();
		const float distToTarget = toCam.Normalize();
		long dx, dy;
		pMouse->GetHDDeltaXY(dx, dy);

		if (pMouse->IsLButtonDown())
		{
			float mouseSens = gFBEnv->pConsole->GetEngineCommand()->MouseSens;
			if (dx != 0)
			{
				mUserParams.dYaw = dx * mouseSens;
			}

			if (dy != 0)
			{
				mUserParams.dPitch = -dy * mouseSens;
			}

			pMouse->LockMousePos(true);
			pMouse->Invalidate();
		}
		else
		{
			pMouse->LockMousePos(false);
		}

		long wheel = pMouse->GetWheel();
		if (wheel)
		{
			float shift = 1.0f;
			if (pKeyboard->IsKeyDown(VK_SHIFT))
				shift = 0.1f;
			float wheelSens = gFBEnv->pConsole->GetEngineCommand()->WheelSens;
			float wheelSensitivity = wheelSens * (float)pMouse->GetNumLinesWheelScroll();
			wheelSensitivity *= std::max(1.f, mInternalParams.dist * 0.05f);
			mUserParams.dDist += -wheel * wheelSensitivity * shift;
			pMouse->Invalidate();
		}
	}
}

void Camera::SetEnalbeInput(bool enable)
{
	mProcessInput = enable;
}

void Camera::SetInitialDistToTarget(float dist)
{
	mInternalParams.dist = dist;
}

void Camera::RegisterCamListener(ICameraListener* p)
{
	assert(std::find(mCamListener.begin(), mCamListener.end(), p) == mCamListener.end());
	mCamListener.push_back(p);
}

void Camera::UnregisterCamListener(ICameraListener* p)
{
	mCamListener.erase(std::remove(mCamListener.begin(), mCamListener.end(), p), mCamListener.end());
}