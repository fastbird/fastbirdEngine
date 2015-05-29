#include <Engine/StdAfx.h>
#include <Engine/DebugHud.h>
#include <Engine/ICamera.h>
#include <Engine/IMeshObject.h>
#include <Engine/Renderer.h>

using namespace fastbird;

// 12 : float3, 4 : ubyte4
const unsigned DebugHud::LINE_STRIDE = 16;
// 12 : float3, 16 : color
//const unsigned DebugHud::HDR_LINE_STRIDE = 28;
const unsigned DebugHud::MAX_LINE_VERTEX = 500;

//----------------------------------------------------------------------------
DebugHud::DebugHud()
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
		"es/shaders/Line.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
		IMaterial::SHADER_DEFINES());
	
	mInputLayout = gFBEnv->pRenderer->GetInputLayout(
		DEFAULT_INPUTS::POSITION_COLOR, mLineShader);
	/*mHdrInputLayout = gFBEnv->pRenderer->GetInputLayout(
		DEFAULT_INPUTS::POSITION_HDR_COLOR, mLineShader);*/

	mVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(0, LINE_STRIDE, MAX_LINE_VERTEX, 
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);
	/*mHdrVertexBuffer = gFBEnv->pRenderer->CreateVertexBuffer(0, HDR_LINE_STRIDE, MAX_LINE_VERTEX,
		BUFFER_USAGE_DYNAMIC, BUFFER_CPU_ACCESS_WRITE);*/

	DEPTH_STENCIL_DESC ddesc;
	ddesc.DepthEnable = false;
	mRenderStates = FB_NEW(RenderStates);
	mRenderStates->CreateDepthStencilState(ddesc);
	
	
	gFBEnv->_pInternalRenderer->GetMainScene()->AddListener(this);
	
	mSphereMesh = gFBEnv->pEngine->GetMeshObject("es/objects/DebugSphere.dae");
	mBoxMesh = gFBEnv->pEngine->GetMeshObject("es/objects/DebugBox.dae");

	//mTriMaterial = IMaterial::CreateMaterial("es/material/DebugTriangle.material");
}

//----------------------------------------------------------------------------
DebugHud::~DebugHud()
{
	if (gFBEnv->_pInternalRenderer->GetMainScene())
		gFBEnv->_pInternalRenderer->GetMainScene()->RemoveListener(this);

	gFBEnv->pEngine->ReleaseMeshObject(mSphereMesh);
	gFBEnv->pEngine->ReleaseMeshObject(mBoxMesh);
}

//----------------------------------------------------------------------------
void DebugHud::DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color, float size)
{
	mTextsForDur.push_back(TextData(pos, text, color, size, secs));
	while (mTextsForDur.size() > 10)
	{
		mTextsForDur.pop_front();
	}
}

//----------------------------------------------------------------------------
void DebugHud::DrawText(const Vec2I& pos, WCHAR* text, 
	const Color& color, float size)
{
	mTexts.push(TextData(pos, text, color, size, 0.f));	
}

void DebugHud::Draw3DText(const Vec3& pos, WCHAR* text, const Color& color, float size)
{
	auto cam = gFBEnv->pRenderer->GetCamera();
	if (cam)
	{
		Vec2I spos = cam->WorldToScreen(pos);
		mTexts.push(TextData(spos, text, color, size, 0.f));
	}
}

//----------------------------------------------------------------------------
void DebugHud::DrawLine(const Vec3& start, const Vec3& end, 
	const Color& color0, const Color& color1)
{
	Line line;
	line.mStart = start;
	line.mColor = color0;

	line.mEnd = end;
	line.mColore = color1;
	
	mWorldLines.push_back(line);
}

void DebugHud::DrawLineBeforeAlphaPass(const Vec3& start, const Vec3& end, const Color& color0,
	const Color& color1)
{
	Line line;
	line.mStart = start;
	line.mColor = color0;

	line.mEnd = end;
	line.mColore = color1;

	mWorldLinesBeforeAlphaPass.push_back(line);
}

void DebugHud::DrawLine(const Vec2I& start, const Vec2I& end, 
	const Color& color0, const Color& color1)
{
	Line line;
	line.mStart = Vec3((float)start.x, (float)start.y, 0.f);
	line.mColor = color0;

	line.mEnd = Vec3((float)end.x, (float)end.y, 0.f);
	line.mColore = color1;
	
	mScreenLines.push_back(line);
}

void DebugHud::DrawSphere(const Vec3& pos, float radius, const Color& color)
{
	mSpheres.push_back(Sphere());
	auto& s = mSpheres.back();
	s.mPos = pos;
	s.mRadius = radius;
	s.mColor = color;
}
void DebugHud::DrawBox(const Vec3& boxMin, const Vec3& boxMax, const Color& color, float alpha)
{
	mBoxes.push_back(Box());
	auto& b = mBoxes.back();
	b.mPos = (boxMin + boxMax) * .5f;
	b.mExtent = boxMax - b.mPos;
	b.mColor = color;
	b.mAlpha = alpha;
}
void DebugHud::DrawTriangle(const Vec3& a, const Vec3& b, const Vec3& c, const Color& color, float alpha)
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
void DebugHud::PreRender()
{
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;

	if (mLastPreRendered == gFBEnv->mFrameCounter)
		return;
	mLastPreRendered = gFBEnv->mFrameCounter;

	
}

void DebugHud::OnBeforeRenderingTransparents(IScene* scene)
{
	if (!gFBEnv->pConsole->GetEngineCommand()->r_debugDraw)
		return;
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;
	unsigned lineCount = mWorldLinesBeforeAlphaPass.size();
	if (lineCount == 0)
		return;

	D3DEventMarker mark("DebugHud::OnBeforeRenderingTransparents()");
	PreRender();

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();
	// object constant buffer
	mInputLayout->Bind();
	mLineShader->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
	pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);	
	if (lineCount > 0)
	{
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
void DebugHud::Render()
{
	if (!gFBEnv->pConsole->GetEngineCommand()->r_debugDraw)
		return;
	if (gFBEnv->mRenderPass != RENDER_PASS::PASS_NORMAL)
		return;

	mObjectConstants_WorldLine.gWorldViewProj =
		gFBEnv->pRenderer->GetCamera()->GetViewProjMat();

	//Profiler profile("void DebugHud::Render()");
	D3DEventMarker mark("DebugHud::Render()");
	//PreRender();

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();
	// object constant buffer
	mRenderStates->Bind();
	mInputLayout->Bind();
	mLineShader->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
	pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
	unsigned lineCount = mWorldLines.size();
	if (lineCount > 0)
	{
		unsigned processedLines = 0;
		while(lineCount > processedLines)
		{			
			MapData mapped = mVertexBuffer->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
			if (mapped.pData)
			{
				unsigned totalVertexCount = lineCount * 2;		
				unsigned numVertex = 0;
				for (numVertex = 0; processedLines < lineCount && numVertex < MAX_LINE_VERTEX; numVertex+=2)
				{
					unsigned base = numVertex*LINE_STRIDE;
					memcpy((char*)mapped.pData + base, &mWorldLines[processedLines], LINE_STRIDE*2);
					processedLines++;
				}
				mVertexBuffer->Unmap();
				mVertexBuffer->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mWorldLines.clear();
	}

	const auto& size = gFBEnv->_pInternalRenderer->GetMainRTSize();
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0,
		(float)size.x,
		(float)size.y,
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();
	pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants);
	lineCount = mScreenLines.size();
	if (lineCount > 0)
	{
		while(lineCount)
		{			
			MapData mapped = mVertexBuffer->Map(MAP_TYPE_WRITE_DISCARD, 0, MAP_FLAG_NONE);
			if (mapped.pData)
			{
				unsigned totalVertexCount = lineCount * 2;		
				unsigned numVertex = 0;
				for (numVertex = 0; numVertex < totalVertexCount && numVertex < MAX_LINE_VERTEX; numVertex+=2)
				{
					unsigned base = numVertex*LINE_STRIDE;
					memcpy((char*)mapped.pData + base, &mScreenLines[numVertex/2], LINE_STRIDE*2);
					lineCount--;
				}
				mVertexBuffer->Unmap();
				mVertexBuffer->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mScreenLines.clear();
	}

	IFont* pFont = gFBEnv->pRenderer->GetFont();
	pFont->PrepareRenderResources();
	pFont->SetRenderStates(false, false);
	while (!mTexts.empty())
	{
		const TextData& textData = mTexts.front();
		pFont->SetHeight(textData.mSize);
		pFont->Write((float)textData.mPos.x, (float)textData.mPos.y, 0.5f, textData.mColor.Get4Byte(), 
			(const char*)textData.mText.c_str(), -1, FONT_ALIGN_LEFT);
		mTexts.pop();
	}

	// render text for duration
	{
		pFont->SetRenderStates();
		auto it = mTextsForDur.begin();
		int count = 0;

		for (; it != mTextsForDur.end(); ++count)
		{
			it->mSecs -= gFBEnv->pTimer->GetDeltaTime();
			if (it->mSecs<=0.f)
			{
				it = mTextsForDur.erase(it);
				continue;
			}
			else
			{
				Vec2I offset(0, 0);
				offset.y += (int)pFont->GetHeight() * count;

				Vec2I drawPos = it->mPos + offset;
				pFont->SetHeight(it->mSize);
				Color color = it->mColor;
				float proportion = 1.0f - (it->mSecs / it->mDuration);
				color.a() = 1.0f - (proportion*proportion);
				pFont->Write((float)drawPos.x, (float)drawPos.y, 0.5f, color.Get4Byte(),
					(const char*)it->mText.c_str(), -1, FONT_ALIGN_LEFT);


				it++;
			}
		}
		pFont->SetBackToOrigHeight();
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
			//mObjectConstants.gWorldView = gFBEnv->pRenderer->GetCamera()->GetViewMat() * mObjectConstants.gWorld;
			//mObjectConstants.gWorldViewProj = gFBEnv->pRenderer->GetCamera()->GetProjMat() * mObjectConstants.gWorldView;
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
void DebugHud::PostRender()
{
}