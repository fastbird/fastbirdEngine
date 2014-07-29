#pragma once
#include <Engine/IInputLayout.h>
#include <Engine/Renderer/RendererEnums.h>
namespace fastbird
{
class InputLayout : public IInputLayout
{
public:
	InputLayout() : mVertexComponents(0) {}
	
	virtual ~InputLayout(){}
	
	virtual void SetDescs(const INPUT_ELEMENT_DESCS& descs);
	virtual const INPUT_ELEMENT_DESCS& GetDescs() const { return mDescs; }
	virtual bool HasVertexComponent(VERTEX_COMPONENT com) { return (mVertexComponents & com) != 0; }

private:
	INPUT_ELEMENT_DESCS mDescs;
	int mVertexComponents; // combination of VERTEX_COMPONENT
};

}