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
#include "NoiseGen.h"
#include "Math.h"
using namespace fb;

NoiseGen::NoiseGen() {
	mP = {
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
		8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
		35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
		134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
		55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
		18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
		250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
		189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167,
		43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
		97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
		107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };
	
	mP.insert(mP.end(), mP.begin(), mP.end());
}

NoiseGen::NoiseGen(unsigned int seed) {
	Seed(seed);
}

static Real Fade(Real t) {
	return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}

static Real Grad(int hash, Real x, Real y, Real z) {
	int h = hash & 0x0F;
	// Convert lower 4 bits of hash inot 12 Gradient directions
	Real u = h < 8 ? x : y,
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

static Real Grad(int32_t hash, Real x) {
	int32_t h = hash & 0x0F;        
	float grad = 1.0f + (h & 7);    
	if ((h & 8) != 0) grad = -grad; 																	
	return (grad * x);              
}

void NoiseGen::Seed(unsigned int seed) {
	mP.resize(256);

	std::iota(mP.begin(), mP.end(), 0);
	std::default_random_engine engine(seed);
	std::shuffle(mP.begin(), mP.end(), engine);
	mP.insert(mP.end(), mP.begin(), mP.end());
}

void NoiseGen::GetPermutation(ByteArray& outData) {
	outData = mP;
}

Real NoiseGen::Get(Real x) {
	int X = (int)floor(x) & 255;
	int NX = X + 1;
	x -= floor(x);
	Real x1 = x - 1.f;

	Real t0 = 1.0f - x*x;
	t0 *= t0;
	Real n0 = t0 * t0 * Grad(mP[X], x);

	Real t1 = 1.0f - x1 * x1;
	t1 *= t1;
	Real n1 = t1 * t1 * Grad(mP[NX], x1);

	return 0.395f * (n0 + n1);
}

Real NoiseGen::Get(Real x, Real y, Real z) {
	// Find the unit cube that contains the point
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	// Find relative x, y,z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute Fade curves for each of x, y, z
	Real u = Fade(x);
	Real v = Fade(y);
	Real w = Fade(z);

	// Hash coordinates of the 8 cube corners
	int A = mP[X] + Y;
	int AA = mP[A] + Z;
	int AB = mP[A + 1] + Z;
	int B = mP[X + 1] + Y;
	int BA = mP[B] + Z;
	int BB = mP[B + 1] + Z;

	// Add blended results from 8 corners of cube
	Real res = Lerp( Lerp( Lerp(Grad(mP[AA], x, y, z), Grad(mP[BA], x - 1, y, z), u),
		Lerp(Grad(mP[AB], x, y - 1, z), Grad(mP[BB], x - 1, y - 1, z), u), v),
		Lerp(Lerp(Grad(mP[AA + 1], x, y, z - 1), Grad(mP[BA + 1], x - 1, y, z - 1), u), Lerp(Grad(mP[AB + 1], x, y - 1, z - 1), Grad(mP[BB + 1], x - 1, y - 1, z - 1), u), v), w);
	return (res + 1.0f) / 2.0f;
}