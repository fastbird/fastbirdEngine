#include "StdAfx.h"
#include "CameraMan.h"

using namespace fastbird;
//---------------------------------------------------------------------------
CameraMan::CameraMan(fastbird::ICamera* cam)
	: mCamera(cam)
	, mFixedTargetPos(0, 0, 0)
	, mDestTargetPos(0, 0, 0)
	, mCurTargetPos(0, 1, 0)
	, mDir(0, 1.f, 0)
	, mThirdPersonOffset(false)
	, mThirdPerson(false)
	, mLocalPos(0, -10.f, 0)
	, mMinDist(4.0f)
{
	mCamera->SetPos(Vec3(0, -10, 0));
	mUserParams.dDist=0.001f;
	mDestCamPos = mCurCamPos = cam->GetPos();
	mDestCamRot = mCurCamRot = cam->GetRot();
}

CameraMan::~CameraMan()
{
}

void CameraMan::SetTarget(const Vec3& targetPos)
{
	mFixedTargetPos = targetPos;
}

const Vec3& CameraMan::GetPos() const
{
	assert(mCamera);
	return mCamera->GetPos();
}

const Vec3& CameraMan::GetDir() const
{
	return mDir;
}

Vec3 CameraMan::GetTargetPos() const
{
	return mFixedTargetPos;
}

//---------------------------------------------------------------------------
void CameraMan::Update(float elapsedTime)
{
	bool cameraDebug = true;
	if (cameraDebug)
	{
		char buffer[1024];

		Mat44 matTransform;
		mCamera->GetTransform(matTransform);
		const Vec4& row0 = matTransform.Row(0);
		const Vec4& row1 = matTransform.Row(1);
		const Vec4& row2 = matTransform.Row(2);
		const Vec4& row3 = matTransform.Row(3);
		
		sprintf_s(buffer, "Cam matrix[0] = %s", StringConverter::toString(row0, 10, 4).c_str());
		int x = 1100;
		int y = 150;
		
		int yStep = 16;
		gEnv->pRenderer->DrawText(Vec2I(x, y), buffer, Color::White);
		y+=yStep;

		sprintf_s(buffer, "Cam matrix[1] = %s", StringConverter::toString(row1, 10, 4).c_str());
		gEnv->pRenderer->DrawText(Vec2I(x, y), buffer, Color::White);
		y+=yStep;

		sprintf_s(buffer, "Cam matrix[2] = %s", StringConverter::toString(row2, 10, 4).c_str());
		gEnv->pRenderer->DrawText(Vec2I(x, y), buffer, Color::White);
		y+=yStep;

		sprintf_s(buffer, "Cam matrix[3] = %s", StringConverter::toString(row3, 10, 4).c_str());		
		gEnv->pRenderer->DrawText(Vec2I(x, y), buffer, Color::White);
		y+=yStep;
	}
}

//---------------------------------------------------------------------------
void CameraMan::OnInputFromHandler(fastbird::IMouse* pMouse, fastbird::IKeyboard* pKeyboard)
{
	float cameraSpeed = 10.0f;
	float mouseSens = 0.01f;
	float wheelSens = 0.001f;

	float deltaTime =  gEnv->pTimer->GetDeltaTime();
	if (pKeyboard && pKeyboard->IsValid())
	{
		if (pKeyboard->IsKeyDown('W'))
		{
			Vec3 forward = mCamera->GetForward();
			mCamera->SetPos(mCamera->GetPos() + forward * cameraSpeed * deltaTime);
		}
		else if (pKeyboard->IsKeyDown('S'))
		{
			Vec3 backward = -mCamera->GetForward();
			mCamera->SetPos(mCamera->GetPos() + backward * cameraSpeed * deltaTime);
		}

		if (pKeyboard->IsKeyDown('D'))
		{
			Vec3 right = mCamera->GetRight();
			mCamera->SetPos(mCamera->GetPos() + right * cameraSpeed * deltaTime);
		}
		else if (pKeyboard->IsKeyDown('A'))
		{
			Vec3 left = -mCamera->GetRight();
			mCamera->SetPos(mCamera->GetPos() + left * cameraSpeed * deltaTime);
		}
	}

	if (pMouse && pMouse->IsValid() && !pKeyboard->IsKeyDown(VK_CONTROL))
	{
		long dx, dy;
		pMouse->GetHDDeltaXY(dx, dy);
		Transformation t = mCamera->GetTransform();
		t.AddRotation(Quat(-dx*deltaTime, mCamera->GetUp()));
		t.AddRotation(Quat(-dy*deltaTime, mCamera->GetRight()));
		mCamera->SetTransform(t);
	}

	// orbit camera
	//if (pMouse && pMouse->IsValid() && !pKeyboard->IsKeyDown(VK_CONTROL))
	//{
	//	const Vec3 camPos = mCamera->GetPos();
	//	Vec3 toCam = camPos - GetTargetPos();
	//	const float distToTarget = toCam.Normalize();
	//	long dx, dy;
	//	pMouse->GetHDDeltaXY(dx, dy);
	//	//Log("Delta x = %d, Delta y = %d", dx, dy);

	//	float time;
	//	mThirdPerson = pMouse->IsRButtonDown(&time);
	//	mThirdPersonOffset = mThirdPerson && (gEnv->pTimer->GetTime() - time > 0.3f);
	//	if (pMouse->IsLButtonDown() || mThirdPerson)
	//	{			
	//		if (dx!=0)
	//		{
	//			mUserParams.dYaw = dx * mouseSens;
	//		}

	//		if (dy!=0)
	//		{
	//			mUserParams.dPitch = -dy * mouseSens;
	//		}

	//		pMouse->LockMousePos(true);
	//	}
	//	else
	//	{
	//		pMouse->LockMousePos(false);
	//	}

	//	long wheel = pMouse->GetWheel();
	//	if (wheel)
	//	{
	//		float shift = 1.0f;
	//		if (pKeyboard->IsKeyDown(VK_SHIFT))
	//			shift = 0.1f;

	//		float wheelSensitivity = wheelSens * (float)pMouse->GetNumLinesWheelScroll();
	//		wheelSensitivity *= std::max(1.f, mInternalParams.dist * 0.05f);
	//		mUserParams.dDist += -wheel * wheelSensitivity * shift;
	//	}
	//}
}

//---------------------------------------------------------------------------
// TRACKBALL
//---------------------------------------------------------------------------
#define TRACKBALLSIZE 1.1f
void CalcTrackball(fastbird::Quat& outQuat, long prevX, long prevY, long x, long y)
{
	using namespace fastbird;
	Vec2 prev = gEnv->pRenderer->ToNdcPos(fastbird::Vec2I(prevX, prevY));
	Vec2 cur = gEnv->pRenderer->ToNdcPos(fastbird::Vec2I(x, y));
	
	// Calc two points on the trackball
	fastbird::Vec3 point1(prev.x, prev.y, ProjectToSphere(TRACKBALLSIZE, prev.x, prev.y));
	fastbird::Vec3 point2(cur.x, cur.y, ProjectToSphere(TRACKBALLSIZE, cur.x, cur.y));

	// Cross produce of two vectors
	fastbird::Vec3 axis = point2.Cross(point1);
    fastbird::Vec3 distance = point2 - point1;
	float t = (float)(distance.Length() / (2.0*TRACKBALLSIZE));
	axis.Normalize();
    
	while (t > 1.0f)
		t -= 2.0f;
	
	float angle = fastbird::PI * t;

	outQuat.FromAngleAxis(angle, axis);
}
float ProjectToSphere(float trackballRadius, float x, float y)
{
	float z;
	float d = sqrt(x*x + y*y);
    if (d < trackballRadius * fastbird::INV_ROOT_2)
	{    
		/* Inside sphere */
        z = sqrt( trackballRadius*trackballRadius - d*d );
    } 
	else 
	{
		/* On hyperbola */
        float t = trackballRadius / fastbird::ROOT_2;
        z = t*t / d;
    }
    return z;
}