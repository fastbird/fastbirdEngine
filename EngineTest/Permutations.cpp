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
#include <FBCommonHeaders/Types.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "EssentialEngineData/shaders/CommonDefines.h"

namespace fb {
	void GeneratePermutation(const char* filepath) {
		// Generate Permutation
		ByteArray p(NUM_PERM + NUM_PERM + 2);
		std::iota(p.begin(), p.begin() + NUM_PERM, 0);
		for (int i = 0; i < NUM_PERM; i += 2) {
			auto j = Random(0, NUM_PERM - 1);
			std::swap(p[i], p[j]);
		}

		for (int i = 0; i < NUM_PERM + 2; ++i) {
			p[NUM_PERM + i] = p[i];
		}
		std::ofstream file(filepath, std::ios_base::binary);
		boost::archive::binary_oarchive ar(file);
		ar << p;
	}

	void GenerateGradients(const char* filepath) {
		// Generate Gradients
		std::vector<Vec4> gradiants(NUM_PERM + NUM_PERM + 2);
		std::srand(1);
		float s;
		Vec3 v;
		for (int i = 0; i < NUM_PERM; ++i) {
			do {
				for (int j = 0; j < 3; ++j) {
					v[j] = Random(-1.f, 1.f);
				}
				s = v.Dot(v);
			} while (s > 1.0f);
			s = sqrt(s);
			gradiants[i] = Vec4(v / s);
		}

		for (int i = 0; i < NUM_PERM + 2; ++i) {
			gradiants[NUM_PERM + i] = gradiants[i];
		}
		std::ofstream file(filepath, std::ios_base::binary);
		boost::archive::binary_oarchive ar(file);
		ar << gradiants;
	}
}
