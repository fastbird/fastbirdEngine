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
#include "Math.h"

namespace fb
{
	bool IsEqual(Real a, Real b, Real epsilon)
	{
		return abs(a - b) < epsilon;
	}

	bool IsNaN(Real f)
	{
		return f != f;
	}	

	Real Degree(Real radian)
	{
		return radian / PI * 180.0f;
	}

	Real Radian(Real degree)
	{
		return degree / 180.0f * PI;
	}

	Vec2 Radian(const Vec2& degrees)
	{
		return Vec2(Radian(degrees.x), Radian(degrees.y));
	}

	bool IsPowerOfTwo(int a)
	{
		return !(a & (a - 1));
	}

	int Round(Real v)
	{
		return (int)(v + 0.5f);
	}

	Vec2I Round(const Vec2& v)
	{
		return Vec2I(Round(v.x), Round(v.y));
	}

	Vec3I Round(const Vec3& v)
	{
		return Vec3I(Round(v.x), Round(v.y), Round(v.z));
	}

	Vec3 FixPrecisionScaleVector(const Vec3& v)
	{
		Vec3 fixedScale = v;
		Vec3I rounded = Round(v);
		if (rounded.x == rounded.y && rounded.x == rounded.z)
		{
			// uniform
			if (abs((Real)rounded.x - v.x) < 0.00001f)
			{
				fixedScale.x = fixedScale.y = fixedScale.z = (Real)rounded.x;
			}
		}
		return fixedScale;
	}

	Real log2(Real v)
	{
		return (Real)(log(v) / log(2));
	}

	Real sinc(Real x) {               /* Supporting sinc function */
		if (fabs(x) < 1.0e-4) return 1.0;
		else return(sin(x) / x);
	}

	bool IsLittleEndian() // intel
	{
		static bool needToCheck = true;
		static bool littleEndian = true;
		if (needToCheck)
		{
			needToCheck = false;

			double one = 1.0;
			unsigned *ip;
			ip = (unsigned*)&one;
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
		unsigned short *hp = (unsigned short *)source; // Type pun input as an unsigned 16-bit int
		unsigned *xp = (unsigned *)target; // Type pun output as an unsigned 32-bit int
		unsigned short h, hs, he, hm;
		unsigned xs, xe, xm;
		int xes;
		int e;

		if (source == NULL || target == NULL) // Nothing to convert (e.g., imag part of pure real)
			return;
		while (num--) {
			h = *hp++;
			if ((h & 0x7FFFu) == 0) {  // Signed zero
				*xp++ = ((unsigned)h) << 16;  // Return the signed zero
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
					xs = ((unsigned)hs) << 16; // Sign bit
					xes = ((int)(he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
					xe = (unsigned)(xes << 23); // Exponent
					xm = ((unsigned)(hm & 0x03FFu)) << 13; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
				else if (he == 0x7C00u) {  // Inf or NaN (all the exponent bits are set)
					if (hm == 0) { // If mantissa is zero ...
						*xp++ = (((unsigned)hs) << 16) | ((unsigned)0x7F800000u); // Signed Inf
					}
					else {
						*xp++ = (unsigned)0xFFC00000u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					xs = ((unsigned)hs) << 16; // Sign bit
					xes = ((int)(he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
					xe = (unsigned)(xes << 23); // Exponent
					xm = ((unsigned)hm) << 13; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
			}
		}
	}
	void Halfp2Doubles(void *target, void *source, unsigned num)
	{
		unsigned short *hp = (unsigned short *)source; // Type pun input as an unsigned 16-bit int
		unsigned *xp = (unsigned *)target; // Type pun output as an unsigned 32-bit int
		unsigned short h, hs, he, hm;
		unsigned xs, xe, xm;
		int xes;
		int e;

		if (IsLittleEndian())
			xp += 1;  // Little Endian adjustment if necessary

		if (source == NULL || target == NULL) // Nothing to convert (e.g., imag part of pure real)
			return;
		while (num--) {
			h = *hp++;
			if ((h & 0x7FFFu) == 0) {  // Signed zero
				*xp++ = ((unsigned)h) << 16;  // Return the signed zero
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
					xs = ((unsigned)hs) << 16; // Sign bit
					xes = ((int)(he >> 10)) - 15 + 1023 - e; // Exponent unbias the halfp, then bias the double
					xe = (unsigned)(xes << 20); // Exponent
					xm = ((unsigned)(hm & 0x03FFu)) << 10; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
				else if (he == 0x7C00u) {  // Inf or NaN (all the exponent bits are set)
					if (hm == 0) { // If mantissa is zero ...
						*xp++ = (((unsigned)hs) << 16) | ((unsigned)0x7FF00000u); // Signed Inf
					}
					else {
						*xp++ = (unsigned)0xFFF80000u; // NaN, only the 1st mantissa bit set
					}
				}
				else {
					xs = ((unsigned)hs) << 16; // Sign bit
					xes = ((int)(he >> 10)) - 15 + 1023; // Exponent unbias the halfp, then bias the double
					xe = (unsigned)(xes << 20); // Exponent
					xm = ((unsigned)hm) << 10; // Mantissa
					*xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
				}
			}
			xp++; // Skip over the remaining 32 bits of the mantissa
		}
	}

	void Singles2Halfp(void *target, void *source, unsigned num)
	{
		unsigned short *hp = (unsigned short *)target; // Type pun output as an unsigned 16-bit int
		unsigned *xp = (unsigned *)source; // Type pun input as an unsigned 32-bit int
		unsigned short    hs, he, hm;
		unsigned x, xs, xe, xm;
		int hes;

		if (source == NULL || target == NULL) { // Nothing to convert (e.g., imag part of pure real)
			return;
		}
		while (num--) {
			x = *xp++;
			if ((x & 0x7FFFFFFFu) == 0) {  // Signed zero
				*hp++ = (unsigned short)(x >> 16);  // Return the signed zero
			}
			else { // Not zero
				xs = x & 0x80000000u;  // Pick off sign bit
				xe = x & 0x7F800000u;  // Pick off exponent bits
				xm = x & 0x007FFFFFu;  // Pick off mantissa bits
				if (xe == 0) {  // Denormal will underflow, return a signed zero
					*hp++ = (unsigned short)(xs >> 16);
				}
				else if (xe == 0x7F800000u) {  // Inf or NaN (all the exponent bits are set)
					if (xm == 0) { // If mantissa is zero ...
						*hp++ = (unsigned short)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else {
						*hp++ = (unsigned short)0xFE00u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					hs = (unsigned short)(xs >> 16); // Sign bit
					hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
					if (hes >= 0x1F) {  // Overflow
						*hp++ = (unsigned short)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else if (hes <= 0) {  // Underflow
						if ((14 - hes) > 24) {  // Mantissa shifted all the way off & no rounding possibility
							hm = (unsigned short)0u;  // Set mantissa to zero
						}
						else {
							xm |= 0x00800000u;  // Add the hidden leading bit
							hm = (unsigned short)(xm >> (14 - hes)); // Mantissa
							if ((xm >> (13 - hes)) & 0x00000001u) // Check for rounding
								hm += (unsigned short)1u; // Round, might overflow into exp bit, but this is OK
						}
						*hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
					}
					else {
						he = (unsigned short)(hes << 10); // Exponent
						hm = (unsigned short)(xm >> 13); // Mantissa
						if (xm & 0x00001000u) // Check for rounding
							*hp++ = (hs | he | hm) + (unsigned short)1u; // Round, might overflow to inf, this is OK
						else
							*hp++ = (hs | he | hm);  // No rounding
					}
				}
			}
		}
	}

	void Doubles2Halfp(void *target, void *source, unsigned num)
	{
		unsigned short *hp = (unsigned short *)target; // Type pun output as an unsigned 16-bit int
		unsigned *xp = (unsigned *)source; // Type pun input as an unsigned 32-bit int
		unsigned short    hs, he, hm;
		unsigned x, xs, xe, xm;
		int hes;

		if (IsLittleEndian())
			xp += 1;  // Little Endian adjustment if necessary

		if (source == NULL || target == NULL) { // Nothing to convert (e.g., imag part of pure real)
			return;
		}
		while (num--) {
			x = *xp++; xp++; // The extra xp++ is to skip over the remaining 32 bits of the mantissa
			if ((x & 0x7FFFFFFFu) == 0) {  // Signed zero
				*hp++ = (unsigned short)(x >> 16);  // Return the signed zero
			}
			else { // Not zero
				xs = x & 0x80000000u;  // Pick off sign bit
				xe = x & 0x7FF00000u;  // Pick off exponent bits
				xm = x & 0x000FFFFFu;  // Pick off mantissa bits
				if (xe == 0) {  // Denormal will underflow, return a signed zero
					*hp++ = (unsigned short)(xs >> 16);
				}
				else if (xe == 0x7FF00000u) {  // Inf or NaN (all the exponent bits are set)
					if (xm == 0) { // If mantissa is zero ...
						*hp++ = (unsigned short)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else {
						*hp++ = (unsigned short)0xFE00u; // NaN, only 1st mantissa bit set
					}
				}
				else { // Normalized number
					hs = (unsigned short)(xs >> 16); // Sign bit
					hes = ((int)(xe >> 20)) - 1023 + 15; // Exponent unbias the double, then bias the halfp
					if (hes >= 0x1F) {  // Overflow
						*hp++ = (unsigned short)((xs >> 16) | 0x7C00u); // Signed Inf
					}
					else if (hes <= 0) {  // Underflow
						if ((10 - hes) > 21) {  // Mantissa shifted all the way off & no rounding possibility
							hm = (unsigned short)0u;  // Set mantissa to zero
						}
						else {
							xm |= 0x00100000u;  // Add the hidden leading bit
							hm = (unsigned short)(xm >> (11 - hes)); // Mantissa
							if ((xm >> (10 - hes)) & 0x00000001u) // Check for rounding
								hm += (unsigned short)1u; // Round, might overflow into exp bit, but this is OK
						}
						*hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
					}
					else {
						he = (unsigned short)(hes << 10); // Exponent
						hm = (unsigned short)(xm >> 10); // Mantissa
						if (xm & 0x00000200u) // Check for rounding
							*hp++ = (hs | he | hm) + (unsigned short)1u; // Round, might overflow to inf, this is OK
						else
							*hp++ = (hs | he | hm);  // No rounding
					}
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------
	unsigned long GetNextPowerOfTwo(unsigned long Value)
	{
		if (Value)
		{
			Value--;
			Value = (Value >> 1) | Value;
			Value = (Value >> 2) | Value;
			Value = (Value >> 4) | Value;
			Value = (Value >> 8) | Value;
			Value = (Value >> 16) | Value;
		}
		return Value + 1;
	}

	Vec2I GetNextPowerOfTwo(const Vec2I& value)
	{
		return Vec2I(GetNextPowerOfTwo(value.x), GetNextPowerOfTwo(value.y));
	}

	unsigned GetNextMultipleOfFour(unsigned value)
	{
		return (value + 3)&~0x03;
	}

	Vec2I GetNextMultipleOfFour(const Vec2I value)
	{
		return Vec2I(GetNextMultipleOfFour(value.x), GetNextMultipleOfFour(value.y));
	}

	Color Lerp(const Color& a, const Color& b, Real lp) {
		lp = std::min(lp, (Real)1.0);
		lp = std::max((Real)0.0, lp);
		return a * (1.0f - lp) + b * lp;
	}

	Quat Slerp(Quat qa, Quat qb, Real t)
	{
		Real cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
		// qa and qb is same.
		if (abs(cosHalfTheta) >= 1.0)
		{
			return qa;
		}
		if (cosHalfTheta < 0){
			qb = -qb;
			return Slerp(qa, qb, t);
		}

		Quat ret;
		Real halfTheta = acos(cosHalfTheta);
		Real sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);
		// 180 degree case
		// not defined.
		if ((Real)fabs(sinHalfTheta) < 0.001f)
		{
			ret.w = (qa.w * 0.5f + qb.w * 0.5f);
			ret.x = (qa.x * 0.5f + qb.x * 0.5f);
			ret.y = (qa.y * 0.5f + qb.y * 0.5f);
			ret.z = (qa.z * 0.5f + qb.z * 0.5f);
			return ret;
		}

		Real ratioA = sin((1.0f - t) * halfTheta) / sinHalfTheta;
		Real ratioB = sin(t * halfTheta) / sinHalfTheta;
		ret.w = (qa.w * ratioA + qb.w * ratioB);
		ret.x = (qa.x * ratioA + qb.x * ratioB);
		ret.y = (qa.y * ratioA + qb.y * ratioB);
		ret.z = (qa.z * ratioA + qb.z * ratioB);
		return ret;
	}

	//-------------------------------------------------------------------------
	Real ACos(Real fValue)
	{
		if (-1.0 < fValue)
		{
			if (fValue < 1.0)
				return acos(fValue);
			else
				return 0.0f;
		}
		else
		{
			return PI;
		}
	}

	//-------------------------------------------------------------------------
	bool IsOverlapped(const Rect& a, const Rect& b)
	{
		if (a.right < b.left || a.left > b.right ||
			a.bottom < b.top || a.top > b.bottom)
			return false;

		return true;
	}

	Real Step(Real edge, Real s)
	{
		return edge > s ? 0.0f : 1.0f;
	}

	//------------------------------------------------------------------------
	Mat44 MakeViewMatrix(const Vec3& pos, const Vec3& x, const Vec3& y, const Vec3& z)
	{
		// transposed
		Mat33 transposedRot(
			x.x, x.y, x.z,
			y.x, y.y, y.z,
			z.x, z.y, z.z
			);
		Vec3 t = -(transposedRot * pos);
		Mat44 viewMat(transposedRot, t);
		return viewMat;
	}

	//------------------------------------------------------------------------
	// d3d left handed
	Mat44 MakeProjectionMatrix(Real fov, Real aspectRatio, Real n, Real f)
	{
		const Real cot = 1.0f / tan(fov * .5f);
		const Real DofA = cot / aspectRatio;
		Real A = f / (f - n);
		Real B = -n*f / (f - n);

		return Mat44(
			DofA, 0, 0, 0,
			0, cot, 0, 0,
			0, 0, A, B,
			0, 0, 1, 0
			);

		/*return Mat44(
			DofA, 0, 0, 0,
			0, 0, cot, 0,
			0, A, 0, B,
			0, 1, 0, 0);*/ // y, z swaped
	}

	//------------------------------------------------------------------------
	// d3d left handed
	Mat44 MakeOrthogonalMatrix(Real l, Real t, Real r, Real b, Real n, Real f)
	{
		return Mat44(
			2.f / (r - l), 0, 0, (l + r) / (l - r),
			0, 2.f / (t - b), 0, (t + b) / (b - t),
			0, 0, 1.f / (f - n), n / (n - f),
			0, 0, 0, 1.f);
	}

	Vec2 operator/(const Vec2& a, const Vec2I b)
	{
		return Vec2(a.x / (Real)b.x, a.y / (Real)b.y);
	}

	Vec3 CleanNegativeZero(const Vec3& a)
	{
		return Vec3(abs(a.x) < EPSILON ? 0.f : a.x,
			abs(a.y) < EPSILON ? 0.f : a.y,
			abs(a.z) < EPSILON ? 0.f : a.z);
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

		Real deltaV0 = uv1.y - uv2.y;
		Real deltaV1 = uv3.y - uv1.y;
	    Vec3 tangent = deltaV1 * side0 - deltaV0 * side1;
		tangent.Normalize();

	    //Calc binormal
	    Real deltaU0 = uv1.x - uv2.x;
	    Real deltaU1 = uv3.x - uv1.x;
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
		Ray3::IResult ret0 = ray0.Intersects(plane);
		if (ret0.second!=0.f)
		{
			Ray3::IResult ret1 = ray1.Intersects(plane);
			if (ret1.second!=0.f)
			{
				Vec3 startOnPlane = ray0.GetPoint(ret0.second);
				Vec3 endOnPlane = ray1.GetPoint(ret1.second);
				Real p = plane.mNormal.Dot(endOnPlane - startOnPlane);
				return plane.mNormal * p;
			}
		}	

		return Vec3(0, 0, 0);
	}

	Vec3 ProjectPointOnToLine(const Vec3& lineStartP, const Vec3& lineDir, const Vec3& point){
		float t = lineDir.Dot(point - lineStartP);
		return lineStartP + lineDir * t;
	}

	int GetMipLevels(Real v)
	{
		return (int)((log(v) / LOG2) + 0.5f) + 1;
	}

	Real Sign(Real s)
	{
		return s < 0.0f ? -1.0f : (s == 0.0f ? 0.0f : 1.0f);
	}

	Vec3 Max(const Vec3& a, const Vec3& b)
	{
		return Vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
	}

	Vec3 Min(const Vec3& a, const Vec3& b)
	{
		return Vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
	}

	Vec2 Abs(const Vec2& v)
	{
		return Vec2(abs(v.x), abs(v.y));
	}

	Vec3 Abs(const Vec3& v)
	{
		return Vec3(abs(v.x), abs(v.y), abs(v.z));
	}

	// 0 <= r < infinite
	// 0 <= theta <= PI
	// 0 <= phi < TWO_PI
	Vec3 SphericalToCartesian(Real r, Real theta, Real phi)
	{
		Real st = sin(theta);
		Real cp = cos(phi);
		Real sp = sin(phi);
		Real ct = cos(theta);

		return Vec3(r*st*cp, r*st*sp, r*ct);
	}

	// for r == 1.0
	Vec3 SphericalToCartesian(Real theta, Real phi)
	{
		Real st = sin(theta);
		Real cp = cos(phi);
		Real sp = sin(phi);
		Real ct = cos(theta);

		return Vec3(st*cp, st*sp, ct);
	}

	Vec3 CartesianToSpherical(const Vec3& c)
	{
		Real r = c.Length();
		Real theta = acos(c.z / r);
		Real phi = atan2(c.y, c.x);
		phi += phi < 0 ? TWO_PI : 0;
		return Vec3(r, theta, phi);
	}

	Real SmoothStep(Real min, Real max, Real v)
	{
		Real size = max - min;
		Real pos = v - min;
		return pos / size;
	}

	Real Squared(Real x)
	{
		return x*x;
	}

	// toTargetDir : normalized
	void CalcInterceptPosition(const Vec3& firePos, Real ammoSpeed, const Vec3& toTargetDir, Real distance, const Vec3& targetVel,
		Vec3& outVelocity, Vec3* outPos, Real* timeToTarget)
	{
		assert(ammoSpeed != 0);
		Real targetVelUMag = targetVel.Dot(toTargetDir);
		Vec3 targetVelU = toTargetDir * targetVelUMag;
		Vec3 targetVelV = targetVel - targetVelU;

		Vec3 shotVelV = targetVelV; // should be the same.
		Real shotVelMag = shotVelV.Length();
		if (shotVelMag < ammoSpeed)
		{
			// Pythagoras theorem
			Real shotSpeedMag = sqrt(ammoSpeed*ammoSpeed - shotVelMag*shotVelMag);
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
		Real numerator = qmp.Cross(s);
		Real denominator = r.Cross(s);
		Real t = numerator / denominator;

		Real u = qmp.Cross(r) / denominator;
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

	void ExpandRect(Rect& r, int size)
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

	bool RayAABB(const Vec3& ro, const Vec3& rdi,
		const Vec3I& raySign, // 1 if negative
		const AABB& aabb,
		Real& min, Vec3& normal,
		Real pseudo_min, Real pseudo_max)
	{
		// 0 : x, 1: y, 2: z
		int collisionFace = 0;
		// impact times.
		Real max, ymin, ymax, zmin, zmax;
		Vec3 bounds[] = { aabb.GetMin(), aabb.GetMax() };
		min = (bounds[raySign.x].x - ro.x) * rdi.x;
		max = (bounds[1 - raySign.x].x - ro.x) * rdi.x;
		ymin = (bounds[raySign.y].y - ro.y) * rdi.y;
		ymax = (bounds[1 - raySign.y].y - ro.y) * rdi.y;

		if ((min > ymax) || (ymin > max))
			return false;

		if (ymin > min)
		{
			min = ymin;
			collisionFace = 1;
		}

		if (ymax < max)
			max = ymax;

		zmin = (bounds[raySign.z].z - ro.z) * rdi.z;
		zmax = (bounds[1 - raySign.z].z - ro.z) * rdi.z;

		if ((min > zmax) || (zmin > max))
			return false;
		if (zmin > min)
		{
			min = zmin;
			collisionFace = 2;
		}

		if (zmax < max)
			max = zmax;

		static Real normalElem[] = { -1.0f, 1.0f }; // reversed order.
		normal = Vec3(collisionFace == 0 ? normalElem[raySign.x] : 0,
			collisionFace == 1 ? normalElem[raySign.y] : 0,
			collisionFace == 2 ? normalElem[raySign.z] : 0);


		return ((min < pseudo_max) && (max > pseudo_min));
	}

	Real GetTimeToExitNDCSquare(const Vec2& pos, const Vec2& dir)
	{
		Real xdist = dir.x >= 0 ? 1.0f - pos.x : -1.0f - pos.x;
		Real xtook = xdist / dir.x;

		Real ydist = dir.y >= 0 ? 1.0f - pos.y : -1.0f - pos.y;
		Real ytook = ydist / dir.y;
		return std::min(ytook, xtook);
	}

	void StepToDigit_(TCHAR** ppch)
	{
		if (*ppch == 0)
			return;

		int len = _tstrlen(*ppch);
		for (int i = 0; i<len; i++)
		{
			if (_tisdigit(**ppch))
				return;
			*ppch += 1;
		}

		*ppch = 0;
	}

	// reference http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
	void GenerateHammersley(int n, std::vector<Vec2>& outResult)
	{
		outResult.clear();
		outResult.reserve(n);
		float p, t, phi;
		int k, kk, pos;
		for (k = 0, pos = 0; k<n; k++)
		{
			t = 0;
			for (p = 0.5f, kk = k; kk; p *= 0.5f, kk >>= 1)
				if (kk & 1) //kkmod2==1
					t += p;
			phi = (k + 0.5f) / (float)n; // a slight shift
			outResult.push_back(Vec2(phi, t));
		}
	}

	int CropSize8(int size)
	{
		return size - size % 8;
	}

	Real ShortestYaw(Real from, Real to){
		while (to > TWO_PI)
		{
			to -= TWO_PI;
		}
		while (to < 0)
		{
			to += TWO_PI;
		}

		// forward check
		float forwardLength;
		float backwardLength;
		bool swaped = false;
		if (from > to){
			std::swap(from, to);
			swaped = true;
		}
		forwardLength = to - from;
		backwardLength = ((from + TWO_PI) - to);

		if (!swaped){
			if (forwardLength <= backwardLength)
				return from + forwardLength;
			else
				return from - backwardLength;
		}
		else{
			if (forwardLength <= backwardLength)
				return from - forwardLength;
			else
				return from + backwardLength;
		}
	}
}