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
#include "FractalTest.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Shader.h"
#include "EssentialEngineData/shaders/Constants.h"
#include "FBSerializationLib/Serialization.h"
#include "FBRenderer/GeometryBuffer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Camera.h"
using namespace fb;

class FractalTest::Impl {
public:
	ShaderPtr mFractalPS;
	GeometryBuffer mSphere;
	MaterialPtr mMaterial;
	Impl() {		
		mSphere = GeometryBuffer::CreateSphereMesh(10.f, 10, 20);
		mMaterial = Renderer::GetInstance().CreateMaterial("EssentialEngineData/materials/FractalSphere.material");
		auto cam = Renderer::GetInstance().GetCamera();
		//cam->SetTargetPos(Vec3(0, 0, 0));
	}
	void BeforeUIRendering(HWindowId hwndId, HWindow hwnd) {
		if (mMaterial) {
			auto& renderer = Renderer::GetInstance();
			auto cam = renderer.GetCamera();			
			OBJECT_CONSTANTS objConstants = {
				Mat44::IDENTITY,
				cam->GetMatrix(ICamera::MatrixType::View),
				cam->GetMatrix(ICamera::MatrixType::ViewProj),
			};
			renderer.UpdateObjectConstantsBuffer(&objConstants);

			mSphere.Bind();
			mMaterial->Bind();
			
			renderer.BindSystemTexture(SystemTextures::Permutation);
			renderer.BindSystemTexture(SystemTextures::ValueNoise);
			renderer.DrawIndexed(mSphere.mIndexBuffer->GetNumIndices(), 0, 0);						
		}		
	}
};

FB_IMPLEMENT_STATIC_CREATE(FractalTest);

FractalTest::FractalTest() 
	: mImpl(new Impl)
{

}
FractalTest::~FractalTest() = default;


void FractalTest::BeforeUIRendering(HWindowId hwndId, HWindow hwnd) {
	mImpl->BeforeUIRendering(hwndId, hwnd);
	
}	