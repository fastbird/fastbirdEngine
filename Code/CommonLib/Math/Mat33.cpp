#include <CommonLib/StdAfx.h>
#include "Mat33.h"
#include "fbMath.h"

using namespace fastbird;

//----------------------------------------------------------------------------
const Mat33 Mat33::IDENTITY(1, 0, 0, 0, 1, 0, 0, 0, 1);
const Mat33 Mat33::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0);

//----------------------------------------------------------------------------
Mat33 Mat33::operator* (const Mat33& rMat) const
{
	Mat33 result;
    for (size_t iRow = 0; iRow < 3; iRow++)
    {
        for (size_t iCol = 0; iCol < 3; iCol++)
        {
            result.m[iRow][iCol] =
                m[iRow][0]*rMat.m[0][iCol] +
                m[iRow][1]*rMat.m[1][iCol] +
                m[iRow][2]*rMat.m[2][iCol];
        }
    }
    return result;
}

//----------------------------------------------------------------------------
Vec3 Mat33::operator* (const Vec3& rVec) const
{
	Vec3 result;
    for (size_t iRow = 0; iRow < 3; iRow++)
    {
        result[iRow] =
            m[iRow][0]*rVec[0] +
            m[iRow][1]*rVec[1] +
            m[iRow][2]*rVec[2];
    }
    return result;
}

//----------------------------------------------------------------------------
void Mat33::FromAxisAngle(const Vec3& axis, float radian)
{
	float fCos = cos(radian);
    float fSin = sin(radian);
    float fOneMinusCos = 1.0f-fCos;
    float fX2 = axis.x*axis.x;
    float fY2 = axis.y*axis.y;
    float fZ2 = axis.z*axis.z;
    float fXYM = axis.x*axis.y*fOneMinusCos;
    float fXZM = axis.x*axis.z*fOneMinusCos;
    float fYZM = axis.y*axis.z*fOneMinusCos;
    float fXSin = axis.x*fSin;
    float fYSin = axis.y*fSin;
    float fZSin = axis.z*fSin;

    m[0][0] = fX2*fOneMinusCos+fCos;
    m[0][1] = fXYM-fZSin;
    m[0][2] = fXZM+fYSin;
    m[1][0] = fXYM+fZSin;
    m[1][1] = fY2*fOneMinusCos+fCos;
    m[1][2] = fYZM-fXSin;
    m[2][0] = fXZM-fYSin;
    m[2][1] = fYZM+fXSin;
    m[2][2] = fZ2*fOneMinusCos+fCos;
}

//----------------------------------------------------------------------------
Mat33 Mat33::Inverse() const
{
    Mat33 inversed;

    inversed.m[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
    inversed.m[0][1] = m[0][2]*m[2][1] - m[0][1]*m[2][2];
    inversed.m[0][2] = m[0][1]*m[1][2] - m[0][2]*m[1][1];
    inversed.m[1][0] = m[1][2]*m[2][0] - m[1][0]*m[2][2];
    inversed.m[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
    inversed.m[1][2] = m[0][2]*m[1][0] - m[0][0]*m[1][2];
    inversed.m[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
    inversed.m[2][1] = m[0][1]*m[2][0] - m[0][0]*m[2][1];
    inversed.m[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];

    float fDet = m[0][0]*inversed[0][0] + m[0][1]*inversed[1][0]+
        m[0][2]*inversed[2][0];

    if (abs(fDet) <= ZERO_TOLERANCE)
    {
        return ZERO;
    }

    inversed /= fDet;
    return inversed;
}

//----------------------------------------------------------------------------
Mat33 Mat33::Transpose() const
{
	return Mat33(	m[0][0], m[1][0], m[2][0],
					m[0][1], m[1][1], m[2][1],
					m[0][2], m[1][2], m[2][2] );
}

//----------------------------------------------------------------------------
Mat33 Mat33::ScaleAxis(const Vec3& scale) const
{
	return Mat33(	m[0][0] * scale.x, m[0][1] * scale.y, m[0][2] * scale.z,
					m[1][0] * scale.x, m[1][1] * scale.y, m[1][2] * scale.z,
					m[2][0] * scale.x, m[2][1] * scale.y, m[2][2] * scale.z	);
}