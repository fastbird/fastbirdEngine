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
#include "Math.h"
namespace fb
{
	void GeomUtils::CreateSphere(Real radius, int nRings, int nSegments,
		Vec3::Array& pos, std::vector<unsigned short>& index) 
	{
		int numVertices = nSegments * (nRings - 1) + 2;
		pos.resize(numVertices);

		float fSliceArc = TWO_PI / nSegments;
		float fSectionArc = PI / nRings;
		std::vector<float> fRingz(nRings + 1);
		std::vector<float> fRingSize(nRings + 1);
		std::vector<float> fRingx(nSegments + 1);
		std::vector<float> fRingy(nSegments + 1);
		for (int i = nRings; i >= 0; --i)
		{
			fRingz[i] = cosf(fSectionArc * i);
			fRingSize[i] = sinf(fSectionArc * i);
		}
		for (int i = 0; i <= nSegments; i++)
		{
			fRingx[i] = cosf(fSliceArc * i);
			fRingy[i] = sinf(fSliceArc * i);
		}

		int nIndex = 0;
		pos[nIndex++] = Vec3(0, 0, -radius);
		for (int j = nRings-1; j>=1; --j)
		{
			for (int i = 0; i<nSegments; i++)
			{
				Vec3 v(fRingx[i] * fRingSize[j], fRingy[i] * fRingSize[j], fRingz[j]);
				v *= radius / v.Length();
				pos[nIndex++] = v;
			}
		}
		index.clear();
		pos[nIndex++] = Vec3(0, 0, radius);

		unsigned short startIndex1 = 0;
		unsigned short startIndex2 = 1;
		unsigned short index1 = startIndex1;
		unsigned short index2 = startIndex2;
		for (int s = 0; s < nSegments; ++s) {
			index.push_back(index1);			
			index.push_back(index2++);
		}
		index.push_back(startIndex1);
		index.push_back(startIndex2);

		index1 = 1;
		index2 = index1 + nSegments;
		for (int r = 1; r < nRings - 1; ++r) {
			startIndex1 = index1;
			startIndex2 = index2;
			for (int s = 0; s < nSegments; ++s) {
				index.push_back(index1++);
				index.push_back(index2++);
			}
			index.push_back(startIndex1);
			index.push_back(startIndex2);			
		}

		// last line
		startIndex1 = index1;
		startIndex2 = index2;
		for (int s = 0; s < nSegments; ++s) {
			index.push_back(index1++);
			index.push_back(index2);
		}
		index.push_back(startIndex1);
		index.push_back(startIndex2);
		assert(index.back() == pos.size()-1);
	}
}