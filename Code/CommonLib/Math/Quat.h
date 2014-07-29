#pragma once

#include <assert.h>
#include <ostream>

namespace fastbird
{
	class Mat33;
	class Vec3;
	class Quat
	{
	public:
		Quat()
			: w(1), x(0), y(0), z(0)
		{
		}

		Quat(float _w, float _x, float _y, float _z)
			: w(_w), x(_x), y(_y), z(_z)
		{
		}

		Quat(const Mat33& rot)
		{
			FromRotationMatrix(rot);
		}

		Quat(float radian, const Vec3& axis)
		{
			FromAngleAxis(radian, axis);
		}

		Quat(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis)
		{
			FromAxes(xAxis, yAxis, zAxis);
		}

		void Swap(Quat& other)
		{
			std::swap(w, other.w);
			std::swap(x, other.x);
			std::swap(y, other.y);
			std::swap(z, other.z);
		}

		inline float operator [] ( const size_t i ) const
		{
			assert( i < 4 );
			return *(&w+i);
		}

		inline float& operator[] (const size_t i)
		{
			assert(i<4);
			return *(&w+i);
		}

		void FromRotationMatrix(const Mat33& rot);
		void ToRotationMatrix(Mat33& rot) const;
		void FromAngleAxis(const float radian, const Vec3& axis);
		void ToAngleAxis(float& radian, Vec3& axis);
		void FromAxes(const Vec3& xAxis, const Vec3& yAxis, const Vec3& zAxis);
		void ToAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis);

		Vec3 xAxis() const;
		Vec3 yAxis() const;
		Vec3 zAxis() const;

		inline Quat& operator= (const Quat& other)
		{
			w = other.w;
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		Quat operator+ (const Quat& qRot) const;
		Quat operator- (const Quat& qRot) const;
		Quat operator* (const Quat& qRot) const;
		Quat operator* (float fScalar) const;
		friend Quat operator* (float fScalar, const Quat& qRot);
		Quat operator- () const;
		inline bool operator== (const Quat& rhs) const
		{
			return w == rhs.w && x == rhs.x &&
				y == rhs.y && z == rhs.z;
		}

		inline bool operator!= (const Quat& rhs) const
		{
			return !operator==(rhs);
		}

		float Dot(const Quat& other) const;
		float Norm() const;
		float Normalise();
		Quat Inverse() const;
		Quat UnitInverse() const;
		Quat Exp() const;
		Quat Log() const;

		Vec3 operator* (const Vec3& vec) const;

		float GetRoll(bool reprojectAxis = true) const;
		float GetPitch(bool reprojectAxis = true) const;
		float GetYaw(bool reprojectAxis = true) const;
		bool Equals(const Quat& rhs, const float toleranceRadian) const;

		static Quat Slerp(float fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);
		static Quat SlerpExtraSpins(float fT, const Quat& qRotP, const Quat& qRotQ, int iExtraSpins);
		static void Intermediate(const Quat& qRot0, const Quat& qRot1, const Quat& qRot2, Quat& qRotA, Quat& qRotB);
		static Quat Squad(float fT, const Quat& qRotP, const Quat& qRotA, 
			const Quat& qRotB, const Quat& qRotQ, bool shortestPath = false);
		static Quat nlerp(float fT, const Quat& qRotP, const Quat& qRotQ, bool shortestPath = false);

		static const float ms_fEpsilon;
		static const Quat ZERO;
		static const Quat IDENTITY;

		inline bool IsNaN() const;
		
		inline friend std::ostream& operator << (std::ostream& o, const Quat& q)
		{
			o << "Quaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
			return o;
		}

	public:

		float w, x, y, z;
	};
}