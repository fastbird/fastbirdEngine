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

#pragma once
#include "Vec3.h"

namespace fb
{
	// model triangle
	struct ModelTriangle {
		unsigned        v[3];
		// cached data for optimized ray-triangle intersections
		Vec2   v0Proj;           // 2D projection of vertices along the dominant axis
		Vec2   v1Proj;
		Vec2   v2Proj;
		Vec3   faceNormal;
		Real         d;                // distance from triangle plane to origin
		int           dominantAxis;     // dominant axis of the triangle plane
	};

	struct ModelIntersection {
		const ModelTriangle        *pTri;      // Pointer to mesh triangle that was intersected by ray
		Vec3     position;   // Intersection point on the triangle
		Real           alpha;      // Alpha and beta are two of the barycentric coordinates of the intersection 
		Real           beta;       // ... the third barycentric coordinate can be calculated by: 1- (alpha + beta).
		bool            valid;      // "valid" is set to true if an intersection was found
	};

	void CreateSphereMesh(Real radius, int nRings, int nSegments,
		std::vector<Vec3>& pos, std::vector<unsigned short>& index, std::vector<Vec3>* normal, std::vector<Vec2>* uv);
}