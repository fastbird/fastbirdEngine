#pragma once
#include <CommonLib/Math/Vec3.h>

namespace fastbird
{
	// model triangle
	typedef struct {
		unsigned        v[3];
		// cached data for optimized ray-triangle intersections
		Vec2   v0Proj;           // 2D projection of vertices along the dominant axis
		Vec2   v1Proj;
		Vec2   v2Proj;
		Vec3   faceNormal;
		float         d;                // distance from triangle plane to origin
		int           dominantAxis;     // dominant axis of the triangle plane
	} ModelTriangle;

	struct ModelIntersection {
		const ModelTriangle        *pTri;      // Pointer to mesh triangle that was intersected by ray
		Vec3     position;   // Intersection point on the triangle
		float           alpha;      // Alpha and beta are two of the barycentric coordinates of the intersection 
		float           beta;       // ... the third barycentric coordinate can be calculated by: 1- (alpha + beta).
		bool            valid;      // "valid" is set to true if an intersection was found
	};

	void CreateSphereMesh(float radius, int nRings, int nSegments,
		std::vector<Vec3>& pos, std::vector<UINT16>& index, std::vector<Vec3>* normal, std::vector<Vec2>* uv);
}