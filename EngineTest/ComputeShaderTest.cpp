#include "stdafx.h"
#include "ComputeShaderTest.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Shader.h"
using namespace fb;

class ComputeShaderTest::Impl {
public:
	ShaderPtr mCS;	
	Impl() {
		const char* hlsl = R"+__+(
RWByteAddressBuffer BufferOut : register(u0);
[numthreads(512, 1, 1)]
void CSMain( uint3 GTid : SV_GroupThreadID){
	BufferOut.Store(GTid.x*4, GTid.x);
}
)+__+";

		auto& renderer = Renderer::GetInstance();
		auto shader = renderer.CompileComputeShader(hlsl, "CSMain", {});
		if (shader) {
			ByteArray resultData;
			auto success = shader->RunComputeShader(0, 0, 1, 1, 1, resultData, 512*4);
			if (success) {
				unsigned* p = (unsigned*)&resultData[0];
				int a = 0;
				a++;
			}
		}

		
	}
	
};

FB_IMPLEMENT_STATIC_CREATE(ComputeShaderTest);

ComputeShaderTest::ComputeShaderTest()
	: mImpl(new Impl)
{

}
ComputeShaderTest::~ComputeShaderTest() = default;