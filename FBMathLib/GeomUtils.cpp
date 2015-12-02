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
#include "GeomUtils.h"

#include "MathDefines.h"
namespace fb
{
void CreateSphereMesh(Real radius, int nRings, int nSegments,
	std::vector<Vec3>& pos, std::vector<unsigned short>& index, std::vector<Vec3>* normal, std::vector<Vec2>* uv)
{
	size_t vertexCount = (nRings +1) * (nSegments+1);
	size_t indexCount = 6 * nRings * (nSegments +1);
	assert(indexCount <= std::numeric_limits<unsigned short>::max());
	pos.reserve(vertexCount);
	index.reserve(indexCount);
	if (normal)
		normal->reserve(vertexCount);
	if (uv)
		uv->reserve(vertexCount);

	Real fDeltaRingAngle = (PI / nRings);
	Real fDeltaSegAngle = (2 * PI / nSegments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= nRings; ring++ ) {
		Real r0 = radius * std::sin(ring * fDeltaRingAngle);
		Real y0 = radius * std::sin(ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= nSegments; seg++) {
			Real x0 = r0 * std::sin(seg * fDeltaSegAngle);
			Real z0 = r0 * std::cos(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			pos.push_back(Vec3(x0, y0, z0));

			if (normal)
			{
				normal->push_back(Vec3(x0, y0, z0).NormalizeCopy());
			}
			if (uv)
			{
				uv->push_back(Vec2((Real) seg / (Real) nSegments, (Real) ring / (Real) nRings));
			}

			if (ring != nRings) 
			{
				// each vertex (except the last) has six indices pointing to it
				
				index.push_back(wVerticeIndex + nSegments + 1);
				index.push_back(wVerticeIndex);
				index.push_back(wVerticeIndex + nSegments);
				index.push_back(wVerticeIndex + nSegments + 1);
				index.push_back(wVerticeIndex + 1);
				index.push_back(wVerticeIndex);
				wVerticeIndex ++;
			}
		}; // end for seg
	} // end for ring

}
}