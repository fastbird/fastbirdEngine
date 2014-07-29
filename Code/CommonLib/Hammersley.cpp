#include <CommonLib/StdAfx.h>
#include <CommonLib/Hammersley.h>

namespace fastbird
{
	// reference http://www.cse.cuhk.edu.hk/~ttwong/papers/udpoint/udpoint.pdf
	void GenerateHammersley(int n, std::vector<Vec2>& outResult)
	{
		outResult.clear();
		outResult.reserve(n);
		float p, t, phi;
		int k, kk, pos;
		for (k=0, pos=0; k<n; k++)
		{
			t=0;
			for (p=0.5f, kk=k ; kk ; p*=0.5f, kk>>=1)
				if(kk&1) //kkmod2==1
					t+=p;			
			phi = (k + 0.5f) / (float)n; // a slight shift
			outResult.push_back(Vec2(phi, t));			
		}
	}
}
