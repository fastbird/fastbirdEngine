#pragma once
#include <CommonLib/VectorMap.h>
namespace fastbird
{
	class Curve;
	class CurveManager
	{
	public:
		CurveManager();
		~CurveManager();

		// file is in .obj
		Curve* ImportCurve(size_t uniqueId, const char* filepath);
		Curve* GetCurve(size_t uniqueId);
		Curve* GetRandomCurve() const;

	private:
		fastbird::VectorMap<size_t, Curve*> mCurves;

	};
}