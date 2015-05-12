#pragma once
#include <Engine/RenderSteps.h>

namespace fastbird
{

class RenderPipeline : public ReferenceCounter
{
	bool mSteps[RenderSteps::Num];

public:
	RenderPipeline();
	RenderPipeline(const RenderPipeline& other);
	RenderPipeline(bool steps[]);
	~RenderPipeline();

	RenderPipeline* Clone() const;
	void SetStep(RenderSteps::Enum step, bool enable);
	bool GetStep(RenderSteps::Enum step) const;

	virtual void SetMaximum();
	virtual void SetMinimum();

};

}