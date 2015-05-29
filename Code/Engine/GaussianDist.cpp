#include <Engine/StdAfx.h>
#include <Engine/GaussianDist.h>
#include <Engine/Renderer.h>
namespace fastbird
{
	void GaussianDist::Calc(unsigned width, unsigned height, float devi, float multiplier)
	{
		auto const renderer = gFBEnv->_pInternalRenderer;
		float afSampleOffsets[15];
		renderer->GetSampleOffsets_Bloom(width, afSampleOffsets, mGaussianDistWeightX, devi, multiplier);
		for (int i = 0; i < 15; i++)
		{
			mGaussianDistOffsetX[i] = Vec4(afSampleOffsets[i], 0.0f, 0.0f, 0.0f);
		}
		renderer->GetSampleOffsets_Bloom(height, afSampleOffsets, mGaussianDistWeightY, devi, multiplier);
		for (int i = 0; i < 15; i++)
		{
			mGaussianDistOffsetY[i] = Vec4(afSampleOffsets[i], 0.0f, 0.0f, 0.0f);
		}
	}
}