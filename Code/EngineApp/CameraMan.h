#pragma once

class CameraMan
{
public:
	CameraMan(fastbird::ICamera* cam);
	~CameraMan();

	void Update(float elapsedTime);
	void SetTarget(const fastbird::Vec3& targetPos);
	fastbird::Vec3 GetTargetPos() const;
	void OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard);
	const fastbird::Vec3& GetPos() const;
	const fastbird::Vec3& GetDir() const;

private:
	fastbird::ICamera* mCamera;
	fastbird::Vec3 mFixedTargetPos;
	fastbird::Vec3 mLocalPos; // camera position around origin.
	fastbird::Vec3 mDestLocalPos;
	fastbird::Vec3 mCurCamPos;
	fastbird::Quat mCurCamRot;
	fastbird::Vec3 mCurTargetPos;
	fastbird::Vec3 mDestCamPos;
	fastbird::Quat mDestCamRot;
	fastbird::Vec3 mDestTargetPos;
	fastbird::Vec3 mDir;
	float mMinDist;
	bool mThirdPerson;
	bool mThirdPersonOffset;

	struct UserParameters
	{
		UserParameters()
		{
			Clear();
		}
		void Clear()
		{
			dDist = 0.00f, dYaw = 0.f, dPitch = 0.f;
		}

		bool Changed()
		{
			return dDist !=0.f || dYaw != 0.f || dPitch != 0.f;
		}
		float dDist;
		float dYaw;
		float dPitch;

	} mUserParams;

	struct InternalParameters
	{
		InternalParameters()
		{
			dist = 10.0f;
			yaw = 0.f;
			pitch = 0.f;
		}
		float dist;
		float yaw;
		float pitch;
	} mInternalParams;
};

void CalcTrackball(fastbird::Quat& outQuat, long prevX, long prevY, long x, long y);
float ProjectToSphere(float trackballRadius, float x, float y);