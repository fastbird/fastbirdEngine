#pragma once

#include <CommonLib/Math/Mat33.h>
#include <CommonLib/Math/Mat44.h>
#include <CommonLib/Math/Vec3.h>
#include <CommonLib/Math/Quat.h>
#include <CommonLib/Math/Plane3.h>
#include <CommonLib/Math/Ray3.h>

namespace fastbird
{
	class Plane;
	class Transformation
	{
	public:
		Transformation();
		~Transformation();

		void MakeIdentity ();
		void MakeUnitScale ();
		inline bool IsIdentity () const
		{
			return mIdentity;
		}
		inline bool IsRSSeperated() const
		{
			return mRSSeperated;
		}
		inline bool IsUniformScale () const
		{
			return mRSSeperated && mUniformScale;
		}

		void SetRotation (const Mat33& r);
		void SetRotation (const Quat& r);
		void AddRotation (const Quat& addR);
		inline const Quat& GetRotation() const
		{
			assert(mRSSeperated);
			return mR;
		}
		void SetMatrix (const Mat33& mat);
		inline const Mat33& GetMatrix () const
		{
			assert(mRSSeperated);
			return mMat;
		}
		void SetTranslation(const Vec3& t);
		void AddTranslation(const Vec3& addT);
		inline const Vec3& GetTranslation() const
		{
			return mT;
		}
		void SetScale (const Vec3& s);
		inline const Vec3& GetScale () const
		{
			assert(mRSSeperated);
			return mS;
		}
		void SetUniformScale (float fScale);
		inline float GetUniformScale () const
		{
			assert(mRSSeperated && mUniformScale);
			return mS.x;
		}

		float GetNorm () const;
		Vec3 ApplyForward (const Vec3& p) const;
		void ApplyForward (int iQuantity, const Vec3* points,
        Vec3* output) const;

		// X = M^{-1}*(Y-T) where Y is the input and X is the output.
		Vec3 ApplyInverse (const Vec3& p) const;
		void ApplyInverse (int iQuantity, const Vec3* points,
	        Vec3* output) const;
		Ray3 ApplyInverse(const Ray3& r) const;

		Vec3 InvertVector (const Vec3& v) const;

		Plane3 ApplyForward (const Plane3& p) const;

		void Product (const Transformation& a, const Transformation& b);

		Transformation operator* (const Transformation& t) const;
		
		void Inverse (Transformation& t) const;
		void GetHomogeneous (Mat44& hm) const;

		Vec3 GetRight() const;
		Vec3 GetForward() const;
		Vec3 GetUp() const;	

		static const Transformation IDENTITY;

	private:
		Mat33 mMat;
		Quat mR;
		Vec3 mT;
		Vec3 mS;
		bool mIdentity, mRSSeperated, mUniformScale;;
	};
}