/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

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