#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{

	Vec3 CalculateTangentSpaceVector(
        const Vec3& position1, const Vec3& position2, const Vec3& position3,
        const Vec2& uv1, const Vec2& uv2, const Vec2& uv3)
    {
	    // Calc Tangent
	    Vec3 side0 = position1 - position2;
	    Vec3 side1 = position3 - position1;
		Vec3 normal = side1.Cross(side0);
		normal.Normalize();

		float deltaV0 = uv1.y - uv2.y;
		float deltaV1 = uv3.y - uv1.y;
	    Vec3 tangent = deltaV1 * side0 - deltaV0 * side1;
		tangent.Normalize();

	    //Calc binormal
	    float deltaU0 = uv1.x - uv2.x;
	    float deltaU1 = uv3.x - uv1.x;
	    Vec3 binormal = deltaU1 * side0 - deltaU0 * side1;
		binormal.Normalize();
	    //Now, we take the cross product of the tangents to get a vector which 
	    //should point in the same direction as our normal calculated above. 
	    //If it points in the opposite direction 
		// (the dot product between the normals is less than zero), 
	    //then we need to reverse the s and t tangents. 
	    //This is because the triangle has been mirrored when going 
		// from tangent space to object space.
	    //reverse tangents if necessary
		Vec3 tangentCross = tangent.Cross(binormal);
		if (tangentCross.Dot(normal) < 0.0f)
	    {
		    tangent = -tangent;
		    binormal = -binormal;
	    }

        return tangent;
    }

	Vec3 ProjectTo(const Plane3& plane, const Ray3& ray0, const Ray3& ray1)
	{
		Ray3::IResult ret0 = ray0.intersects(plane);
		if (ret0.second!=0.f)
		{
			Ray3::IResult ret1 = ray1.intersects(plane);
			if (ret1.second!=0.f)
			{
				Vec3 startOnPlane = ray0.GetPoint(ret0.second);
				Vec3 endOnPlane = ray1.GetPoint(ret1.second);
				float p = plane.mNormal.Dot(endOnPlane - startOnPlane);
				return plane.mNormal * p;
			}
		}	

		return Vec3(0, 0, 0);
	}
}