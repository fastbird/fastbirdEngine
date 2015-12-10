/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "Camera.h"
#include "FBMathLib/BoundingVolume.h"
#include "FBInputManager/IInputInjector.h"
#include "FBInputManager/KeyCodes.h"
#include "FBSceneManager/ISpatialObject.h"

using namespace fb;

class Camera::Impl{
public:
	struct UserParameters
	{
		UserParameters()
		{
			Clear();
			forceChanged = true;
		}
		void Clear()
		{
			dDist = 0.00f, dYaw = 0.f, dPitch = 0.f;
			forceChanged = false;
		}

		bool Changed()
		{
			return dDist != 0.f || dYaw != 0.f || dPitch != 0.f || forceChanged;
		}
		Real dDist;
		Real dYaw;
		Real dPitch;
		bool forceChanged;

	} mUserParams;

	struct InternalParameters
	{
		InternalParameters()
		{
			dist = 10.0f;
			yaw = 0.f;
			pitch = 0.f;
		}
		Real dist;
		Real yaw;
		Real pitch;
	} mInternalParams;

	Camera* mSelf;
	bool mMainCamera;
	bool mViewPropertyChanged;
	bool mProjPropertyChanged;
	bool mOrthogonal;
	Mat44 mMatrices[NumMatrices];
	Transformation mTransformation;
	Real mFov;
	Real mTanHalfFOV;
	Real mAspectRatio;
	Real mNear;
	Real mFar;
	Real mWidth;
	Real mHeight;
	std::string mName;
	Plane3 mPlanes[6];
	ISpatialObjectWeakPtr mTarget;
	size_t mCamIndex;
	bool mYZSwap;
	bool mCurrentCamera;
	bool mProcessInput;
	Vec3 mPrevTargetPos;
	std::mutex mMutex;

	Impl(Camera* self) 
		: mSelf(self)
		, mMainCamera(false)
		, mViewPropertyChanged(true)
		, mProjPropertyChanged(true)
		, mOrthogonal(false)
		, mYZSwap(true)
		, mCurrentCamera(false)
		, mCamIndex(-1)
		, mProcessInput(false)
		, mPrevTargetPos(0, 0, 0){
		// proj properties
		SetFOV(Radian(70));
		mNear = 0.5f;
		mFar = 1500.0f;
	}

	~Impl(){		
	}

	Impl& operator=(const Impl& other){
		mViewPropertyChanged = other.mViewPropertyChanged;
		mProjPropertyChanged = other.mProjPropertyChanged;
		mOrthogonal = other.mOrthogonal;
		for (int i = 0; i < NumMatrices; ++i){
			mMatrices[i] = other.mMatrices[i];
		}
		mTransformation = other.mTransformation;
		mFov = other.mFov;
		mTanHalfFOV = other.mTanHalfFOV;
		mAspectRatio = other.mAspectRatio;
		mNear = other.mNear;
		mFar = other.mFar;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mName = other.mName;
		for (int i = 0; i < 6; ++i){
			mPlanes[i] = other.mPlanes[i];
		}
		mTarget = other.mTarget;
		mCamIndex = other.mCamIndex;
		mYZSwap = other.mYZSwap;
		mCurrentCamera = other.mCurrentCamera;
		mProcessInput = other.mProcessInput;
		mPrevTargetPos = other.mPrevTargetPos;		
		return *this;
	}

	//----------------------------------------------------------------------------
	void SetOrthogonal(bool ortho)
	{		
		mOrthogonal = ortho;
		mProjPropertyChanged = true;
	}

	void SetPosition(const Vec3& pos)
	{
		mTransformation.SetTranslation(pos);
		mViewPropertyChanged = true;
	}

	const Vec3& GetPosition() const
	{
		return mTransformation.GetTranslation();
	}

	void SetRotation(const Quat& rot)
	{
		mTransformation.SetRotation(rot);
		mViewPropertyChanged = true;
	}

	const Quat& GetRotation() const{
		return mTransformation.GetRotation();
	}

	Vec3 GetRight() const{
		return mTransformation.GetRight();
	}

	Vec3 GetUp() const{
		return mTransformation.GetUp();
	}

	void SetDirection(const Vec3& dir)
	{
		mTransformation.SetDirection(dir);
		mViewPropertyChanged = true;
	}

	void SetDirectionAndRight(const Vec3& dir, const Vec3& right)
	{
		mTransformation.SetDirectionAndRight(dir, right);
		mViewPropertyChanged = true;
	}

	void SetTransformation(const Vec3& pos, const Quat& rot)
	{
		mTransformation.SetTranslation(pos);
		mTransformation.SetRotation(rot);
		mViewPropertyChanged = true;
	}

	void SetTransformation(const Transformation& t)
	{
		mTransformation = t;
		mViewPropertyChanged = true;
	}

	const Transformation& GetTransformation() const{
		return mTransformation;
	}

	const Vec3 GetDirection() const
	{
		return mTransformation.GetMatrix().Column(1);
	}

	//----------------------------------------------------------------------------
	void SetNearFar(Real n, Real f)
	{
		mNear = n;
		mFar = f;
		mProjPropertyChanged = true;
	}

	//----------------------------------------------------------------------------
	void GetNearFar(Real& n, Real& f) const
	{
		n = mNear;
		f = mFar;
	}

	//----------------------------------------------------------------------------
	void ProcessInputData()
	{
		auto target = mTarget.lock();
		if (!mProcessInput || !mCurrentCamera || !target)
			return;
		if (mUserParams.Changed() || mPrevTargetPos != target->GetPosition())
		{
			mInternalParams.dist += mUserParams.dDist;
			mInternalParams.dist = std::max((Real)2.0f, (Real)mInternalParams.dist);
			if (mInternalParams.dist > 300.0)
				mInternalParams.dist = 300.0;

			mInternalParams.pitch += mUserParams.dPitch;
			if (mInternalParams.pitch > fb::HALF_PI - fb::Radian(5))
			{
				mInternalParams.pitch = fb::HALF_PI - fb::Radian(5);
			}
			else if (mInternalParams.pitch <  -fb::HALF_PI + fb::Radian(5))
			{
				mInternalParams.pitch = -fb::HALF_PI + fb::Radian(5);
			}

			mInternalParams.yaw += mUserParams.dYaw;
			if (mInternalParams.yaw > fb::TWO_PI)
			{
				mInternalParams.yaw -= fb::TWO_PI;
			}
			else if (mInternalParams.yaw < -fb::TWO_PI)
			{
				mInternalParams.yaw += fb::TWO_PI;
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
			Vec3 pos = target->GetPosition() + toCam * mInternalParams.dist;
			SetTransformation(pos, Quat(rot));
			mUserParams.Clear();
			mPrevTargetPos = target->GetPosition();
		}
	}

	//----------------------------------------------------------------------------
	void Update()
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
			mMatrices[View] = fb::MakeViewMatrix(pos, right, forward, up);			
			mTransformation.GetHomogeneous(mMatrices[InverseView]);
		}

		bool projChanged = mProjPropertyChanged;
		if (mProjPropertyChanged)
		{
			mAspectRatio = mWidth / (Real)mHeight;
			mProjPropertyChanged = false;
			if (!mOrthogonal)
			{
				mMatrices[Proj] = MakeProjectionMatrix(mFov, mAspectRatio, mNear, mFar);
			}
			else
			{
				mMatrices[Proj] = MakeOrthogonalMatrix(-mWidth*.5f, mHeight*.5f, mWidth*.5f, -mHeight*.5f, mNear, mFar);
			}
			if (mYZSwap)
			{
				Mat44 swapMat(
					1, 0, 0, 0,
					0, 0, 1, 0,
					0, 1, 0, 0,
					0, 0, 0, 1);
				mMatrices[Proj] = mMatrices[Proj] * swapMat;
			}
			mMatrices[InverseProj] = mMatrices[Proj].Inverse();			
		}

		if (projChanged || viewChanged)
		{
			
			mMatrices[ViewProj] = mMatrices[Proj] * mMatrices[View];
			mMatrices[InverseViewProj] = mMatrices[ViewProj].Inverse();

			UpdateFrustum();

			if (viewChanged && !mSelf->mObservers_.empty()){
				auto& observers = mSelf->mObservers_[TransformChanged];
				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->OnViewMatrixChanged();
				}				
			}
			if (projChanged && !mSelf->mObservers_.empty()){
				auto& observers = mSelf->mObservers_[TransformChanged];
				for (auto it = observers.begin(); it != observers.end(); /**/){
					auto observer = it->lock();
					if (!observer){
						it = observers.erase(it);
						continue;
					}
					++it;
					observer->OnProjMatrixChanged();
				}
			}			
		}
	}

	//----------------------------------------------------------------------------
	void UpdateFrustum()
	{
		const auto& viewProjMat = mMatrices[ViewProj];
		mPlanes[FRUSTUM_PLANE_LEFT].mNormal.x = viewProjMat[3][0] + viewProjMat[0][0];
		mPlanes[FRUSTUM_PLANE_LEFT].mNormal.y = viewProjMat[3][1] + viewProjMat[0][1];
		mPlanes[FRUSTUM_PLANE_LEFT].mNormal.z = viewProjMat[3][2] + viewProjMat[0][2];
		mPlanes[FRUSTUM_PLANE_LEFT].mConstant = -(viewProjMat[3][3] + viewProjMat[0][3]);

		mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.x = viewProjMat[3][0] - viewProjMat[0][0];
		mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.y = viewProjMat[3][1] - viewProjMat[0][1];
		mPlanes[FRUSTUM_PLANE_RIGHT].mNormal.z = viewProjMat[3][2] - viewProjMat[0][2];
		mPlanes[FRUSTUM_PLANE_RIGHT].mConstant = -(viewProjMat[3][3] - viewProjMat[0][3]);

		mPlanes[FRUSTUM_PLANE_TOP].mNormal.x = viewProjMat[3][0] - viewProjMat[1][0];
		mPlanes[FRUSTUM_PLANE_TOP].mNormal.y = viewProjMat[3][1] - viewProjMat[1][1];
		mPlanes[FRUSTUM_PLANE_TOP].mNormal.z = viewProjMat[3][2] - viewProjMat[1][2];
		mPlanes[FRUSTUM_PLANE_TOP].mConstant = -(viewProjMat[3][3] - viewProjMat[1][3]);

		mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.x = viewProjMat[3][0] + viewProjMat[1][0];
		mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.y = viewProjMat[3][1] + viewProjMat[1][1];
		mPlanes[FRUSTUM_PLANE_BOTTOM].mNormal.z = viewProjMat[3][2] + viewProjMat[1][2];
		mPlanes[FRUSTUM_PLANE_BOTTOM].mConstant = -(viewProjMat[3][3] + viewProjMat[1][3]);

		mPlanes[FRUSTUM_PLANE_NEAR].mNormal.x = viewProjMat[3][0] + viewProjMat[2][0];
		mPlanes[FRUSTUM_PLANE_NEAR].mNormal.y = viewProjMat[3][1] + viewProjMat[2][1];
		mPlanes[FRUSTUM_PLANE_NEAR].mNormal.z = viewProjMat[3][2] + viewProjMat[2][2];
		mPlanes[FRUSTUM_PLANE_NEAR].mConstant = -(viewProjMat[3][3] + viewProjMat[2][3]);

		mPlanes[FRUSTUM_PLANE_FAR].mNormal.x = viewProjMat[3][0] - viewProjMat[2][0];
		mPlanes[FRUSTUM_PLANE_FAR].mNormal.y = viewProjMat[3][1] - viewProjMat[2][1];
		mPlanes[FRUSTUM_PLANE_FAR].mNormal.z = viewProjMat[3][2] - viewProjMat[2][2];
		mPlanes[FRUSTUM_PLANE_FAR].mConstant = -(viewProjMat[3][3] - viewProjMat[2][3]);

		// Renormalise any normals which were not unit length
		for (int i = 0; i<6; i++)
		{
			Real length = mPlanes[i].mNormal.Normalize();
			mPlanes[i].mConstant /= length;
		}
	}

	const Mat44& GetMatrix(MatrixType type){
		return mMatrices[type];
	}
	
	//----------------------------------------------------------------------------
	bool IsCulled(BoundingVolume* pBV) const
	{
		for (int i = 0; i<6; i++)
		{
			if (pBV->WhichSide(mPlanes[i])<0)
				return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------
	Ray3 ScreenPosToRay(long x, long y)
	{
		Update();

		Real fx = 2.0f * x / mWidth - 1.0f;
		Real fy = 1.0f - 2.0f * y / mHeight;
		Vec3 screenPos((Real)fx, (Real)fy, -1.0f);
		Vec3 screenMidPos((Real)fx, (Real)fy, 0.0f);
		Vec3 origin = mMatrices[InverseViewProj]* screenPos;
		Vec3 target = mMatrices[InverseViewProj] * screenMidPos;
		Vec3 dir = target - origin;
		dir.Normalize();

		Ray3 ray(origin, dir);
		return ray;
	}

	Vec2I WorldToScreen(const Vec3& worldPos) const
	{
		auto projPos = mMatrices[ViewProj] * Vec4(worldPos, 1);
		Real x = projPos.x / projPos.w;
		Real y = projPos.y / projPos.w;
		int ix = Round((x * .5f + .5f) * mWidth);
		int iy = Round((-y*.5f + .5f) * mHeight);
		return Vec2I(ix, iy);
	}

	void SetYZSwap(bool enable){		
		mYZSwap = enable; mProjPropertyChanged = true;
	}

	//----------------------------------------------------------------------------
	void SetDistanceFromTarget(Real dist){
		mInternalParams.dist = dist;
		mUserParams.forceChanged = true;
	}

	void SetTarget(ISpatialObjectPtr obj)
	{
		if (mTarget.lock() == obj)
			return;

		mTarget = obj;
	}

	//---------------------------------------------------------------------------
	void ConsumeInput(IInputInjectorPtr injector)
	{
		if (!mCurrentCamera)
			return;
		auto target = mTarget.lock();
		if (!mProcessInput || !target)
			return;
		if (injector->IsValid(InputDevice::Mouse) && !injector->IsKeyDown(VK_CONTROL)){
			const Vec3 camPos = GetPosition();
			Vec3 toCam = camPos - target->GetPosition();
			const Real distToTarget = toCam.Normalize();
			long dx, dy;
			injector->GetDeltaXY(dx, dy);

			if (injector->IsLButtonDown())
			{
				Real mouseSens = injector->GetSensitivity();
				if (dx != 0)
				{
					mUserParams.dYaw = dx * mouseSens;
				}

				if (dy != 0)
				{
					mUserParams.dPitch = -dy * mouseSens;
				}

				injector->LockMousePos(true, this);
				injector->Invalidate(InputDevice::Mouse);
			}
			else
			{
				//pMouse->LockMousePos(false, this);
			}

			long wheel = injector->GetWheel();
			if (wheel)
			{
				injector->PopWheel();
				Real shift = 1.0f;
				if (injector->IsKeyDown(VK_SHIFT))
					shift = 0.1f;
				Real wheelSens = injector->GetWheelSensitivity();
				Real numLinesSens = wheelSens * (Real)injector->GetNumLinesWheelScroll();
				numLinesSens *= std::max((Real)1.0f, (Real)(mInternalParams.dist * 0.05f));
				mUserParams.dDist += -wheel * numLinesSens * shift;
				injector->Invalidate(InputDevice::Mouse);
			}
		}
	}

	void SetEnalbeInput(bool enable)
	{
		mProcessInput = enable;
	}

	void SetInitialDistToTarget(Real dist)
	{
		mInternalParams.dist = dist;
	}

	void SetFOV(Real fov)
	{
		mFov = fov;
		mTanHalfFOV = tan(mFov*.5f);
		mProjPropertyChanged = true;
	}

	void SetAspectRatio(Real ar) {		
		mAspectRatio = ar; mProjPropertyChanged = true;
	}

	void SetWidth(Real width) {
		mWidth = width; mProjPropertyChanged = true;
	}

	void SetHeight(Real height) {		
		mHeight = height; mProjPropertyChanged = true;
	}
};

//----------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(Camera);

Camera::Camera()
	: mImpl(new Impl(this)){
	
}

Camera::Camera(const Camera& other)
	: mImpl(new Impl(this))
{
	*mImpl = *other.mImpl;
}

Camera::~Camera(){	
}

Camera& Camera::operator= (const Camera& other)
{
	*mImpl = *other.mImpl;
	return *this;
}

void Camera::SetMainCamera(bool main){
	mImpl->mMainCamera = main;
}

bool Camera::IsMainCamera() const{
	return mImpl->mMainCamera;
}

//----------------------------------------------------------------------------
void Camera::SetOrthogonal(bool ortho){
	mImpl->SetOrthogonal(ortho);
}

void Camera::SetPosition(const Vec3& pos){
	mImpl->SetPosition(pos);
}

const Vec3& Camera::GetPosition() const{
	return mImpl->GetPosition();
}

void Camera::SetRotation(const Quat& rot){
	mImpl->SetRotation(rot);
}

const Quat& Camera::GetRotation() const{
	return mImpl->GetRotation();
}

Vec3 Camera::GetRight() const{
	return mImpl->GetRight();
}

Vec3 Camera::GetUp() const{
	return mImpl->GetUp();
}

void Camera::SetDirection(const Vec3& dir){
	mImpl->SetDirection(dir);	
}

void Camera::SetDirrectionAndRight(const Vec3& dir, const Vec3& right){
	mImpl->SetDirectionAndRight(dir, right);
}

void Camera::SetTransformation(const Vec3& pos, const Quat& rot){
	mImpl->SetTransformation(pos, rot);
}

void Camera::SetTransformation(const Transformation& t){
	mImpl->SetTransformation(t);
}

const Transformation& Camera::GetTransformation() const{
	return mImpl->GetTransformation();
}

const Vec3 Camera::GetDirection() const{
	return mImpl->GetDirection();
}

//----------------------------------------------------------------------------
void Camera::SetNearFar(Real n, Real f){
	mImpl->SetNearFar(n, f);
}

//----------------------------------------------------------------------------
void Camera::GetNearFar(Real& n, Real& f) const{
	mImpl->GetNearFar(n, f);
}

//----------------------------------------------------------------------------
void Camera::ProcessInputData(){
	mImpl->ProcessInputData();
}

//----------------------------------------------------------------------------
void Camera::Update(){
	mImpl->Update();
}

//----------------------------------------------------------------------------
void Camera::UpdateFrustum(){
	mImpl->UpdateFrustum();
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetMatrix(MatrixType type){
	if (mImpl->mViewPropertyChanged || mImpl->mProjPropertyChanged)
		mImpl->Update();
	return mImpl->GetMatrix(type);
}
//----------------------------------------------------------------------------
bool Camera::IsCulled(BoundingVolume* pBV) const{
	return mImpl->IsCulled(pBV);
}

//----------------------------------------------------------------------------
Ray3 Camera::ScreenPosToRay(long x, long y){
	return mImpl->ScreenPosToRay(x, y);
}

Vec2I Camera::WorldToScreen(const Vec3& worldPos) const{
	return mImpl->WorldToScreen(worldPos);
}

void Camera::SetEnalbeInput(bool enable){
	mImpl->SetEnalbeInput(enable);
}

void Camera::SetInitialDistToTarget(Real dist){
	mImpl->SetInitialDistToTarget(dist);
}

void Camera::SetFOV(Real fov)
{
	mImpl->SetFOV(fov);
}

Real Camera::GetFOV() const {
	return mImpl->mFov; 
}

Real Camera::GetTanHalfFOV() const { 
	return mImpl->mTanHalfFOV; 
}

void Camera::SetAspectRatio(Real ar) { 
	mImpl->SetAspectRatio(ar);
}

Real Camera::GetAspectRatio() const { 
	return mImpl->mAspectRatio; 
}

// width and height of the view volume at the near view-plane
void Camera::SetWidth(Real width) { 
	mImpl->SetWidth(width);
}

void Camera::SetHeight(Real height) { 
	mImpl->SetHeight(height);
}

Real Camera::GetWidth() const { 
	return mImpl->mWidth; 
}

Real Camera::GetHeight() const {
	return mImpl->mHeight; 
}

void Camera::SetName(const char* name) { 
	mImpl->mName = name; 
}

const char* Camera::GetName() const {
	return mImpl->mName.c_str(); 
}

void Camera::SetYZSwap(bool enable){ 
	mImpl->SetYZSwap(enable);
}

void Camera::SetDistanceFromTarget(Real dist){
	mImpl->SetDistanceFromTarget(dist);
}

void Camera::SetTarget(ISpatialObjectPtr pObj){
	mImpl->SetTarget(pObj);
}

ISpatialObjectPtr Camera::GetTarget() const {
	return mImpl->mTarget.lock(); 
}

void Camera::SetCurrent(bool cur) { 
	mImpl->mCurrentCamera = cur; 
}

bool Camera::GetCurrent() const{
	return mImpl->mCurrentCamera;
}

void Camera::SetCameraIndex(size_t idx) {
	mImpl->mCamIndex = idx; 
}

void Camera::ConsumeInput(IInputInjectorPtr injector){
	mImpl->ConsumeInput(injector);
}