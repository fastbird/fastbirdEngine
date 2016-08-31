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
#include "Renderer.h"
#include "FBMathLib/BoundingVolume.h"
#include "FBMathLib/Frustum.h"
#include "FBMathLib/InterpolateWrap.h"
#include "FBInputManager/IInputInjector.h"
#include "FBInputManager/KeyCodes.h"
#include "FBSceneManager/ISpatialObject.h"
#include "FBTimer/Timer.h"
#include "FBStringLib/MurmurHash.h"

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
	CameraWeakPtr mSelfPtr;
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
	Rectf mOrthogonalData;
	std::string mName;	
	ISpatialObjectWeakPtr mTarget;
	Vec3 mTargetPos; /// used when mTarget is nullptr
	Real mMaxDistToTarget;
	Real mMinDistToTarget;
	size_t mCamIndex;
	bool mYZSwap;
	bool mCurrentCamera;
	bool mProcessInput;
	bool mRenderFrustum;
	bool mProportionalMove;
	Vec3 mPrevTargetPos;
	std::mutex mMutex;
	Frustum mFrustum;
	Frustum mFrustumInCameraSpace;
	size_t mFrustumInCameraSpaceCalcFrame = -1;
	CameraPtr mOverridingCamera;
	std::unordered_map<Vec2I, Ray> mRayCache;
	InterpolateWrapManager<Real> mInterpolManReal;
	FRAME_PRECISION mLastUpdateFrame;

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
		, mPrevTargetPos(0, 0, 0)
		, mTargetPos(0, 0, 0)
		, mMaxDistToTarget(FLT_MAX)
		, mMinDistToTarget(0)
		, mProportionalMove(false)
		, mRenderFrustum(false)
		, mLastUpdateFrame(-1)
	{

		// proj properties
		SetFOV(Radian(45));
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
		mOrthogonalData = other.mOrthogonalData;
		mName = other.mName;		
		mTarget = other.mTarget;
		mCamIndex = other.mCamIndex;
		mYZSwap = other.mYZSwap;
		mCurrentCamera = other.mCurrentCamera;
		mProcessInput = other.mProcessInput;
		mPrevTargetPos = other.mPrevTargetPos;		
		mFrustum = other.mFrustum;
		mFrustumInCameraSpace = other.mFrustumInCameraSpace;
		mFrustumInCameraSpaceCalcFrame = other.mFrustumInCameraSpaceCalcFrame;
		mRayCache = other.mRayCache;
		mInternalParams = other.mInternalParams;
		mMaxDistToTarget = other.mMaxDistToTarget;
		mMinDistToTarget = other.mMinDistToTarget;
		mProportionalMove = other.mProportionalMove;
		return *this;
	}

	//----------------------------------------------------------------------------
	void SetOrthogonal(bool ortho)
	{		
		if (mOverridingCamera){
			mOverridingCamera->SetOrthogonal(ortho);
			return;
		}
		mOrthogonal = ortho;
		mFrustum.SetOrthogonal(ortho);
		mProjPropertyChanged = true;
	}

	void SetPosition(const Vec3& pos)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetPosition(pos);
			return;
		}
		if (mTransformation.GetTranslation() == pos)
			return;
		mTransformation.SetTranslation(pos);
		mViewPropertyChanged = true;
	}

	const Vec3& GetPosition() const
	{
		if (mOverridingCamera){
			return mOverridingCamera->GetPosition();
		}
		return mTransformation.GetTranslation();
	}

	void SetRotation(const Quat& rot)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetRotation(rot);
			return;
		}
		mTransformation.SetRotation(rot);
		mViewPropertyChanged = true;
	}

	const Quat& GetRotation() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetRotation();
		}
		return mTransformation.GetRotation();
	}

	Vec3 GetRight() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetRight();
		}
		return mTransformation.GetRight();
	}

	Vec3 GetUp() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetUp();
		}
		return mTransformation.GetUp();
	}

	void SetDirection(const Vec3& dir)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetDirection(dir);
			return;
		}
		if (mTransformation.GetForward() == dir)
			return;

		mTransformation.SetDirection(dir);
		mViewPropertyChanged = true;
	}

	void SetDirectionAndRight(const Vec3& dir, const Vec3& right)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetDirrectionAndRight(dir, right);
			return;
		}
		mTransformation.SetDirectionAndRight(dir, right);
		mViewPropertyChanged = true;
	}

	void SetTransformation(const Vec3& pos, const Quat& rot)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetTransformation(pos, rot);
			return;
		}
		mTransformation.SetTranslation(pos);
		mTransformation.SetRotation(rot);
		mViewPropertyChanged = true;
	}

	void SetTransformation(const Transformation& t)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetTransformation(t);
			return;
		}
		mTransformation = t;
		mViewPropertyChanged = true;
	}

	const Transformation& GetTransformation() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetTransformation();
		}
		return mTransformation;
	}

	const Vec3 GetDirection() const
	{
		if (mOverridingCamera){
			return mOverridingCamera->GetDirection();
		}
		return mTransformation.GetMatrix().Column(1);
	}

	//----------------------------------------------------------------------------
	void SetNearFar(Real n, Real f)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetNearFar(n, f);
			return;
		}
		mNear = n;
		mFar = f;
		mProjPropertyChanged = true;
	}

	//----------------------------------------------------------------------------
	void GetNearFar(Real& n, Real& f) const
	{
		if (mOverridingCamera){
			mOverridingCamera->GetNearFar(n, f);
			return;
		}
		n = mNear;
		f = mFar;
	}

	Real GetNear() const
	{
		return mNear;
	}

	Real GetFar() const
	{
		return mFar;
	}

	void SetNear(Real n)
	{
		mNear = n;		
		mProjPropertyChanged = true;
	}

	void SetFar(Real f)
	{
		mFar = f;
		mProjPropertyChanged = true;
	}

	Real GetWidth() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetWidth();
		}
		return (float)(mOrthogonalData.right - mOrthogonalData.left);
	}

	Real GetHeight() const{
		if (mOverridingCamera){
			return mOverridingCamera->GetHeight();
		}
		return (float)(mOrthogonalData.top - mOrthogonalData.bottom);
	}

	struct AngleClamper {
		void operator()(Real& value) {
			if (value > fb::TWO_PI) {
				value -= fb::TWO_PI;
			}
			else if (value < -fb::TWO_PI) {
				value += fb::TWO_PI;
			}
		}
	};
	//----------------------------------------------------------------------------
	void ProcessInputData()
	{
		static AngleClamper clamper;
		if (mOverridingCamera) {
			mOverridingCamera->ProcessInputData();
			return;
		}
		auto target = mTarget.lock();
		auto& targetPos = target ? target->GetPosition() : mTargetPos;
		if (!mProcessInput || !mCurrentCamera)
			return;
		static const float ReachTime = 0.2f;
		if (mUserParams.Changed() || mPrevTargetPos != targetPos)
		{			
			auto destDist = ClampRet(std::max((Real)2.0f, (Real)(mInterpolManReal.GetDestValue(&mInternalParams.dist) + mUserParams.dDist)),
				mMinDistToTarget, mMaxDistToTarget);
			mInterpolManReal.RegisterInterpolator(&mInternalParams.dist, destDist, ReachTime*2.0f);

			auto destPitch = ClampRet(mInterpolManReal.GetDestValue(&mInternalParams.pitch) + mUserParams.dPitch,
				-fb::HALF_PI + fb::Radian(5), fb::HALF_PI - fb::Radian(5));
			mInterpolManReal.RegisterInterpolator(&mInternalParams.pitch, destPitch, ReachTime);

			auto destYaw = mInterpolManReal.GetDestValue(&mInternalParams.yaw) + mUserParams.dYaw;			
			mInterpolManReal.RegisterInterpolator(&mInternalParams.yaw, destYaw, ReachTime, clamper);
			mUserParams.Clear();
		}		
	}

	void Update(float dt) {		
		if (mLastUpdateFrame == gpTimer->GetFrame())
			return;
		mLastUpdateFrame = gpTimer->GetFrame();
		if (!mInterpolManReal.IsEmpty()) {
			
			auto target = mTarget.lock();
			auto& targetPos = target ? target->GetPosition() : mTargetPos;
			mInterpolManReal.Update(gpTimer->GetDeltaTime());

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
			Vec3 pos = targetPos + toCam * mInternalParams.dist;
			SetTransformation(pos, Quat(rot));
			mPrevTargetPos = targetPos;
		}
	}
	//----------------------------------------------------------------------------
	void RefreshTransform()
	{
		if (mOverridingCamera){
			mOverridingCamera->RefreshTransform();
		}
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
			mFrustum.mOrigin = mTransformation.GetTranslation();
			mFrustum.mOrientation = mTransformation.GetRotation();
		}

		bool projChanged = mProjPropertyChanged;
		if (mProjPropertyChanged)
		{
			mAspectRatio = GetWidth() / (Real)GetHeight();
			mProjPropertyChanged = false;
			if (!mOrthogonal)
			{
				mMatrices[ProjBeforeSwap] = mMatrices[Proj] = 
					MakeProjectionMatrix(mFov, mAspectRatio, mNear, mFar);
			}
			else
			{
				mMatrices[ProjBeforeSwap] = mMatrices[Proj] = 
					MakeOrthogonalMatrix((Real)mOrthogonalData.left, (Real)mOrthogonalData.top,
					(Real)mOrthogonalData.right, (Real)mOrthogonalData.bottom,
					mNear, mFar);
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
			// if Orthogonal, fov is ignored.
			mFrustum.SetData(mNear, mFar, mFov, mAspectRatio, mOrthogonal);
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
			mRayCache.clear();
		}
	}

	//----------------------------------------------------------------------------
	void UpdateFrustum()
	{
		if (mOverridingCamera){
			mOverridingCamera->UpdateFrustum();
		}
		const auto& viewProjMat = mMatrices[ViewProj];
		mFrustum.UpdatePlaneWithViewProjMat(viewProjMat);
	}

	const Mat44& GetMatrix(MatrixType type){
		if (mOverridingCamera){
			return mOverridingCamera->GetMatrix(type);
		}

		if (mViewPropertyChanged || mProjPropertyChanged)
			RefreshTransform();
		return mMatrices[type];
	}
	
	//----------------------------------------------------------------------------
	bool IsCulled(BoundingVolume* pBV) const
	{
		return mFrustum.IsCulled(pBV);		
	}

	//----------------------------------------------------------------------------
	Ray ScreenPosToRay(long x, long y)
	{
		RefreshTransform();
		auto it = mRayCache.find(Vec2I(x, y));
		if (it != mRayCache.end()){
			return it->second;
		}

		Real fx = 2.0f * x / GetWidth() - 1.0f;
		Real fy = 1.0f - 2.0f * y / GetHeight();
		Vec3 screenPos((Real)fx, (Real)fy, -1.0f);
		Vec3 screenMidPos((Real)fx, (Real)fy, 0.0f);
		Vec3 origin = mMatrices[InverseViewProj]* screenPos;
		Vec3 target = mMatrices[InverseViewProj] * screenMidPos;
		Vec3 dir = target - origin;
		dir.Normalize();

		Ray ray(mTransformation.GetTranslation(), dir);
		mRayCache[Vec2I(x, y)] = ray;
		return ray;
	}

	Vec3 ScreenToNDC(const Vec2I& screenPos)
	{
		Real fx = 2.0f * screenPos.x / GetWidth() - 1.0f;
		Real fy = 1.0f - 2.0f * screenPos.y / GetHeight();
		return Vec3((Real)fx, (Real)fy, 0.5f);		
	}

	Vec2I WorldToScreen(const Vec3& worldPos)
	{
		RefreshTransform();
		auto projPos = mMatrices[ViewProj] * Vec4(worldPos, 1);
		Real x = projPos.x / projPos.w;
		Real y = projPos.y / projPos.w;
		int ix = Round((x * .5f + .5f) * GetWidth());
		int iy = Round((-y*.5f + .5f) * GetHeight());
		return Vec2I(ix, iy);
	}

	void SetYZSwap(bool enable){		
		if (mOverridingCamera){
			return mOverridingCamera->SetYZSwap(enable);
		}
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
		if (mOverridingCamera)
		{
			mOverridingCamera->ConsumeInput(injector);
			return;
		}

		if (!mCurrentCamera)
			return;
		auto target = mTarget.lock();
		auto& targetPos = target ? target->GetPosition() : mTargetPos;
		if (!mProcessInput)
			return;
		float proportional = 1.f;
		long wheel = injector->GetWheel();
		if (mProportionalMove) {
			proportional = (mInternalParams.dist - mMinDistToTarget) / (mMaxDistToTarget - mMinDistToTarget);				
			if (!injector->IsKeyDown(VK_SHIFT))
				proportional *= 2;
			else
				proportional *= 4;
			Clamp(proportional, 0.0012f, 1.0f);
			
		}
		if (injector->IsValid(InputDevice::Mouse) && !injector->IsKeyDown(VK_CONTROL)){			
			long dx, dy;
			injector->GetDpiDependentDeltaXY(dx, dy);

			if (injector->IsLButtonDown())
			{
				Real mouseSens = injector->GetSensitivity();
				if (dx != 0)
				{
					mUserParams.dYaw = dx * mouseSens * proportional;
				}

				if (dy != 0)
				{
					mUserParams.dPitch = -dy * mouseSens * proportional;
				}

				injector->LockMousePos(true, this);
				injector->Invalidate(InputDevice::Mouse);
			}
			else
			{
				//pMouse->LockMousePos(false, this);
			}

			
			if (wheel)
			{
				injector->PopWheel();
				Real shift = 1.0f;
				if (!mProportionalMove && injector->IsKeyDown(VK_SHIFT))
					shift = 0.1f;
				Real wheelSens = injector->GetWheelSensitivity();
				Real numLinesSens = wheelSens * (Real)injector->GetNumLinesWheelScroll();
				numLinesSens *= std::max((Real)1.0f, (Real)(mInternalParams.dist * 0.05f));
				mUserParams.dDist += -wheel * numLinesSens * shift  * proportional;
				injector->Invalidate(InputDevice::Mouse);
			}
		}
	}

	const Frustum& GetFrustum(){
		RefreshTransform();
		return mFrustum;
	}

	const Frustum& GetFrustumLocal()
	{
		if (mFrustumInCameraSpaceCalcFrame == gpTimer->GetFrame())
		{
			return mFrustumInCameraSpace;
		}

		mFrustumInCameraSpace = mFrustum.TransformBy(mMatrices[ICamera::MatrixType::View]);
		mFrustumInCameraSpaceCalcFrame = gpTimer->GetFrame();
		return mFrustumInCameraSpace;		
	}

	void SetOverridingCamera(CameraPtr cam){
		mOverridingCamera = cam;
	}

	Real ComputePixelSizeAtDistance(Real distance) {
		// If the viewport width is zero, than replace it with 1, which effectively ignores the viewport width.
		Real viewportWidth = GetWidth();
		Real pixelSizeScale = 2.f * mTanHalfFOV / (viewportWidth <= 0 ? 1 : viewportWidth);

		return std::abs(distance) * pixelSizeScale;
	}

	void SetEnalbeInput(bool enable)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetEnalbeInput(enable);
			return;
		}
		mProcessInput = enable;
		if (enable) {
			mPrevTargetPos = mTargetPos;
			mInternalParams.dist = mTargetPos.DistanceTo(mTransformation.GetTranslation());
		}
	}

	void SetInitialDistToTarget(Real dist)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetInitialDistToTarget(dist);
			return;
		}
		mInternalParams.dist = dist;
	}

	void SetMaxDistToTarget(Real dist)
	{
		mMaxDistToTarget = dist;
	}

	void SetMinDistToTarget(Real dist) {
		mMinDistToTarget = dist;
	}

	void SetProportionalMove(bool enable) {
		mProportionalMove = enable;
	}

	void SetFOV(Real fov)
	{
		if (mOverridingCamera){
			mOverridingCamera->SetFOV(fov);
			return;
		}
		mFov = fov;
		mTanHalfFOV = tan(mFov*.5f);		
		mProjPropertyChanged = true;
	}

	void SetAspectRatio(Real ar) {		
		if (mOverridingCamera){
			mOverridingCamera->SetAspectRatio(ar);
			return;
		}
		mAspectRatio = ar; mProjPropertyChanged = true;
	}

	void SetWidth(Real width) {
		if (mOverridingCamera){
			mOverridingCamera->SetWidth(width);
			return;
		}
		mOrthogonalData.left = -width * .5f;
		mOrthogonalData.right = width * .5f;
		mProjPropertyChanged = true;
	}

	void SetHeight(Real height) {		
		if (mOverridingCamera){
			mOverridingCamera->SetHeight(height);
			return;
		}
		mOrthogonalData.top = height*.5f;
		mOrthogonalData.bottom = -height*.5f;
		mProjPropertyChanged = true;
	}

	void SetOrthogonalData(float left, float top, float right, float bottom){
		if (mOverridingCamera){
			mOverridingCamera->SetOrthogonalData(left, top, right, bottom);
			return;
		}
		mOrthogonalData.left = left;
		mOrthogonalData.top = top;
		mOrthogonalData.right = right;
		mOrthogonalData.bottom = bottom;
		mProjPropertyChanged = true;
	}

	void RenderFrustum()
	{
		Renderer::GetInstance().DrawFrustum(mFrustum);
	}

	size_t ComputeHash() const {
		size_t h = mTransformation.ComputeHash();
		hash_combine(h, std::hash<float>()(mFov));
		hash_combine(h, std::hash<float>()(mNear));
		hash_combine(h, std::hash<float>()(mFar));
		hash_combine(h, std::hash<float>()(mAspectRatio));
		if (mOrthogonal) {
			hash_combine(h, (size_t)murmur3_32((const char*)&mOrthogonalData, sizeof(mOrthogonalData)));
		}		
		return h;
	}
};

//----------------------------------------------------------------------------
CameraPtr Camera::Create() {
	auto p = CameraPtr(new Camera, [](Camera* obj) {
		delete obj;
	});
	p->mImpl->mSelfPtr = p;
	return p;
}

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

CameraPtr Camera::Clone()
{
	return CameraPtr(new Camera(*this), [](Camera* obj) {delete obj; });	
}

void Camera::CloneTo(const ICameraPtr& icam) {
	auto cam =  std::dynamic_pointer_cast<Camera>(icam);
	*cam->mImpl = *mImpl;
}

void Camera::SetMainCamera(bool main){
	mImpl->mMainCamera = main;
}

bool Camera::IsMainCamera() const{
	return mImpl->mMainCamera;
}

ICameraPtr Camera::GetSelfPtr() {
	return mImpl->mSelfPtr.lock();
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

Real Camera::GetNear() const
{
	return mImpl->GetNear();
}

Real Camera::GetFar() const
{
	return mImpl->GetFar();
}

void Camera::SetNear(Real n)
{
	mImpl->SetNear(n);
}

void Camera::SetFar(Real f)
{
	mImpl->SetFar(f);
}

//----------------------------------------------------------------------------
void Camera::ProcessInputData(){
	mImpl->ProcessInputData();
}

void Camera::Update(float dt) {
	mImpl->Update(dt);
}

//----------------------------------------------------------------------------
void Camera::RefreshTransform(){
	mImpl->RefreshTransform();
}

//----------------------------------------------------------------------------
void Camera::UpdateFrustum(){
	mImpl->UpdateFrustum();
}

//----------------------------------------------------------------------------
const Mat44& Camera::GetMatrix(MatrixType type){
	return mImpl->GetMatrix(type);
}

//----------------------------------------------------------------------------
bool Camera::IsCulled(BoundingVolume* pBV) const{
	return mImpl->IsCulled(pBV);
}

//----------------------------------------------------------------------------
Ray Camera::ScreenPosToRay(long x, long y){
	return mImpl->ScreenPosToRay(x, y);
}

Vec3 Camera::ScreenToNDC(const Vec2I& screenPos)
{
	return mImpl->ScreenToNDC(screenPos);
}

Vec2I Camera::WorldToScreen(const Vec3& worldPos){
	return mImpl->WorldToScreen(worldPos);
}

void Camera::SetEnalbeInput(bool enable){
	mImpl->SetEnalbeInput(enable);
}

void Camera::SetInitialDistToTarget(Real dist){
	mImpl->SetInitialDistToTarget(dist);
}

void Camera::SetMaxDistToTarget(Real dist)
{
	mImpl->SetMaxDistToTarget(dist);
}

void Camera::SetMinDistToTarget(Real dist) {
	mImpl->SetMinDistToTarget(dist);
}

void Camera::SetProportionalMove(bool enable) {
	mImpl->SetProportionalMove(enable);
}

void Camera::SetFOV(Real fov)
{
	mImpl->SetFOV(fov);
}

Real Camera::GetFOV() const {
	return mImpl->mFov; 
}

Real Camera::GetAspectRatio() const {
	return mImpl->mAspectRatio;
}

Real Camera::GetTanHalfFOV() const { 
	return mImpl->mTanHalfFOV; 
}

void Camera::SetAspectRatio(Real ar) { 
	mImpl->SetAspectRatio(ar);
}

// width and height of the view volume at the near view-plane
void Camera::SetWidth(Real width) { 
	mImpl->SetWidth(width);
}

void Camera::SetHeight(Real height) { 
	mImpl->SetHeight(height);
}

Real Camera::GetWidth() const { 
	return mImpl->GetWidth();
}

Real Camera::GetHeight() const {
	return mImpl->GetHeight();
}

void Camera::SetOrthogonalData(float left, float top, float right, float bottom){
	mImpl->SetOrthogonalData(left, top, right, bottom);
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

const Frustum& Camera::GetFrustum(){
	return mImpl->GetFrustum();
}

const Frustum& Camera::GetFrustumLocal()
{
	return mImpl->GetFrustumLocal();
}

void Camera::SetOverridingCamera(CameraPtr cam){
	mImpl->SetOverridingCamera(cam);
}

CameraPtr Camera::GetOverridingCamera() const{
	return mImpl->mOverridingCamera;
}

Real Camera::ComputePixelSizeAtDistance(Real distance) {
	return mImpl->ComputePixelSizeAtDistance(distance);
}

void Camera::SetTargetPos(const Vec3& pos) {
	mImpl->mTargetPos = pos;
	mImpl->mTarget.reset();
}

void Camera::SetRenderFrustum(bool renderFrustum)
{
	mImpl->mRenderFrustum = renderFrustum;
}

bool Camera::GetRenderFrustum() const
{
	return mImpl->mRenderFrustum;
}

void Camera::RenderFrustum()
{
	mImpl->RenderFrustum();
}

size_t Camera::ComputeHash() const {
	return mImpl->ComputeHash();
}