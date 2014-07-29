#include <Engine/StdAfx.h>
#include <Engine/Renderer/InputLayout.h>

namespace fastbird
{

void InputLayout::SetDescs(const INPUT_ELEMENT_DESCS& descs)
{
	mDescs = descs;
	mVertexComponents = 0;
	FB_FOREACH(it, descs)
	{
		if (it->mSemanticName == "POSITION")
		{
			mVertexComponents |= VC_POSITION;
		}
		else if (it->mSemanticName == "NORMAL")
		{
			mVertexComponents |= VC_NORMAL;
		}
		else if (it->mSemanticName == "COLOR")
		{
			mVertexComponents |= VC_COLOR;
		}
		else if (it->mSemanticName == "TEXCOORD")
		{
			mVertexComponents |= VC_TEXCOORD;
		}
		else if (it->mSemanticName == "TANGENT")
		{
			mVertexComponents |= VC_TANGENT;
		}
	}
}

}