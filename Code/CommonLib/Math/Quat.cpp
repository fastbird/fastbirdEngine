#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/Quat.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{
	const float Quat::ms_fEpsilon = 1e-03f;
	const Quat Quat::ZERO(0, 0, 0, 0);
	const Quat Quat::IDENTITY(1, 0, 0, 0);

	//-------------------------------------------------------------------------
	void Quat::FromRotationMatrix(const Mat33& rot)
	{
		// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
        // article "Quaternion Calculus and Fast Animation".

        float fTrace = rot[0][0]+rot[1][1]+rot[2][2];
        float fRoot;

        if ( fTrace > 0.0 )
        {
            // |w| > 1/2, may as well choose w > 1/2
            fRoot = sqrt(fTrace + 1.0f);  // 2w
            w = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;  // 1/(4w)
            x = (rot[2][1]-rot[1][2])*fRoot;
            y = (rot[0][2]-rot[2][0])*fRoot;
            z = (rot[1][0]-rot[0][1])*fRoot;
        }
        else
        {
            // |w| <= 1/2
            static size_t s_iNext[3] = { 1, 2, 0 };
            size_t i = 0;
            if ( rot[1][1] > rot[0][0] )
                i = 1;
            if ( rot[2][2] > rot[i][i] )
                i = 2;
            size_t j = s_iNext[i];
            size_t k = s_iNext[j];

            fRoot = sqrt(rot[i][i]-rot[j][j]-rot[k][k] + 1.0f);
            float* apkQuat[3] = { &x, &y, &z };
            *apkQuat[i] = 0.5f*fRoot;
            fRoot = 0.5f/fRoot;
            w = (rot[k][j]-rot[j][k])*fRoot;
            *apkQuat[j] = (rot[j][i]+rot[i][j])*fRoot;
            *apkQuat[k] = (rot[k][i]+rot[i][k])*fRoot;
        }
	}

	//-------------------------------------------------------------------------
	void Quat::ToRotationMatrix(Mat33& rot) const
	{
		float fTx  = x+x;
		float fTy  = y+y;
		float fTz  = z+z;
		float fTwx = fTx*w;
		float fTwy = fTy*w;
		float fTwz = fTz*w;
		float fTxx = fTx*x;
		float fTxy = fTy*x;
		float fTxz = fTz*x;
		float fTyy = fTy*y;
		float fTyz = fTz*y;
		float fTzz = fTz*z;

		rot[0][0] = 1.0f-(fTyy+fTzz);
		rot[0][1] = fTxy-fTwz;
		rot[0][2] = fTxz+fTwy;
		rot[1][0] = fTxy+fTwz;
		rot[1][1] = 1.0f-(fTxx+fTzz);
		rot[1][2] = fTyz-fTwx;
		rot[2][0] = fTxz-fTwy;
		rot[2][1] = fTyz+fTwx;
		rot[2][2] = 1.0f-(fTxx+fTyy);
	}

	//-----------------------------------------------------------------------
	Vec3 Quat::operator* (const Vec3& v) const
	{
		// from nVidia SDK
		Vec3 uv, uuv;
		Vec3 qvec(x, y, z);
		uv = qvec.Cross(v);
		uuv = qvec.Cross(uv);
		uv *= (2.0f * w);
		uuv *= 2.0f;

		return v + uv + uuv;
	}

	//-----------------------------------------------------------------------
    Quat Quat::operator* (float scalar) const
    {
        return Quat(scalar*w,scalar*x,scalar*y,scalar*z);
    }

	//-----------------------------------------------------------------------
    Quat Quat::operator* (const Quat& q) const
    {
        return Quat
        (
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y + y * q.w + z * q.x - x * q.z,
            w * q.z + z * q.w + x * q.y - y * q.x
        );
    }

	//-----------------------------------------------------------------------
	void Quat::FromAngleAxis (float radian,
        const Vec3& axis)
    {
        float fHalfAngle = radian * .5f;
        float fSin = sin(fHalfAngle);
        w = cos(fHalfAngle);
        x = fSin*axis.x;
        y = fSin*axis.y;
        z = fSin*axis.z;
    }

	//-----------------------------------------------------------------------
	void Quat::ToAngleAxis(float& radian, Vec3& axis)
	{
		//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)
        float fSqrLength = x*x+y*y+z*z;
        if ( fSqrLength > 0.0 )
        {
            radian = 2.0f * acos(w);
            float fInvLength = 1.f / sqrt(fSqrLength);
            axis.x = x*fInvLength;
            axis.y = y*fInvLength;
            axis.z = z*fInvLength;
        }
        else
        {
            radian = Radian(0.0);
            axis.x = 1.0;
            axis.y = 0.0;
            axis.z = 0.0;
        }
	}

	//-----------------------------------------------------------------------
	void Quat::FromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis)
	{
		Mat33 rot;

        rot[0][0] = xAxis.x;
        rot[1][0] = xAxis.y;
        rot[2][0] = xAxis.z;

		rot[0][1] = yAxis.x;
        rot[1][1] = yAxis.y;
        rot[2][1] = yAxis.z;

		rot[0][2] = zAxis.x;
        rot[1][2] = zAxis.y;
        rot[2][2] = zAxis.z;

        FromRotationMatrix(rot);
	}

	//-----------------------------------------------------------------------
	void Quat::ToAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis)
	{
		Mat33 kRot;

        ToRotationMatrix(kRot);

        xAxis.x = kRot[0][0];
        xAxis.y = kRot[1][0];
        xAxis.z = kRot[2][0];

		yAxis.x = kRot[0][1];
        yAxis.y = kRot[1][1];
        yAxis.z = kRot[2][1];

		zAxis.x = kRot[0][2];
        zAxis.y = kRot[1][2];
        zAxis.z = kRot[2][2];
	}

	void Quat::FromDirection(const Vec3& dir){
		float angle = Vec3::UNIT_Y.AngleBetween(dir);
		if (IsEqual(angle, 0.f, 0.001f)){
			*this = IDENTITY;
		}
		else{
			auto axis = Vec3::UNIT_Y.Cross(dir).NormalizeCopy();
			FromAngleAxis(angle, axis);
		}
	}

	//-----------------------------------------------------------------------
	float Quat::Norm() const
	{
		return w*w + x*x + y*y + z*z;
	}

	//-----------------------------------------------------------------------
	float Quat::Normalise()
	{
		float len = Norm();
		float factor = 1.f / (float)sqrt(len);
		*this = *this * factor;
		return len;
	}
	
	//-----------------------------------------------------------------------
	bool Quat::Equals(const Quat& rhs, const float toleranceRadian) const
	{
		float fCos = Dot(rhs);
        float angle = ACos(fCos);

		return (abs(angle) <= toleranceRadian) || IsEqual(angle, PI, toleranceRadian);
	}

	//-----------------------------------------------------------------------
	float Quat::Dot(const Quat& other) const
	{
		return w*other.w+x*other.x+y*other.y+z*other.z; 
	}


	inline bool Quat::IsNaN() const
	{
			return fastbird::IsNaN(w) || fastbird::IsNaN(x) ||
				fastbird::IsNaN(y) || fastbird::IsNaN(z);
	}

}