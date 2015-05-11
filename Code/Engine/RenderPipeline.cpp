#include <Engine/StdAfx.h>
#include <Engine/RenderPipeline.h>

namespace fastbird
{

RenderPipeline::RenderPipeline()
{
	memset(mSteps, 0, sizeof(mSteps));
	mSteps[RenderSteps::ShadowMap] = true;
	mSteps[RenderSteps::Depth] = true;
	mSteps[RenderSteps::Silouette] = true;
}

RenderPipeline::RenderPipeline(bool steps[])
{
	memcpy(mSteps, steps, sizeof(mSteps));
}

RenderPipeline::RenderPipeline(const RenderPipeline& other)
{
	memcpy(mSteps, other.mSteps, sizeof(mSteps));
}

RenderPipeline::~RenderPipeline()
{

}

RenderPipeline* RenderPipeline::Clone() const
{
	auto newPipeline = FB_NEW(RenderPipeline)(*this);
	return newPipeline;
}

void RenderPipeline::SetStep(RenderSteps::Enum step, bool enable)
{
	mSteps[step] = enable;
}

bool RenderPipeline::GetStep(RenderSteps::Enum step) const
{
	return mSteps[step];
}

void RenderPipeline::EnableAll()
{
	for (unsigned i = 0; i < RenderSteps::Num; ++i)
	{
		mSteps[i] = true;
	}
}

}