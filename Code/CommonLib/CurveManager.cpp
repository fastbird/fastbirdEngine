#include <CommonLib/StdAfx.h>
#include <CommonLib/CurveManager.h>
#include <CommonLib/Curve.h>
#include <CommonLib/CurveImporter.h>

namespace fastbird
{
CurveManager::CurveManager()
{

}
CurveManager::~CurveManager()
{
	// delete all curves;
	for each (auto it in mCurves)
	{
		FB_DELETE(it.second);
	}
}

// file is in .obj
Curve* CurveManager::ImportCurve(size_t uniqueId, const char* filepath)
{
	if (mCurves.Find(uniqueId) != mCurves.end())
	{
		assert(0);
		return 0;
	}
	Curve* curve = FB_NEW(Curve);
	CurveImporter impoter;
	bool success = impoter.Import(curve, filepath);
	assert(success);
	mCurves[uniqueId] = curve;
	return curve;	
}
Curve* CurveManager::GetCurve(size_t uniqueId)
{
	auto it = mCurves.Find(uniqueId);
	if (it== mCurves.end())
		return 0;
	return it->second;
}

}