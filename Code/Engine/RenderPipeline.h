#pragma once
#include <Engine/RenderSteps.h>

namespace fastbird
{

class RenderPipeline : public ReferenceCounter
{
	bool mSteps[RenderSteps::Num];

public:
	RenderPipeline();
	RenderPipeline(bool steps[]);
	~RenderPipeline();

	void SetStep(RenderSteps::Enum step, bool enable);
	bool GetStep(RenderSteps::Enum step) const;

	void EnableAll();

};

}