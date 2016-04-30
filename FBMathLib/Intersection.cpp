#include "stdafx.h"
#include "Intersection.h"
using namespace fb;


const Intersection Intersection::DummyIntersection{Vec3::ZERO, false};

Intersection::Intersection() 	
{
}

Intersection::Intersection(const Vec3& intersectionPoint, bool isTangent)
	: mIntersectionPoint(intersectionPoint)
	, mIsTangent(isTangent)
{
}

Intersection::Intersection(const Vec3& intersectionPoint, const Vec3& normal)
	: mIntersectionPoint(intersectionPoint)
	, mNormal(normal)
{

}

Intersection::Intersection(const Vec3& intersectionPoint, Real intersectionLength, bool isTangent)
	: Intersection(intersectionPoint, isTangent)
{
	mIntersectionLength = intersectionLength;
}

void Intersection::SetUserData(void* data) {
	mUserData = data;
}

void* Intersection::GetUserData() const {
	return mUserData;
}

Vec3 Intersection::GetIntersectionPoint() const {
	return mIntersectionPoint;
}

void Intersection::SetIntersectionPoint(const Vec3& intersectionPoint) {
	mIntersectionPoint = intersectionPoint;
}

bool Intersection::IsTangent() const {
	return mIsTangent;
}

void Intersection::SetTangent(bool tangent) {
	mIsTangent = tangent;
}

Real Intersection::GetIntersectionLength() const {
	return mIntersectionLength;
}

Intersection::Array Intersection::sort(const Vec3& refPoint, const Array& listA, const Array& listB) {
	Array sorted;
	sorted.reserve(listA.size() + listB.size());
	sorted.insert(sorted.end(), listA.begin(), listA.end());
	sorted.insert(sorted.end(), listB.begin(), listB.end());
	std::sort(sorted.begin(), sorted.end(), [&refPoint](const Intersection& losiA, const Intersection& losiB) {
		auto dA = refPoint.DistanceToSQ(losiA.mIntersectionPoint);
		auto dB = refPoint.DistanceToSQ(losiB.mIntersectionPoint);
		return dA < dB;
	});	

	return sorted;
}

bool Intersection::operator == (const Intersection& other) const {
	return mIsTangent == other.mIsTangent && mIntersectionPoint == other.mIntersectionPoint;
}

std::string Intersection::ToString() const {
	return FormatString("Intersection Point: %s %s", mIntersectionPoint.ToString().c_str(),
		mIsTangent ? "is a tangent." : "is not a tangent");
}