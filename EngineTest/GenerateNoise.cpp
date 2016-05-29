#include "stdafx.h"
#include "GenerateNoise.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Shader.h"
#include "FBRenderer/Texture.h"
using namespace fb;

class GenerateNoise::Impl {
public:
	Impl() {		
		
	}
};

FB_IMPLEMENT_STATIC_CREATE(GenerateNoise);

GenerateNoise::GenerateNoise() 
	: mImpl(new Impl)
{

}
GenerateNoise::~GenerateNoise() = default;
