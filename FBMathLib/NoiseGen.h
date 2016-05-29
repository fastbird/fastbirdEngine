#pragma once
#include "FBCommonHeaders/Types.h"
namespace fb {
	class NoiseGen {
		ByteArray mP;
	public:
		NoiseGen();
		NoiseGen(unsigned int seed);
		void Seed(unsigned int seed);
		void GetPermutation(ByteArray& outData);
		Real Get(Real x);
		Real Get(Real x, Real y, Real z);
	};
}
