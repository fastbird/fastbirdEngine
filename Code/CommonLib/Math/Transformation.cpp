#include <CommonLib/StdAfx.h>
#include "Transformation.h"

namespace fastbird
{

//----------------------------------------------------------------------------
const Transformation Transformation::IDENTITY;

//----------------------------------------------------------------------------
Transformation::Transformation ()
	: mMat(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)
	, mT(0.0f,0.0f,0.0f)
	, mS(1.0f,1.0f,1.0f)
{
    // default transform is the identity
    mIdentity = true;
    mRSSeperated = true;
    mUniformScale = true;
}

//----------------------------------------------------------------------------
Transformation::~Transformation ()
{
}

Transformation::Transformation(const Quat& q)
:Transformation()
{
	SetRotation(q);

}

//----------------------------------------------------------------------------
void Transformation::MakeIdentity ()
{
	mMat = Mat33::IDENTITY;
    mT = Vec3::ZERO;
    mS = Vec3(1.0f,1.0f,1.0f);
    mIdentity = true;
    mRSSeperated = true;
    mUniformScale = true;
}

//----------------------------------------------------------------------------
void Transformation::MakeUnitScale ()
{
    assert(mRSSeperated);

    mS = Vec3(1.0f,1.0f,1.0f);
    mUniformScale = true;
}

//----------------------------------------------------------------------------
float Transformation::GetNorm () const
{
    if (mRSSeperated)
    {
		float fMax = abs(mS.x);
        if (abs(mS.y) > fMax)
        {
            fMax = abs(mS.y);
        }
        if (abs(mS.z) > fMax)
        {
            fMax = abs(mS.z);
        }
        return fMax;
    }

	// approximation of the maximum scale.
    float fMaxRowSum =
        abs(mMat[0][0]) +
        abs(mMat[0][1]) +
        abs(mMat[0][2]);

    float fRowSum =
        abs(mMat[1][0]) +
        abs(mMat[1][1]) +
        abs(mMat[1][2]);

    if ( fRowSum > fMaxRowSum )
        fMaxRowSum = fRowSum;

    fRowSum =
        abs(mMat[2][0]) +
        abs(mMat[2][1]) +
        abs(mMat[2][2]);

    if (fRowSum > fMaxRowSum)
    {
        fMaxRowSum = fRowSum;
    }

    return fMaxRowSum;
}

//----------------------------------------------------------------------------
void Transformation::SetRotation (const Mat33& r)
{
    mMat = r;
	mR.FromRotationMatrix(r);
    mIdentity = false;
    mRSSeperated = true;
}

//----------------------------------------------------------------------------
void Transformation::SetRotation (const Quat& r)
{
	mR = r;
	r.ToRotationMatrix(mMat);
	mIdentity = false;
	mRSSeperated = true;
}

void Transformation::SetDir(const Vec3& dir)
{
	Vec3 forward = dir;
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
	SetRotation(rot);
}

void Transformation::SetDirAndRight(const Vec3& dir, const Vec3& right)
{
	Vec3 forward = dir;
	Vec3 up = right.Cross(forward);

	up.Normalize();

	Mat33 rot;
	rot.SetColumn(0, right);
	rot.SetColumn(1, forward);
	rot.SetColumn(2, up);
	SetRotation(rot);
}

//----------------------------------------------------------------------------
void Transformation::AddRotation (const Quat& addR)
{
	Quat ret = addR * mR;
	SetRotation(ret);
}

//----------------------------------------------------------------------------
void Transformation::SetTranslation (const Vec3& t)
{
    mT = t;
    mIdentity = false;
}

void Transformation::AddTranslation(const Vec3& addT)
{
	mT +=addT;
    mIdentity = false;
}

//----------------------------------------------------------------------------
void Transformation::SetScale (const Vec3& s)
{
    assert(mRSSeperated && s.x != 0.0f && s.y != 0.0f
        && s.z != 0.0f);

    mS = s;
    mIdentity = false;
	if (s.x == s.y && s.x == s.z)
		mUniformScale = true;
	else
		mUniformScale = false;
}

//----------------------------------------------------------------------------
void Transformation::SetUniformScale (float fScale)
{
    assert(mRSSeperated && fScale != 0.0f);
    mS = Vec3(fScale,fScale,fScale);
    mIdentity = false;
    mUniformScale = true;
}

//----------------------------------------------------------------------------
void Transformation::SetMatrix (const Mat33& mat)
{
    mMat = mat;
    mIdentity = false;
    mRSSeperated = false;
    mUniformScale = false;
}

//----------------------------------------------------------------------------
Vec3 Transformation::ApplyForward (const Vec3& p) const
{
    if (mIdentity)
    {
        // Y = X
        return p;
    }

    if (mRSSeperated)
    {
        // Y = R*S*X + T
        Vec3 kOutput(mS.x*p.x,mS.y*p.y,
            mS.z*p.z);
        kOutput = mMat*kOutput + mT;
        return kOutput;
    }

    // Y = M*X + T
    Vec3 kOutput = mMat*p + mT;
    return kOutput;
}

//----------------------------------------------------------------------------
void Transformation::ApplyForward (int iQuantity, const Vec3* points,
    Vec3* outputs) const
{
    if (mIdentity)
    {
        // Y = X
        size_t uiSize = iQuantity*sizeof(Vec3);
        memcpy(outputs,points,uiSize);
    }
    else
    {
        int i;
        if (mRSSeperated)
        {
            // Y = R*S*X + T
            for (i = 0; i < iQuantity; i++)
            {
                outputs[i].x = mS.x*points[i].x;
                outputs[i].y = mS.y*points[i].y;
                outputs[i].z = mS.z*points[i].z;
                outputs[i] = mMat*outputs[i] + mT;
            }
        }
        else
        {
            // Y = M*X + T
            for (i = 0; i < iQuantity; i++)
            {
                outputs[i] = mMat*points[i] + mT;
            }
        }
    }
}

//----------------------------------------------------------------------------
Vec3 Transformation::ApplyInverse (const Vec3& p) const
{
    if (mIdentity)
    {
        // X = Y
        return p;
    }

    Vec3 outputPoint = p - mT;
    if (mRSSeperated)
    {
        // X = S^{-1}*R^t*(Y - T)
        outputPoint = outputPoint*mMat;
        if (mUniformScale)
        {
            outputPoint /= GetUniformScale();
        }
        else
        {
            outputPoint /= mS;
        }
    }
    else
    {
        // X = M^{-1}*(Y - T)
        outputPoint = mMat.Inverse()*outputPoint;
    }

    return outputPoint;
}

//----------------------------------------------------------------------------
void Transformation::ApplyInverse (int iQuantity, const Vec3* points,
    Vec3* outputs) const
{
    if (mIdentity)
    {
        // X = Y
        size_t uiSize = iQuantity*sizeof(Vec3);
        memcpy(outputs,points,uiSize);
        return;
    }

    Vec3 kDiff;
    int i;
    if (mRSSeperated)
    {
        // X = S^{-1}*R^t*(Y - T)
        if (mUniformScale)
        {
            float fInvScale = 1.0f/GetUniformScale();
            for (i = 0; i < iQuantity; i++)
            {
                kDiff = points[i] - mT;
                outputs[i] = fInvScale*(kDiff*mMat);
            }
        }
        else
        {
            for (i = 0; i < iQuantity; i++)
            {
                kDiff = points[i] - mT;
                outputs[i] = kDiff*mMat;
				outputs[i] /= mS;
            }
        }
    }
    else
    {
        // X = M^{-1}*(Y - T)
        Mat33 kInverse = mMat.Inverse();
        for (i = 0; i < iQuantity; i++)
        {
            kDiff = points[i] - mT;
            outputs[i] = kInverse*kDiff;
        }
    }
}

//----------------------------------------------------------------------------
Ray3 Transformation::ApplyInverse(const Ray3& r) const
{
	if (mIdentity)
    {
        return r;
    }
	Ray3 newray = r;
	Vec3 kDiff;
    if (mRSSeperated)
    {
        // X = S^{-1}*R^t*(Y - T)
        if (mUniformScale)
        {
            float fInvScale = 1.0f/GetUniformScale();
			kDiff = r.GetOrigin() - mT;
			newray.SetOrigin(fInvScale*(kDiff*mMat)); // transposed
			newray.SetDir((r.GetDir() * mMat).NormalizeCopy());
        }
        else
        {
			kDiff = r.GetOrigin() - mT;
			newray.SetOrigin( kDiff*mMat / mS);
			newray.SetDir(r.GetDir() * mMat.Inverse());// transposed
        }
    }
    else
    {
        // X = M^{-1}*(Y - T)
        Mat33 kInverse = mMat.Inverse();
		kDiff = r.GetOrigin() - mT;
		newray.SetOrigin(kInverse*kDiff);
		newray.SetDir((kInverse * r.GetDir()).NormalizeCopy()); // transposed
    }

	return newray;
}

//----------------------------------------------------------------------------
Vec3 Transformation::InvertVector (const Vec3& inputVector) const
{
    if (mIdentity)
    {
        // X = Y
        return inputVector;
    }

    Vec3 outputVector;
    if (mRSSeperated)
    {
        // X = S^{-1}*R^t*Y
        outputVector = inputVector*mMat;
        if (mUniformScale)
        {
            outputVector /= GetUniformScale();
        }
        else
        {
            outputVector /= mS;
        }
    }
    else
    {
        // X = M^{-1}*Y
        outputVector = mMat.Inverse()*inputVector;
    }

    return outputVector;
}

//----------------------------------------------------------------------------
Plane3 Transformation::ApplyForward (const Plane3& inputPlane) const
{
    if (mIdentity)
    {
        return inputPlane;
    }

    Plane3 output;
    if (mRSSeperated)
    {
        if (mUniformScale)
        {
            output.mNormal = mMat*inputPlane.mNormal;
            output.mConstant = GetUniformScale()*inputPlane.mConstant +
                output.mNormal.Dot(mT);
            return output;
        }

        output.mNormal = inputPlane.mNormal;
        output.mNormal /= mS;
        output.mNormal = mMat*output.mNormal;
    }
    else
    {
        Mat33 kInverse = mMat.Inverse();
        output.mNormal = inputPlane.mNormal*kInverse;
    }

    float fInvLength = 1.0f/output.mNormal.Length();
    output.mNormal *= fInvLength;
    output.mConstant = fInvLength*inputPlane.mConstant +
        output.mNormal.Dot(mT);

    return output;
}

//----------------------------------------------------------------------------
void Transformation::Product (const Transformation& rkA,
    const Transformation& rkB)
{
    if (rkA.IsIdentity())
    {
        *this = rkB;
        return;
    }

    if (rkB.IsIdentity())
    {
        *this = rkA;
        return;
    }

    if (rkA.mRSSeperated && rkB.mRSSeperated)
    {
        if (rkA.mUniformScale)
        {
            SetRotation(rkA.mR*rkB.mR);

            SetTranslation(rkA.GetUniformScale()*(
                rkA.mMat*rkB.mT) + rkA.mT);

            if (rkB.IsUniformScale())
            {
                SetUniformScale(rkA.GetUniformScale()*rkB.GetUniformScale());
            }
            else
            {
                SetScale(rkA.GetUniformScale()*rkB.GetScale());
            }

            return;
        }
    }

    // In all remaining cases, the matrix cannot be written as R*S*X+T.
    Mat33 kMA = (rkA.mRSSeperated ?
		rkA.mMat.ScaleAxis(rkA.mS) :
        rkA.mMat);

    Mat33 kMB = (rkB.mRSSeperated ?
        rkB.mMat.ScaleAxis(rkB.mS) :
        rkB.mMat);

    SetMatrix(kMA*kMB);
    SetTranslation(kMA*rkB.mT+rkA.mT);
}

//----------------------------------------------------------------------------
Transformation Transformation::operator* (const Transformation& t) const
{
	Transformation result;
	result.Product(*this, t);
	return result;
}

//----------------------------------------------------------------------------
void Transformation::Inverse (Transformation& rkInverse) const
{
    if (mIdentity)
    {
        rkInverse = *this;
        return;
    }

    if (mRSSeperated)
    {
        if (mUniformScale)
        {
            rkInverse.mMat = GetMatrix().Transpose() / GetUniformScale();
        }
        else
        {
            Mat33 kRS = mMat.ScaleAxis(mS);
            rkInverse.mMat = kRS.Inverse();
        }
    }
    else
    {
        rkInverse.mMat = mMat.Inverse();
    }

    rkInverse.mT = -(rkInverse.mMat*mT);
	if (!mRSSeperated || !mUniformScale || mS.x != 1.0f || mS.y != 1.0f || mS.z != 1.0f)
		rkInverse.mRSSeperated = false;
    rkInverse.mIdentity = false;
    
    rkInverse.mUniformScale = false;
}

//----------------------------------------------------------------------------
void Transformation::GetHomogeneous (Mat44& rkHMatrix) const
{
    if (mRSSeperated)
    {
        rkHMatrix[0][0] = mS[0]*mMat[0][0];
        rkHMatrix[1][0] = mS[0]*mMat[1][0];
        rkHMatrix[2][0] = mS[0]*mMat[2][0];
		rkHMatrix[3][0] = 0.0f;
        
        rkHMatrix[0][1] = mS[1]*mMat[0][1];
        rkHMatrix[1][1] = mS[1]*mMat[1][1];
        rkHMatrix[2][1] = mS[1]*mMat[2][1];
		rkHMatrix[3][1] = 0.0f;
        
        rkHMatrix[0][2] = mS[2]*mMat[0][2];
        rkHMatrix[1][2] = mS[2]*mMat[1][2];
        rkHMatrix[2][2] = mS[2]*mMat[2][2];
		rkHMatrix[3][2] = 0.0f;
    }
    else
    {
        rkHMatrix[0][0] = mMat[0][0];
        rkHMatrix[1][0] = mMat[1][0];
        rkHMatrix[2][0] = mMat[2][0];
        rkHMatrix[3][0] = 0.0f;
        rkHMatrix[0][1] = mMat[0][1];
        rkHMatrix[1][1] = mMat[1][1];
        rkHMatrix[2][1] = mMat[2][1];
        rkHMatrix[3][1] = 0.0f;
        rkHMatrix[0][2] = mMat[0][2];
        rkHMatrix[1][2] = mMat[1][2];
        rkHMatrix[2][2] = mMat[2][2];
        rkHMatrix[3][2] = 0.0f;
    }

    rkHMatrix[0][3] = mT[0];
    rkHMatrix[1][3] = mT[1];
    rkHMatrix[2][3] = mT[2];
    rkHMatrix[3][3] = 1.0f;
}

Vec3 Transformation::GetRight() const
{
	return mMat.Column(0);
}
Vec3 Transformation::GetForward() const
{
	return mMat.Column(1);
}
Vec3 Transformation::GetUp() const
{
	return mMat.Column(2);
}

}