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
#include "DebugHud.h"
#include "Renderer.h"
#include "Font.h"
#include "FBTimer/Timer.h"
#include "RendererOptions.h"
#include "RenderStates.h"
#include "InputLayout.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "Material.h"
#include "Camera.h"
#include "FBSceneManager/IScene.h"
#include "EssentialEngineData/shaders/Constants.h"
#undef DrawText
using namespace fb;

class DebugHud::Impl{
public:
	struct TextData
	{
		TextData(const Vec2I& pos, WCHAR* text, const Color& color, Real size, Real secs)
			: mPos(pos), mText(text), mColor(color), mSecs(secs), mSize(size), mDuration(secs)
			, mWidth(10)
		{
		}

		Vec2I mPos;
		std::wstring mText;
		Color mColor;
		Real mSecs;
		Real mDuration;
		Real mSize;
		Real mWidth;
	};

	struct Line
	{
		Vec3 mStart;
		unsigned mColor;

		Vec3 mEnd;
		unsigned mColore;
	};

	struct Quad
	{
		Vec2I mPos;
		Vec2I mSize;
		Color mColor;
	};

	struct Sphere
	{
		Vec3 mPos;
		Real mRadius;
		Color mColor;
	};

	struct Box
	{
		Vec3 mPos;
		Vec3 mExtent;
		Color mColor;
		Real mAlpha;
	};

	struct Triangle
	{
		Vec3 a;
		Vec3 b;
		Vec3 c;
		Color mColor;
		Real mAlpha;
	};
	struct LINE_VERTEX
	{
		LINE_VERTEX(const Vec3& pos, unsigned c)
			: v(pos), color(c)
		{
		}
		Vec3 v;
		unsigned color;
	};	
	// 12 : Real3, 4 : ubyte4
	static const unsigned LINE_STRIDE = 16;	
	// 12 : Real3, 16 : color
	static const unsigned MAX_LINE_VERTEX = 500;
	typedef std::queue<TextData> MessageQueue;
	MessageQueue mTexts;
	typedef std::list<TextData> MessageBuffer;
	std::map<Vec2I, MessageBuffer> mTextsForDur;
	std::vector<Line> mScreenLines;
	std::vector<Line> mWorldLines;
	std::vector<Quad> mQuads;
	std::vector<Line> mWorldLinesBeforeAlphaPass;
	OBJECT_CONSTANTS mObjectConstants;
	OBJECT_CONSTANTS mObjectConstants_WorldLine;

	std::vector<Sphere> mSpheres;
	std::vector<Box> mBoxes;
	std::vector<Triangle> mTriangles;

	ShaderPtr mLineShader;
	InputLayoutPtr mInputLayout;
	/*SmartPtr<IInputLayout> mHdrInputLayout;*/
	VertexBufferPtr mVertexBuffer;
	/*SmartPtr<IVertexBuffer> mHdrVertexBuffer;*/
	MaterialPtr mTriMaterial;

	RenderStatesPtr mRenderStates;
	//MeshObjectPtr mSphereMesh;
	//MeshObjectPtr mBoxMesh;
	unsigned mLastPreRendered;

	//---------------------------------------------------------------------------
	Impl(){
		mObjectConstants.gWorld.MakeIdentity();

		mObjectConstants_WorldLine.gWorld.MakeIdentity();
		mObjectConstants_WorldLine.gWorldViewProj.MakeIdentity();
		auto& renderer = Renderer::GetInstance();
		mLineShader = renderer.CreateShader(
			"EssentialEnginedata/shaders/Line.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS, SHADER_DEFINES());

		mInputLayout = renderer.GetInputLayout(DEFAULT_INPUTS::POSITION_COLOR, mLineShader);
		mVertexBuffer = renderer.CreateVertexBuffer(0, LINE_STRIDE, MAX_LINE_VERTEX,
			BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);

		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthEnable = false;
		mRenderStates = RenderStates::Create();
		mRenderStates->CreateDepthStencilState(ddesc);		
	}

	void SetRenderTargetSize(const Vec2I& size){
		mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
			(Real)size.x,
			(Real)size.y,
			0.f, 1.0f);
	}

	//----------------------------------------------------------------------------
	void DrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text,
		const Color& color, Real size)
	{
		auto it = mTextsForDur.find(pos);
		if (it == mTextsForDur.end()){
			it = mTextsForDur.insert(std::make_pair(pos, MessageBuffer())).first;
		}
		it->second.insert(it->second.begin(), TextData(pos, text, color, size, secs));
		auto& renderer = Renderer::GetInstance();
		auto font = renderer.GetFont(size);
		if (font){
			it->second.begin()->mWidth = font->GetTextWidth((const char*)text);
		}
		/*while (it->second.size() > 10)
		{
		it->second.pop_front();
		}*/
	}

	void ClearDurationTexts() {
		mTextsForDur.clear();
	}

	//----------------------------------------------------------------------------
	void DrawText(const Vec2I& pos, WCHAR* text,
		const Color& color, Real size)
	{
		mTexts.push(TextData(pos, text, color, size, 0.f));
	}

	void Draw3DText(const Vec3& pos, WCHAR* text, const Color& color, Real size)
	{
		auto& renderer = Renderer::GetInstance();
		auto cam = renderer.GetCamera();
		if (cam)
		{
			Vec2I spos = cam->WorldToScreen(pos);
			mTexts.push(TextData(spos, text, color, size, 0.f));
		}
	}

	//----------------------------------------------------------------------------
	void DrawLine(const Vec3& start, const Vec3& end,
		const Color& color0, const Color& color1)
	{
		Line line;
		line.mStart = start;
		line.mColor = color0;

		line.mEnd = end;
		line.mColore = color1;

		mWorldLines.push_back(line);
	}

	void DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color){

		mQuads.push_back(Quad());
		auto& q = mQuads.back();
		q.mPos = pos;
		q.mSize = size;
		q.mColor = color;
	}

	void DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
		const Color& color1)
	{
		Line line;
		line.mStart = start;
		line.mColor = color0;

		line.mEnd = end;
		line.mColore = color1;

		mWorldLinesBeforeAlphaPass.push_back(line);
	}

	void DrawLine(const Vec2I& start, const Vec2I& end,
		const Color& color0, const Color& color1)
	{
		Line line;
		line.mStart = Vec3((Real)start.x, (Real)start.y, 0.f);
		line.mColor = color0;

		line.mEnd = Vec3((Real)end.x, (Real)end.y, 0.f);
		line.mColore = color1;

		mScreenLines.push_back(line);
	}

	void DrawSphere(const Vec3& pos, Real radius, const Color& color)
	{
		mSpheres.push_back(Sphere());
		auto& s = mSpheres.back();
		s.mPos = pos;
		s.mRadius = radius;
		s.mColor = color;
	}
	void DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha)
	{
		mBoxes.push_back(Box());
		auto& b = mBoxes.back();
		b.mPos = (boxMin + boxMax) * .5f;
		b.mExtent = boxMax - b.mPos;
		b.mColor = color;
		b.mAlpha = alpha;
	}
	void DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha)
	{
		mTriangles.push_back(Triangle());
		auto& t = mTriangles.back();
		t.a = a;
		t.b = b;
		t.c = c;
		t.mColor = color;
		t.mAlpha = alpha;
	}

	//----------------------------------------------------------------------------
	void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut)
	{
		auto& renderer = Renderer::GetInstance();
		if (!renderer.GetRendererOptions()->r_debugDraw)
			return;
		if (renderParam.mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;

		mObjectConstants_WorldLine.gWorldViewProj = 
			renderer.GetCamera()->GetMatrix(Camera::ViewProj);

		//Profiler profile("void Render()");
		RenderEventMarker marker("Debug Hud");

		// object constant buffer
		mRenderStates->Bind();
		mInputLayout->Bind();
		mLineShader->Bind();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
		renderer.UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
		unsigned lineCount = mWorldLines.size();
		if (lineCount > 0)
		{
			unsigned processedLines = 0;
			while (lineCount > processedLines)
			{
				MapData mapped = mVertexBuffer->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
				if (mapped.pData)
				{
					unsigned totalVertexCount = lineCount * 2;
					unsigned numVertex = 0;
					for (numVertex = 0; processedLines < lineCount && numVertex < MAX_LINE_VERTEX; numVertex += 2)
					{
						unsigned base = numVertex*LINE_STRIDE;
						memcpy((char*)mapped.pData + base, &mWorldLines[processedLines], LINE_STRIDE * 2);
						processedLines++;
					}
					mVertexBuffer->Unmap(0);
					mVertexBuffer->Bind();
					renderer.Draw(numVertex, 0);
				}
			}
			mWorldLines.clear();
		}

		const auto& size = renderer.GetMainRenderTargetSize();
		mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
			(Real)size.x,
			(Real)size.y,
			0.f, 1.0f);
		mObjectConstants.gWorld.MakeIdentity();
		renderer.UpdateObjectConstantsBuffer(&mObjectConstants);
		lineCount = mScreenLines.size();
		if (lineCount > 0)
		{
			while (lineCount)
			{
				MapData mapped = mVertexBuffer->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
				if (mapped.pData)
				{
					unsigned totalVertexCount = lineCount * 2;
					unsigned numVertex = 0;
					for (numVertex = 0; numVertex < totalVertexCount && numVertex < MAX_LINE_VERTEX; numVertex += 2)
					{
						unsigned base = numVertex*LINE_STRIDE;
						memcpy((char*)mapped.pData + base, &mScreenLines[numVertex / 2], LINE_STRIDE * 2);
						lineCount--;
					}
					mVertexBuffer->Unmap(0);
					mVertexBuffer->Bind();
					renderer.Draw(numVertex, 0);
				}
			}
			mScreenLines.clear();
		}
		bool updateRs = true;
		for (auto& q : mQuads){
			renderer.DrawQuad(q.mPos, q.mSize, q.mColor, updateRs);
			updateRs = false;
		}
		mQuads.clear();

		auto pFont = renderer.GetFont(20.f);
		Real curFontHeight = 20.f;

		pFont->PrepareRenderResources();
		pFont->SetRenderStates(false, false);
		while (!mTexts.empty())
		{
			const TextData& textData = mTexts.front();
			if (curFontHeight != textData.mSize){
				pFont = renderer.GetFont(textData.mSize);
				curFontHeight = textData.mSize;
			}
			pFont->Write((Real)textData.mPos.x, (Real)textData.mPos.y, 0.5f, textData.mColor.Get4Byte(),
				(const char*)textData.mText.c_str(), -1, Font::FONT_ALIGN_LEFT);
			mTexts.pop();
		}

		// render text for duration
		{
			auto dt = Timer::GetMainTimer()->GetDeltaTime();
			pFont->SetRenderStates();
			auto itDur = mTextsForDur.begin();
			for (; itDur != mTextsForDur.end(); ++itDur)
			{
				Real weight = 1.0f;
				auto& textList = itDur->second;
				auto it = textList.rbegin();
				for (; it != textList.rend(); ++it){
					it->mSecs -= dt * weight;
					weight *= 0.92f;

				}
			}
			itDur = mTextsForDur.begin();
			Real accHeight = 0.f;
			for (; itDur != mTextsForDur.end(); ++itDur)
			{
				int count = 0;
				auto& textList = itDur->second;
				auto it = textList.begin();
				for (; it != textList.end(); ++count){
					if (it->mSecs <= 0.f)
					{
						it = textList.erase(it);
						continue;
					}
					else
					{
						Vec2I drawPos = it->mPos;
						drawPos.y -= Round(accHeight);
						if (drawPos.y < 140){
							for (auto delIt = it; delIt != textList.end();){
								delIt = textList.erase(delIt);
							}
							break;
						}
						if (it->mSize != curFontHeight){
							pFont->SetHeight(it->mSize);
							curFontHeight = it->mSize;
						}
						Color color = it->mColor;
						Real proportion = 1.0f - (it->mSecs / it->mDuration);
						color.a() = 1.0f - (proportion*proportion);
						renderer.DrawQuad(Vec2I(drawPos.x - 4, drawPos.y - (int)it->mSize - 2), Vec2I((int)it->mWidth + 8, (int)it->mSize + 4), Color(0, 0, 0, color.a()*0.7f));
						pFont->PrepareRenderResources();
						pFont->Write((Real)drawPos.x, (Real)drawPos.y, 0.5f, color.Get4Byte(),
							(const char*)it->mText.c_str(), -1, Font::FONT_ALIGN_LEFT);
						accHeight += pFont->GetHeight() + 4.f;


						it++;
					}
				}
			}
			pFont->SetBackToOrigHeight();
		}

		//if (mSphereMesh)
		//{
		//	RenderEventMarker marker("Debug Hud - Spheres");			
		//	renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//	for (auto& sphere : mSpheres)
		//	{
		//		Transformation t;
		//		t.SetScale(Vec3(sphere.mRadius, sphere.mRadius, sphere.mRadius));
		//		t.SetTranslation(sphere.mPos);
		//		t.GetHomogeneous(mObjectConstants.gWorld);
		//		// only world are available. other matrix will be calculated in the shader
		//		//mObjectConstants.gWorldView = renderer.GetCamera()->GetViewMat() * mObjectConstants.gWorld;
		//		//mObjectConstants.gWorldViewProj = renderer.GetCamera()->GetProjMat() * mObjectConstants.gWorldView;
		//		renderer.UpdateObjectConstantsBuffer(&mObjectConstants);
		//		mSphereMesh->GetMaterial()->SetMaterialParameter(0, sphere.mColor.GetVec4());
		//		mSphereMesh->GetMaterial()->Bind(true);
		//		mSphereMesh->RenderSimple();
		//	}
		//}
		//mSpheres.clear();

		/*if (mBoxMesh)
		{
			
			RenderEventMarker mark("DebugHud - Boxes");
			renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			for (auto& box : mBoxes)
			{
				Transformation t;
				t.SetScale(box.mExtent);
				t.SetTranslation(box.mPos);
				t.GetHomogeneous(mObjectConstants.gWorld);

				renderer.UpdateObjectConstantsBuffer(&mObjectConstants);
				Vec4 color = box.mColor.GetVec4();
				color.w = box.mAlpha;
				mBoxMesh->GetMaterial()->SetMaterialParameter(0, color);
				mBoxMesh->GetMaterial()->Bind(true);
				mBoxMesh->RenderSimple();
			}
		}*/

		mBoxes.clear();

		if (mTriMaterial)
		{
			for (auto& tri : mTriangles)
			{
				Vec4 color = tri.mColor.GetVec4();
				color.w = tri.mAlpha;
				renderer.DrawTriangle(tri.a, tri.b, tri.c, color, mTriMaterial);
			}
		}
		mTriangles.clear();
	}

	void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut)
	{
		if (!Renderer::GetInstance().GetRendererOptions()->r_debugDraw)
			return;
		if (renderParam.mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;
		unsigned lineCount = mWorldLinesBeforeAlphaPass.size();
		if (lineCount == 0)
			return;

		RenderEventMarker mark("DebugHud - OnBeforeRenderingTransparents()");
		auto& renderer = Renderer::GetInstance();
		// object constant buffer
		mInputLayout->Bind();
		mLineShader->Bind();
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
		renderer.UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
		if (lineCount > 0)
		{
			while (lineCount)
			{
				MapData mapped = mVertexBuffer->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
				if (mapped.pData)
				{
					unsigned totalVertexCount = lineCount * 2;
					unsigned numVertex = 0;
					for (numVertex = 0; numVertex < totalVertexCount && numVertex < MAX_LINE_VERTEX; numVertex += 2)
					{
						unsigned base = numVertex*LINE_STRIDE;
						memcpy((char*)mapped.pData + base, &mWorldLinesBeforeAlphaPass[numVertex / 2], LINE_STRIDE * 2);
						lineCount--;
					}
					mVertexBuffer->Unmap(0);
					mVertexBuffer->Bind();
					renderer.Draw(numVertex, 0);
				}
			}
			mWorldLinesBeforeAlphaPass.clear();
		}
	}
	
};

//----------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(DebugHud);
DebugHud::DebugHud()
	: mImpl(new Impl)
{
	
}

//----------------------------------------------------------------------------
DebugHud::~DebugHud()
{
}

void DebugHud::SetRenderTargetSize(const Vec2I& size){
	mImpl->SetRenderTargetSize(size);
}

//----------------------------------------------------------------------------
void DebugHud::DrawTextForDuration(Real secs, const Vec2I& pos, WCHAR* text, 
		const Color& color, Real size)
{
	mImpl->DrawTextForDuration(secs, pos, text, color, size);
}

void DebugHud::ClearDurationTexts() {
	mImpl->ClearDurationTexts();
}

//----------------------------------------------------------------------------
void DebugHud::DrawText(const Vec2I& pos, WCHAR* text, 
	const Color& color, Real size)
{
	mImpl->DrawText(pos, text, color, size);
}

void DebugHud::Draw3DText(const Vec3& pos, WCHAR* text, const Color& color, Real size)
{
	mImpl->Draw3DText(pos, text, color, size);
}

//----------------------------------------------------------------------------
void DebugHud::DrawLine(const Vec3& start, const Vec3& end, 
	const Color& color0, const Color& color1)
{
	mImpl->DrawLine(start, end, color0, color1);
}

void DebugHud::DrawQuad(const Vec2I& pos, const Vec2I& size, const Color& color){
	mImpl->DrawQuad(pos, size, color);
}

void DebugHud::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
	const Color& color1)
{
	mImpl->DrawLineBeforeAlphaPass(start, end, color0, color1);
}

void DebugHud::DrawLine(const Vec2I& start, const Vec2I& end, 
	const Color& color0, const Color& color1)
{
	mImpl->DrawLine(start, end, color0, color1);
}

void DebugHud::DrawSphere(const Vec3& pos, Real radius, const Color& color)
{
	mImpl->DrawSphere(pos, radius, color);
}
void DebugHud::DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha)
{
	mImpl->DrawBox(boxMin, boxMax, color, alpha);
}
void DebugHud::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha)
{
	mImpl->DrawTriangle(a, b, c, color, alpha);
}

void DebugHud::OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut)
{
	mImpl->OnBeforeRenderingTransparents(scene, renderParam, renderParamOut);
}

//----------------------------------------------------------------------------
void DebugHud::Render(const RenderParam& renderParam, RenderParamOut* renderParamOut)
{
	mImpl->Render(renderParam, renderParamOut);
}