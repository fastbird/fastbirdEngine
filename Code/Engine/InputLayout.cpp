#include <Engine/StdAfx.h>
#include <Engine/InputLayout.h>

namespace fastbird
{
	void InputLayout::FinishSmartPtr(){
		FB_DELETE(this);
	}

void InputLayout::SetDescs(const INPUT_ELEMENT_DESCS& descs)
{
	mDescs = descs;
	mVertexComponents = 0;
	FB_FOREACH(it, descs)
	{
		if (strcmp(it->mSemanticName,"POSITION")==0)
		{
			mVertexComponents |= VC_POSITION;
		}
		else if (strcmp(it->mSemanticName, "NORMAL")==0)
		{
			mVertexComponents |= VC_NORMAL;
		}
		else if (strcmp(it->mSemanticName, "COLOR")==0)
		{
			mVertexComponents |= VC_COLOR;
		}
		else if (strcmp(it->mSemanticName, "TEXCOORD")==0)
		{
			mVertexComponents |= VC_TEXCOORD;
		}
		else if (strcmp(it->mSemanticName, "TANGENT")==0)
		{
			mVertexComponents |= VC_TANGENT;
		}
		else if (strcmp(it->mSemanticName, "BLENDINDICES") == 0)
		{
			mVertexComponents |= VC_BLENDINDICES;
		}
		else
		{
			assert(0);
		}
	}
}

}