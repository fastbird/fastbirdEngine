#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/fbMath.h>

namespace fastbird
{
	bool IsLittleEndian() // intel
	{
		static bool needToCheck = true;
		static bool littleEndian = true;
		if (needToCheck)
		{
			needToCheck = false;

			double one = 1.0;
			UINT32 *ip;
			ip = (UINT32*)&one;
			if (*ip)
			{
				littleEndian = false;
			}
			else
			{
				littleEndian = true;
			}
		}
		return littleEndian;


	}

	void Halfp2Singles(void *target, void *source, unsigned num)
	{
		UINT16 *hp = (UINT16 *)source; // Type pun input as an unsigned 16-bit int
		UINT32 *xp = (UINT32 *)target; // Type pun output as an unsigned 32-bit int
		UINT16 h, hs, he, hm;
		UINT32 xs, xe, xm;
		INT32 xes;
		int e;

		if (source == NULL || target == NULL) // Nothing to convert (e.g., imag part of pure real)
			return;
		while (num--) {
			h = *hp++;
			if ((h & 0x7FFFu) == 0) {  // Signed zero
				*xp++ = ((UINT32)h) << 16;  // Return the signed zero
			}
			else { // Not zero
				hs = h & 0x8000u;  // Pick off sign bit
				he = h & 0x7C00u;  // Pick off exponent bits
				hm = h & 0x03FFu;  // Pick off mantissa bits
				if (he == 0) {  // Denormal will convert to normalized
					e = -1; // The following loop figures out how much extra to adjust the exponent
					do {
						e++;
						hm <<= 1;
					} while ((hm & 0x0400u) == 0); // Shift until leading bit overflows into exponent bit
					xs = ((UINT32)hs) << 16; // Sign bit
					xes = ((INT32)(he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
					xe = (UINT32)(xes << 23); // Exponent
					xm = ((UINT32)(hm & 0x03FFu)) << 13; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
				else if (he == 0x7C00u) {  // Inf or NaN (all the exponent bits are set)
					if (hm == 0) { // If mantissa is zero ...
						*xp++ = (((UINT32)hs) << 16) | ((UINT32)0x7F800000u); // Signed Inf
					}
					else {
						*xp++ = (UINT32)0xFFC00000u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					xs = ((UINT32)hs) << 16; // Sign bit
					xes = ((INT32)(he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
					xe = (UINT32)(xes << 23); // Exponent
					xm = ((UINT32)hm) << 13; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
			}
		}
	}
	void Halfp2Doubles(void *target, void *source, unsigned num)
	{
		UINT16 *hp = (UINT16 *)source; // Type pun input as an unsigned 16-bit int
		UINT32 *xp = (UINT32 *)target; // Type pun output as an unsigned 32-bit int
		UINT16 h, hs, he, hm;
		UINT32 xs, xe, xm;
		INT32 xes;
		int e;

		if (IsLittleEndian())
			xp += 1;  // Little Endian adjustment if necessary

		if (source == NULL || target == NULL) // Nothing to convert (e.g., imag part of pure real)
			return;
		while (num--) {
			h = *hp++;
			if ((h & 0x7FFFu) == 0) {  // Signed zero
				*xp++ = ((UINT32)h) << 16;  // Return the signed zero
			}
			else { // Not zero
				hs = h & 0x8000u;  // Pick off sign bit
				he = h & 0x7C00u;  // Pick off exponent bits
				hm = h & 0x03FFu;  // Pick off mantissa bits
				if (he == 0) {  // Denormal will convert to normalized
					e = -1; // The following loop figures out how much extra to adjust the exponent
					do {
						e++;
						hm <<= 1;
					} while ((hm & 0x0400u) == 0); // Shift until leading bit overflows into exponent bit
					xs = ((UINT32)hs) << 16; // Sign bit
					xes = ((INT32)(he >> 10)) - 15 + 1023 - e; // Exponent unbias the halfp, then bias the double
					xe = (UINT32)(xes << 20); // Exponent
					xm = ((UINT32)(hm & 0x03FFu)) << 10; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
				else if (he == 0x7C00u) {  // Inf or NaN (all the exponent bits are set)
					if (hm == 0) { // If mantissa is zero ...
						*xp++ = (((UINT32)hs) << 16) | ((UINT32)0x7FF00000u); // Signed Inf
					}
					else {
						*xp++ = (UINT32)0xFFF80000u; // NaN, only the 1st mantissa bit set
					}
				}
				else {
					xs = ((UINT32)hs) << 16; // Sign bit
					xes = ((INT32)(he >> 10)) - 15 + 1023; // Exponent unbias the halfp, then bias the double
					xe = (UINT32)(xes << 20); // Exponent
					xm = ((UINT32)hm) << 10; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
			}
			xp++; // Skip over the remaining 32 bits of the mantissa
		}
	}

	void Singles2Halfp(void *target, void *source, unsigned num)
	{
		UINT16 *hp = (UINT16 *)target; // Type pun output as an unsigned 16-bit int
		UINT32 *xp = (UINT32 *)source; // Type pun input as an unsigned 32-bit int
		UINT16    hs, he, hm;
		UINT32 x, xs, xe, xm;
		int hes;

		if (source == NULL || target == NULL) { // Nothing to convert (e.g., imag part of pure real)
			return;
		}
		while (num--) {
			x = *xp++;
			if ((x & 0x7FFFFFFFu) == 0) {  // Signed zero
				*hp++ = (UINT16)(x >> 16);  // Return the signed zero
			}
			else { // Not zero
				xs = x & 0x80000000u;  // Pick off sign bit
				xe = x & 0x7F800000u;  // Pick off exponent bits
				xm = x & 0x007FFFFFu;  // Pick off mantissa bits
				if (xe == 0) {  // Denormal will underflow, return a signed zero
					*hp++ = (UINT16)(xs >> 16);
				}
				else if (xe == 0x7F800000u) {  // Inf or NaN (all the exponent bits are set)
					if (xm == 0) { // If mantissa is zero ...
						*hp++ = (UINT16)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else {
						*hp++ = (UINT16)0xFE00u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					hs = (UINT16)(xs >> 16); // Sign bit
					hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
					if (hes >= 0x1F) {  // Overflow
						*hp++ = (UINT16)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else if (hes <= 0) {  // Underflow
						if ((14 - hes) > 24) {  // Mantissa shifted all the way off & no rounding possibility
							hm = (UINT16)0u;  // Set mantissa to zero
						}
						else {
							xm |= 0x00800000u;  // Add the hidden leading bit
							hm = (UINT16)(xm >> (14 - hes)); // Mantissa
							if ((xm >> (13 - hes)) & 0x00000001u) // Check for rounding
								hm += (UINT16)1u; // Round, might overflow into exp bit, but this is OK
						}
						*hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
					}
					else {
						he = (UINT16)(hes << 10); // Exponent
						hm = (UINT16)(xm >> 13); // Mantissa
						if (xm & 0x00001000u) // Check for rounding
							*hp++ = (hs | he | hm) + (UINT16)1u; // Round, might overflow to inf, this is OK
						else
							*hp++ = (hs | he | hm);  // No rounding
					}
				}
			}
		}
	}

	void Doubles2Halfp(void *target, void *source, unsigned num)
	{
		UINT16 *hp = (UINT16 *)target; // Type pun output as an unsigned 16-bit int
		UINT32 *xp = (UINT32 *)source; // Type pun input as an unsigned 32-bit int
		UINT16    hs, he, hm;
		UINT32 x, xs, xe, xm;
		int hes;

		if (IsLittleEndian())
			xp += 1;  // Little Endian adjustment if necessary

		if (source == NULL || target == NULL) { // Nothing to convert (e.g., imag part of pure real)
			return;
		}
		while (num--) {
			x = *xp++; xp++; // The extra xp++ is to skip over the remaining 32 bits of the mantissa
			if ((x & 0x7FFFFFFFu) == 0) {  // Signed zero
				*hp++ = (UINT16)(x >> 16);  // Return the signed zero
			}
			else { // Not zero
				xs = x & 0x80000000u;  // Pick off sign bit
				xe = x & 0x7FF00000u;  // Pick off exponent bits
				xm = x & 0x000FFFFFu;  // Pick off mantissa bits
				if (xe == 0) {  // Denormal will underflow, return a signed zero
					*hp++ = (UINT16)(xs >> 16);
				}
				else if (xe == 0x7FF00000u) {  // Inf or NaN (all the exponent bits are set)
					if (xm == 0) { // If mantissa is zero ...
						*hp++ = (UINT16)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else {
						*hp++ = (UINT16)0xFE00u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					hs = (UINT16)(xs >> 16); // Sign bit
					hes = ((int)(xe >> 20)) - 1023 + 15; // Exponent unbias the double, then bias the halfp
					if (hes >= 0x1F) {  // Overflow
						*hp++ = (UINT16)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else if (hes <= 0) {  // Underflow
						if ((10 - hes) > 21) {  // Mantissa shifted all the way off & no rounding possibility
							hm = (UINT16)0u;  // Set mantissa to zero
						}
						else {
							xm |= 0x00100000u;  // Add the hidden leading bit
							hm = (UINT16)(xm >> (11 - hes)); // Mantissa
							if ((xm >> (10 - hes)) & 0x00000001u) // Check for rounding
								hm += (UINT16)1u; // Round, might overflow into exp bit, but this is OK
						}
						*hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
					}
					else {
						he = (UINT16)(hes << 10); // Exponent
						hm = (UINT16)(xm >> 10); // Mantissa
						if (xm & 0x00000200u) // Check for rounding
							*hp++ = (hs | he | hm) + (UINT16)1u; // Round, might overflow to inf, this is OK
						else
							*hp++ = (hs | he | hm);  // No rounding
					}
				}
			}
		}
	}
	


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

	bool IsEqual(float a, float b, float epsilon)
	{
		return abs(a - b) < epsilon;
	}

	// toTargetDir : normalized
	void CalcInterceptPosition(const Vec3& firePos, float ammoSpeed, const Vec3& toTargetDir, float distance, const Vec3& targetVel,
		Vec3& outVelocity, Vec3* outPos, float* timeToTarget)
	{
		assert(ammoSpeed != 0);
		float targetVelUMag = targetVel.Dot(toTargetDir);
		Vec3 targetVelU = toTargetDir * targetVelUMag;
		Vec3 targetVelV = targetVel - targetVelU;

		Vec3 shotVelV = targetVelV; // should be the same.
		float shotVelMag = shotVelV.Length();
		if (shotVelMag < ammoSpeed)
		{
			// Pythagoras theorem
			float shotSpeedMag = sqrt(ammoSpeed*ammoSpeed - shotVelMag*shotVelMag);
			Vec3 shotVelU = toTargetDir * shotSpeedMag;
			outVelocity = shotVelU + shotVelV;
			if (outPos && timeToTarget)
			{
				*timeToTarget = distance / (shotSpeedMag - targetVelUMag);
				*outPos = firePos + outVelocity * (*timeToTarget);
			}
			
		}
		else
		{
			outVelocity = toTargetDir * ammoSpeed;
			if (outPos && timeToTarget)
			{
				*outPos = firePos + outVelocity;
				*timeToTarget = 1000.0f;
			}			
		}
	}

	SegmentIntersectResult SegmentIntersect(const Vec2& p, const Vec2& pend,
		const Vec2& q, const Vec2& qend,
		Vec2& outIntersect){
		Vec2 r = pend - p;
		Vec2 s = qend - q;
		Vec2 qmp = q - p;
		float numerator = qmp.Cross(s);
		float denominator = r.Cross(s);
		float t = numerator / denominator;

		float u = qmp.Cross(r) / denominator;
		if (numerator == 0 && denominator == 0){
			return SIR_COLLINEAR;
		}
		if (denominator == 0 && numerator != 0){
			return SIR_NONINTERSECTING;
		}
		if (denominator != 0 && t >= 0 && t <= 1 && u >= 0 && u <= 1){
			outIntersect = p+ r * t;
			return SIR_INTERSECT;
		}
		else{
			return SIR_NONINTERSECTING;
		}
	}

	void ExpandRect(RECT& r, int size)
	{
		r.left -= size;
		r.top -= size;
		r.right += size;
		r.bottom += size;
	}

	Vec2I GetMinComp(const Vec2I& a, const Vec2I& b)
	{
		int x = std::min(a.x, b.x);
		int y = std::min(a.y, b.y);
		return Vec2I(x, y);
	}

	Vec2I GetMaxComp(const Vec2I& a, const Vec2I& b)
	{
		int x = std::max(a.x, b.x);
		int y = std::max(a.y, b.y);
		return Vec2I(x, y);
	}


}