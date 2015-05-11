#include <Engine/StdAfx.h>
#include <Engine/GeometryRenderer.h>
#include <Engine/ICamera.h>
#include <Engine/IMeshObject.h>
#include <Engine/Renderer.h>

using namespace fastbird;

// 12 : float3, 4 : ubyte4
const unsigned GeometryRenderer::LINE_STRIDE = 16;
// 12 : float3, 8 : uv, 4 : ubyte4
const unsigned GeometryRenderer::THICK_LINE_STRIDE = 24;
const unsigned GeometryRenderer::MAX_LINE_VERTEX = 500;

//----------------------------------------------------------------------------
GeometryRenderer::GeometryRenderer()
{
	const auto& size = gFBEnv->_pInternalRenderer->GetMainRTSize();
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
		(float)size.x,
		(float)size.y,
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();

	mObjectConstants_WorldLine.gWorld.MakeIdentity();
	mObjectConstants_WorldLine.gWorldViewProj.MakeIdentity();

	mLineShader = gFBEnv->pEngine->GetRenderer()->CreateShader(
		"es/shaders/Line.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS);

	mInputLayout = gFBEnv->pRenderer->GetInputLayout(
		DEFAULT_INPUTS::POSITION_COLOR, mLineShader);

	mThickLineMaterial = IMaterial::CreateMaterial("es/materials/ThickLine.material");

	mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(0, LINE_STRIDE, MAX_LINE_VERTEX,
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	mVertexBufferThickLine = gFBEnv->pRenderer->CreateVertexBuffer(0, THICK_LINE_STRIDE, MAX_LINE_VERTEX,
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);	

	DEPTH_STENCIL_DESC ddesc;
	ddesc.DepthEnable = true;
	mDepthStencilState = gFBEnv->pRenderer->CreateDepthStencilState(ddesc);

	RASTERIZER_DESC desc;
	mRasterizerState = gFBEnv->pRenderer->CreateRasterizerState(desc);

	gFBEnv->_pInternalRenderer->GetMainScene()->AddListener(this);

	mSphereMesh = gFBEnv->pEngine->GetMeshObject("es/objects/Sphere.dae");
	mBoxMesh = gFBEnv->pEngine->GetMeshObject("es/objects/DebugBox.dae");

	//mTriMaterial = IMaterial::CreateMaterial("es/materials/DebugTriangle.material");
	//assert(mTriMaterial);

	mThickLines.reserve(1000);
}

//----------------------------------------------------------------------------
GeometryRenderer::~GeometryRenderer()
{
	if (gFBEnv->_pInternalRenderer->GetMainScene())
		gFBEnv->_pInternalRenderer->GetMainScene()->RemoveListener(this);

	gFBEnv->pEngine->ReleaseMeshObject(mSphereMesh);
	gFBEnv->pEngine->ReleaseMeshObject(mBoxMesh);
}

//----------------------------------------------------------------------------
void GeometryRenderer::DrawLine(const Vec3& start, const Vec3& end,
	const Color& color0, const Color& color1)
{
	Line line;
	line.mStart = start;
	line.mColor = color0;

	line.mEnd = end;
	line.mColore = color1;

	mWorldLines.push_back(line);
}

void GeometryRenderer::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
	const Color& color1)
{
	Line line;
	line.mStart = start;
	line.mColor = color0;

	line.mEnd = end;
	line.mColore = color1;

	mWorldLinesBeforeAlphaPass.push_back(line);
}

void GeometryRenderer::DrawTexturedThickLine(const Vec3& start, const Vec3& end, const Color& color0, const Color& color1, float thickness,
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
		line.mTexture = gFBEnv->pRenderer->CreateTexture(texture);
	}	
	line.mTextureFlow = textureFlow;

	mThickLines.push_back(line);
}

void GeometryRenderer::DrawSphere(const Vec3& pos, float radius, const Color& color)
{
	mSpheres.push_back(Sphere());
	auto& s = mSpheres.back();
	s.mPos = pos;
	s.mRadius = radius;
	s.mColor = color;
}
void GeometryRenderer::DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha)
{
	mBoxes.push_back(Box());
	auto& b = mBoxes.back();
	b.mPos = (boxMin + boxMax) * .5f;
	b.mExtent = boxMax - b.mPos;
	b.mColor = color;
	b.mAlpha = alpha;
}
void GeometryRenderer::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha)
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
void GeometryRenderer::PreRender()
{
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;
	mObjectConstants_WorldLine.gWorldViewProj =
		gFBEnv->pRenderer->GetCamera()->GetViewProjMat();
}

void GeometryRenderer::OnBeforeRenderingTransparents()
{
	if (!gFBEnv->pConsole->GetEngineCommand()->r_debugDraw)
		return;
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;
	unsigned lineCount = mWorldLinesBeforeAlphaPass.size();
	if (lineCount == 0)
		return;

	D3DEventMarker mark("GeometryRenderer::OnBeforeRenderingTransparents()");
	PreRender();

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();
	// object constant buffer
	
	if (lineCount > 0)
	{
		mInputLayout->Bind();
		mLineShader->Bind();
		pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
		pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
		while (lineCount)
		{
			MapData mapped = mVertexBuffer->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
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
				mVertexBuffer->Unmap();
				mVertexBuffer->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mWorldLinesBeforeAlphaPass.clear();
	}
}

//----------------------------------------------------------------------------
void GeometryRenderer::Render()
{
	if (!gFBEnv->pConsole->GetEngineCommand()->r_debugDraw)
		return;
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;
	D3DEventMarker mark("GeometryRenderer::Render()");
	
	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();
	// object constant buffer
	mRasterizerState->Bind();
	mDepthStencilState->Bind(0);
	pRenderer->SetAlphaBlendState();
	mInputLayout->Bind();
	mLineShader->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
	pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
	unsigned lineCount = mWorldLines.size();
	if (lineCount > 0)
	{
		unsigned processedLines = 0;
		while (lineCount > processedLines)
		{
			MapData mapped = mVertexBuffer->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
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
				mVertexBuffer->Unmap();
				mVertexBuffer->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mWorldLines.clear();
	}

	std::sort(mThickLines.begin(), mThickLines.end(), 
		[](const ThickLine& left, const ThickLine& right)->bool
		{
		auto cmp = left.mStrTexture.compare(right.mStrTexture);
			if ( cmp < 0)
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
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	lineCount = mThickLines.size();
	auto& camDir = gFBEnv->pRenderer->GetCamera()->GetForward();
	if (lineCount > 0)
	{
		unsigned processedLines = 0;
		while (lineCount > processedLines)
		{
			MapData mapped = mVertexBufferThickLine->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
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

				if (refreshDefines)
				{
					mThickLineMaterial->ApplyShaderDefines();
				}

				mThickLineMaterial->Bind(true);

				unsigned totalVertexCount = lineCount * 6;
				unsigned numVertex = 0;
				for (numVertex = 0; processedLines < lineCount && numVertex < MAX_LINE_VERTEX; numVertex += 6)
				{
					const auto& curProcessingLine = mThickLines[processedLines];
					if (curProcessingLine.mStrTexture != curTexture || curProcessingLine.mTextureFlow != curTextureFlow)
						break;

					float halfThick = mThickLines[processedLines].mThickness * .5f;
					
					Color colorStart(curProcessingLine.mColor);
					Color colorEnd(curProcessingLine.mColore);
					auto left = camDir.Cross((curProcessingLine.mEnd - curProcessingLine.mStart).NormalizeCopy());
					auto length = left.Normalize();
					if (length < 0.05f)
					{
						left = gFBEnv->pRenderer->GetCamera()->GetRight();
					}
					THICK_LINE_VERTEX vertices[6] =
					{
						THICK_LINE_VERTEX(curProcessingLine.mStart + left * halfThick, colorStart.Get4Byte(),
						Vec2(1, 0)),
						THICK_LINE_VERTEX(curProcessingLine.mEnd - left * halfThick, colorEnd.Get4Byte(),
						Vec2(0, 1)),
						THICK_LINE_VERTEX(curProcessingLine.mEnd + left * halfThick, colorEnd.Get4Byte(),
						Vec2(1, 1)),						

						THICK_LINE_VERTEX(curProcessingLine.mEnd - left * halfThick, colorEnd.Get4Byte(),
						Vec2(0, 1)),
						THICK_LINE_VERTEX(curProcessingLine.mStart + left * halfThick, colorStart.Get4Byte(),
						Vec2(1, 0)),
						THICK_LINE_VERTEX(curProcessingLine.mStart - left * halfThick, colorStart.Get4Byte(),
						Vec2(0, 0)),
						
					};
					unsigned base = numVertex*THICK_LINE_STRIDE;
					memcpy((char*)mapped.pData + base, vertices, THICK_LINE_STRIDE * 6);
					processedLines++;
				}
				mVertexBufferThickLine->Unmap();
				mVertexBufferThickLine->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mThickLines.clear();
		mThickLineMaterial->Unbind();
	}

	if (mSphereMesh)
	{
		D3DEventMarker mark("DebugHud::Render - Spheres()");
		gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (auto& sphere : mSpheres)
		{
			Transformation t;
			t.SetScale(Vec3(sphere.mRadius, sphere.mRadius, sphere.mRadius));
			t.SetTranslation(sphere.mPos);
			t.GetHomogeneous(mObjectConstants.gWorld);
			// only world are available. other matrix will be calculated in the shader
			mObjectConstants.gWorldView = gFBEnv->pRenderer->GetCamera()->GetViewMat() * mObjectConstants.gWorld;
			mObjectConstants.gWorldViewProj = gFBEnv->pRenderer->GetCamera()->GetProjMat() * mObjectConstants.gWorldView;
			gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);
			mSphereMesh->GetMaterial()->SetMaterialParameters(0, sphere.mColor.GetVec4());
			mSphereMesh->GetMaterial()->Bind(true);
			mSphereMesh->RenderSimple();
		}
	}
	mSpheres.clear();

	if (mBoxMesh)
	{
		D3DEventMarker mark("DebugHud::Render - Boxes()");
		gFBEnv->pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for (auto& box : mBoxes)
		{
			Transformation t;
			t.SetScale(box.mExtent);
			t.SetTranslation(box.mPos);
			t.GetHomogeneous(mObjectConstants.gWorld);

			gFBEnv->pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);
			Vec4 color = box.mColor.GetVec4();
			color.w = box.mAlpha;
			mBoxMesh->GetMaterial()->SetMaterialParameters(0, color);
			mBoxMesh->GetMaterial()->Bind(true);
			mBoxMesh->RenderSimple();
		}
	}
	mBoxes.clear();

	if (mTriMaterial)
	{
		for (auto& tri : mTriangles)
		{
			Vec4 color = tri.mColor.GetVec4();
			color.w = tri.mAlpha;
			gFBEnv->pRenderer->DrawTriangleNow(tri.a, tri.b, tri.c, color, mTriMaterial);
		}
	}
	mTriangles.clear();

	PostRender();
}
//----------------------------------------------------------------------------
void GeometryRenderer::PostRender()
{
}