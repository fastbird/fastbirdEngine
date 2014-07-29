#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/GeomUtils.h>

namespace fastbird
{
void CreateSphereMesh(float radius, int nRings, int nSegments,
	std::vector<Vec3>& pos, std::vector<UINT16>& index, std::vector<Vec3>* normal, std::vector<Vec2>* uv)
{
	size_t vertexCount = (nRings +1) * (nSegments+1);
	size_t indexCount = 6 * nRings * (nSegments +1);
	assert(indexCount <= std::numeric_limits<UINT16>::max());
	pos.reserve(vertexCount);
	index.reserve(indexCount);
	if (normal)
		normal->reserve(vertexCount);
	if (uv)
		uv->reserve(vertexCount);

	float fDeltaRingAngle = (PI / nRings);
	float fDeltaSegAngle = (2 * PI / nSegments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= nRings; ring++ ) {
		float r0 = radius * sinf (ring * fDeltaRingAngle);
		float y0 = radius * cosf (ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			pos.push_back(Vec3(x0, y0, z0));

			if (normal)
			{
				normal->push_back(Vec3(x0, y0, z0).NormalizeCopy());
			}
			if (uv)
			{
				uv->push_back(Vec2((float) seg / (float) nSegments, (float) ring / (float) nRings));
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