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
#include "DustRenderer.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/VertexBuffer.h"
using namespace fb;

class DustRenderer::Impl{
public:
	DustRenderer* mSelf;
	Vec3 mMin;
	VertexBufferPtr mVBColor;
	VertexBufferPtr mVBPos;
	MaterialPtr mMaterial;

	//---------------------------------------------------------------------------
	Impl(DustRenderer* self)
		: mSelf(self)
	{
	}

	//---------------------------------------------------------------------------
	// SceneObject Interfaces
	//---------------------------------------------------------------------------
	void PreRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}

	void Render(const RenderParam& param, RenderParamOut* paramOut){
		if (mSelf->HasObjFlag(SceneObjectFlag::Hide) || param.mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;

		if (!mVBPos)
			return;

		RenderEventMarker mark("DustRenderer");

		mMaterial->Bind(true);
		auto& renderer = Renderer::GetInstance();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_POINTLIST);

		const unsigned int numBuffers = 2;
		VertexBufferPtr buffers[numBuffers] = { mVBPos, mVBColor };
		unsigned int strides[numBuffers] = { mVBPos->GetStride(),
			mVBColor ? mVBColor->GetStride() : 0 };
		unsigned int offsets[numBuffers] = { 0, 0 };
		renderer.SetVertexBuffers(0, numBuffers, buffers, strides, offsets);
		renderer.Draw(mVBPos->GetNumVertices(), 0);
	}

	void PostRender(const RenderParam& param, RenderParamOut* paramOut){
		
	}

	//---------------------------------------------------------------------------
	void Initialize(const Vec3& min, const Vec3& max, size_t count,
		const Color& cmin, const Color& cmax, float normalizeDist){
		if (count == 0)
			return;
		mMin = min;
		std::vector<Vec3> pos;
		pos.reserve(count);
		std::vector<DWORD> color;
		color.reserve(count);

		for (size_t i = 0; i<count; i++)
		{
			pos.push_back(Random(min, max));
			color.push_back(Random(cmin, cmax).Get4Byte());
		}

		if (normalizeDist>0.0f)
		{
			for (auto it : pos){
				it.Normalize();
				it *= normalizeDist;
			}
		}
		auto& renderer = Renderer::GetInstance();

		mVBPos = renderer.CreateVertexBuffer(&pos[0],
			sizeof(Vec3f), count, BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
		assert(mVBPos);

		mVBColor = renderer.CreateVertexBuffer(&color[0],
			sizeof(DWORD), count, BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);

		Vec3 len = max - min;
		mSelf->GetBoundingVolume()->SetCenter(min + len / 2.f);
		mSelf->GetBoundingVolume()->SetRadius(len.Length());
		*mSelf->GetBoundingVolumeWorld() = *mSelf->GetBoundingVolume();
	}

	void SetMaterial(const char* filepath, int pass){
		mMaterial = Renderer::GetInstance().CreateMaterial(filepath);
	}

	void SetMaterial(MaterialPtr pMat, int pass){
		mMaterial = pMat;
	}

	MaterialPtr GetMaterial(int pass = 0) const{
		return mMaterial;
	}

	const Vec3& GetMin() const{
		return mMin;
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(DustRenderer);
DustRenderer::DustRenderer()
	: mImpl(new Impl(this))
{
}
DustRenderer::~DustRenderer(){

}

void DustRenderer::SetMaterial(const char* filepath, int pass) {
	mImpl->SetMaterial(filepath, pass);
}

void DustRenderer::SetMaterial(MaterialPtr pMat, int pass) {
	mImpl->SetMaterial(pMat, pass);
}

MaterialPtr DustRenderer::GetMaterial(int pass) const {
	return mImpl->GetMaterial(pass);
}

void DustRenderer::PreRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PreRender(param, paramOut);
}

void DustRenderer::Render(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->Render(param, paramOut);
}

void DustRenderer::PostRender(const RenderParam& param, RenderParamOut* paramOut) {
	mImpl->PostRender(param, paramOut);
}

void DustRenderer::Initialize(const Vec3& min, const Vec3& max, size_t count,	const Color& cmin, const Color& cmax, float normalizeDist) {
	mImpl->Initialize(min, max, count, cmin, cmax, normalizeDist);
}

const Vec3& DustRenderer::GetMin() const {
	return mImpl->GetMin();
}

