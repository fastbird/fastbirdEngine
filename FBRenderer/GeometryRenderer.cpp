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
#include "GeometryRenderer.h"
#include "Texture.h"
#include "Shader.h"
#include "InputLayout.h"
#include "Material.h"
#include "VertexBuffer.h"
#include "RenderStates.h"
#include "Renderer.h"
#include "RenderParam.h"
#include "RendererOptions.h"
#include "ResourceProvider.h"
#include "ResourceTypes.h"
#include "Camera.h"
#include "FBSceneManager/IScene.h"
#include "EssentialEngineData/shaders/Constants.h"
using namespace fb;

class GeometryRenderer::Impl{
public:
	struct Line
	{
		Vec3 mStart;
		unsigned mColor;

		Vec3 mEnd;
		unsigned mColore;
	};

	struct ThickLine
	{
		Vec3 mStart;
		unsigned mColor;
		Vec3 mEnd;
		unsigned mColore;

		Real mThickness;
		std::string mStrTexture;
		TexturePtr mTexture;
		bool mTextureFlow;
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
	static const unsigned LINE_STRIDE=16;

	struct THICK_LINE_VERTEX
	{
		THICK_LINE_VERTEX(const Vec3& pos, unsigned c, const Vec4& texcoord, const Vec3& nextPos)
			: v(pos), color(c), uv(texcoord), next(nextPos)
		{
		}
		Vec3 v;
		unsigned color;
		Vec4 uv;
		Vec3 next;
	};
	static const unsigned THICK_LINE_STRIDE=44;
	static const unsigned MAX_LINE_VERTEX=500;
	std::vector<Line> mWorldLines;
	std::vector<Line> mWorldLinesBeforeAlphaPass;
	std::vector<ThickLine> mThickLines;
	OBJECT_CONSTANTS mObjectConstants;
	OBJECT_CONSTANTS mObjectConstants_WorldLine;

	std::vector<Sphere> mSpheres;
	std::vector<Box> mBoxes;
	std::vector<Triangle> mTriangles;

	ShaderPtr mLineShader;
	InputLayoutPtr mInputLayout;

	MaterialPtr mThickLineMaterial;

	VertexBufferPtr mVertexBuffer;
	VertexBufferPtr mVertexBufferThickLine;
	MaterialPtr mTriMaterial;

	DepthStencilStatePtr mDepthStencilState;
	RasterizerStatePtr mRasterizerState;

	//---------------------------------------------------------------------------
	Impl(){
		mObjectConstants.gWorld.MakeIdentity();
		mObjectConstants_WorldLine.gWorld.MakeIdentity();
		mObjectConstants_WorldLine.gWorldViewProj.MakeIdentity();
		auto& renderer = Renderer::GetInstance();
		mLineShader = renderer.CreateShader(
			"EssentialEnginedata/shaders/Line.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);
		mInputLayout = renderer.GetInputLayout(DEFAULT_INPUTS::POSITION_COLOR, mLineShader);
		mThickLineMaterial = renderer.CreateMaterial("EssentialEngineData/materials/ThickLine.material");
		mVertexBuffer = renderer.CreateVertexBuffer(0, LINE_STRIDE, MAX_LINE_VERTEX,
			BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		mVertexBufferThickLine = renderer.CreateVertexBuffer(0, THICK_LINE_STRIDE, MAX_LINE_VERTEX,
			BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
		DEPTH_STENCIL_DESC ddesc;
		ddesc.DepthEnable = true;
		mDepthStencilState = renderer.CreateDepthStencilState(ddesc);
		RASTERIZER_DESC desc;
		mRasterizerState = renderer.CreateRasterizerState(desc);		
		mThickLines.reserve(1000);
	}

	//----------------------------------------------------------------------------
	void SetRenderTargetSize(const Vec2I& size){
		mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
			(Real)size.x,
			(Real)size.y,
			0.f, 1.0f);
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

	void DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness,
		const char* texture, bool textureFlow)
	{
		ThickLine line;
		line.mStart = start;
		line.mEnd = end;
		line.mColor = color0;
		line.mColore = color1;
		line.mThickness = thickness;
		if (texture && strlen(texture) != 0)
		{
			line.mStrTexture = texture;
			auto& renderer = Renderer::GetInstance();
			line.mTexture = renderer.CreateTexture(texture, true);
		}
		line.mTextureFlow = textureFlow;

		mThickLines.push_back(line);
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
	// not using this function
	void OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut)
	{
		auto& renderer = Renderer::GetInstance();
		if (!renderer.GetRendererOptions()->r_debugDraw)
			return;		
		unsigned lineCount = mWorldLinesBeforeAlphaPass.size();
		if (lineCount == 0)
			return;

		RenderEventMarker marker("Geometry Render: OnBeforeTransparents");
		// object constant buffer
		if (lineCount > 0)
		{
			mInputLayout->Bind();
			mLineShader->Bind();
			renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
			renderer.UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
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

	//----------------------------------------------------------------------------
	void Render(const RenderParam& renderParam, RenderParamOut* renderParamOut)
	{
		auto& renderer = Renderer::GetInstance();
		if (!renderer.GetRendererOptions()->r_debugDraw)
			return;
		if (renderParam.mRenderPass != RENDER_PASS::PASS_NORMAL)
			return;

		RenderEventMarker marker("Geometry Render");

		mObjectConstants_WorldLine.gWorldViewProj = renderer.GetCamera()->GetMatrix(Camera::ViewProj);
		
		// object constant buffer
		mRasterizerState->Bind();
		mDepthStencilState->Bind(0);
		auto provider = renderer.GetResourceProvider();
		provider->BindBlendState(ResourceTypes::BlendStates::AlphaBlend);
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

		std::sort(mThickLines.begin(), mThickLines.end(),
			[](const ThickLine& left, const ThickLine& right)->bool
		{
			auto cmp = left.mStrTexture.compare(right.mStrTexture);
			if (cmp < 0)
			{
				return true;
			}
			else if (cmp == 0)
			{
				if (left.mTextureFlow < right.mTextureFlow)
					return true;
			}
			return false;

		}
		);
		renderer.SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		lineCount = mThickLines.size();
		auto& camDir = renderer.GetCamera()->GetDirection();
		if (lineCount > 0)
		{
			unsigned processedLines = 0;
			while (lineCount > processedLines)
			{
				MapData mapped = mVertexBufferThickLine->Map(0, MAP_TYPE_WRITE_DISCARD, MAP_FLAG_NONE);
				if (mapped.pData)
				{
					const auto& curProcessingLine = mThickLines[processedLines];
					std::string curTexture = curProcessingLine.mStrTexture;
					bool curTextureFlow = curProcessingLine.mTextureFlow;
					mThickLineMaterial->SetTexture(curProcessingLine.mTexture, BINDING_SHADER_PS, 0);
					bool refreshDefines = false;
					if (curProcessingLine.mTexture)
					{
						refreshDefines = mThickLineMaterial->AddShaderDefine("DIFFUSE_TEXTURE", "1") || refreshDefines;
					}
					else
					{
						refreshDefines = mThickLineMaterial->RemoveShaderDefine("DIFFUSE_TEXTURE") || refreshDefines;
					}
					if (curProcessingLine.mTextureFlow)
					{
						refreshDefines = mThickLineMaterial->AddShaderDefine("_TEXTURE_FLOW", "1") || refreshDefines;
					}
					else
					{
						refreshDefines = mThickLineMaterial->RemoveShaderDefine("_TEXTURE_FLOW") || refreshDefines;
					}

					/*if (refreshDefines)
					{
						mThickLineMaterial->ApplyShaderDefines();
					}*/

					mThickLineMaterial->Bind(true);

					unsigned totalVertexCount = lineCount * 6;
					unsigned numVertex = 0;
					for (numVertex = 0; processedLines < lineCount && numVertex < MAX_LINE_VERTEX; numVertex += 6)
					{
						const auto& curProcessingLine = mThickLines[processedLines];
						if (curProcessingLine.mStrTexture != curTexture || curProcessingLine.mTextureFlow != curTextureFlow)
							break;

						Real thickness = mThickLines[processedLines].mThickness;

						Color colorStart(curProcessingLine.mColor);
						Color colorEnd(curProcessingLine.mColore);
						Vec3 dir = curProcessingLine.mEnd - curProcessingLine.mStart;
						THICK_LINE_VERTEX vertices[6] =
						{
							THICK_LINE_VERTEX(curProcessingLine.mStart, colorStart.Get4Byte(), Vec4(1, 0, thickness, +1.f), curProcessingLine.mEnd),
							THICK_LINE_VERTEX(curProcessingLine.mEnd, colorEnd.Get4Byte(), Vec4(0, 1, thickness, -1.f), curProcessingLine.mEnd + dir),
							THICK_LINE_VERTEX(curProcessingLine.mEnd, colorEnd.Get4Byte(), Vec4(1, 1, thickness, +1.f), curProcessingLine.mEnd + dir),

							THICK_LINE_VERTEX(curProcessingLine.mEnd, colorEnd.Get4Byte(), Vec4(0, 1, thickness, -1.f), curProcessingLine.mEnd + dir),
							THICK_LINE_VERTEX(curProcessingLine.mStart, colorStart.Get4Byte(), Vec4(1, 0, thickness, +1.f), curProcessingLine.mEnd),
							THICK_LINE_VERTEX(curProcessingLine.mStart, colorStart.Get4Byte(), Vec4(0, 0, thickness, -1.f), curProcessingLine.mEnd),

						};
						unsigned base = numVertex*THICK_LINE_STRIDE;
						memcpy((char*)mapped.pData + base, vertices, THICK_LINE_STRIDE * 6);
						processedLines++;
					}
					mVertexBufferThickLine->Unmap(0);
					mVertexBufferThickLine->Bind();
					renderer.Draw(numVertex, 0);
				}
			}
			mThickLines.clear();
			mThickLineMaterial->Unbind();
		}

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
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(GeometryRenderer);
GeometryRenderer::GeometryRenderer()
	: mImpl(new Impl)
{
}

GeometryRenderer::~GeometryRenderer(){

}

void GeometryRenderer::SetRenderTargetSize(const Vec2I& size) {
	mImpl->SetRenderTargetSize(size);
}

void GeometryRenderer::Render(const RenderParam& renderParam, RenderParamOut* renderParamOut) {
	mImpl->Render(renderParam, renderParamOut);
}

void GeometryRenderer::DrawLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1) {
	mImpl->DrawLine(start, end, color0, color1);
}

void GeometryRenderer::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1) {
	mImpl->DrawLineBeforeAlphaPass(start, end, color0, color1);
}

void GeometryRenderer::DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, Real thickness, const char* texture, bool textureFlow) {
	mImpl->DrawTexturedThickLine(start, end, color0, color1, thickness, texture, textureFlow);
}

void GeometryRenderer::DrawSphere(const Vec3& pos, Real radius, const Color& color) {
	mImpl->DrawSphere(pos, radius, color);
}

void GeometryRenderer::DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, Real alpha) {
	mImpl->DrawBox(boxMin, boxMax, color, alpha);
}

void GeometryRenderer::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, Real alpha) {
	mImpl->DrawTriangle(a, b, c, color, alpha);
}

void GeometryRenderer::OnBeforeRenderingTransparents(IScene* scene, const RenderParam& renderParam, RenderParamOut* renderParamOut){
	mImpl->OnBeforeRenderingTransparents(scene, renderParam, renderParamOut);
}

