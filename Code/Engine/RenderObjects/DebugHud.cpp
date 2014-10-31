#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/DebugHud.h>
#include <Engine/ICamera.h>

using namespace fastbird;

// 12 : float3, 4 : ubyte4
const unsigned DebugHud::LINE_STRIDE = 16;
// 12 : float3, 16 : color
//const unsigned DebugHud::HDR_LINE_STRIDE = 28;
const unsigned DebugHud::MAX_LINE_VERTEX = 500;

//----------------------------------------------------------------------------
DebugHud::DebugHud()
{
	mObjectConstants.gWorldViewProj = MakeOrthogonalMatrix(0, 0, 
		(float)gFBEnv->pEngine->GetRenderer()->GetWidth(),
		(float)gFBEnv->pEngine->GetRenderer()->GetHeight(),
		0.f, 1.0f);
	mObjectConstants.gWorld.MakeIdentity();

	mObjectConstants_WorldLine.gWorld.MakeIdentity();
	mObjectConstants_WorldLine.gWorldViewProj.MakeIdentity();

	mLineShader = gFBEnv->pEngine->GetRenderer()->CreateShader(
		"Code/Engine/Renderer/Shaders/Line.hlsl", BINDING_SHADER_VS | BINDING_SHADER_PS,
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
	SetDepthStencilState(ddesc);
	gFBEnv->pEngine->GetScene()->AddListener(this);
}

//----------------------------------------------------------------------------
DebugHud::~DebugHud()
{
	if (gFBEnv->pEngine->GetScene())
		gFBEnv->pEngine->GetScene()->RemoveListener(this);
}

//----------------------------------------------------------------------------
void DebugHud::DrawTextForDuration(float secs, const Vec2I& pos, WCHAR* text, 
		const Color& color)
{
	mTextsForDur.push_back(TextData(pos, text, color, secs));
	while (mTextsForDur.size() > 4)
	{
		mTextsForDur.pop_front();
	}
}

//----------------------------------------------------------------------------
void DebugHud::DrawText(const Vec2I& pos, WCHAR* text, 
	const Color& color)
{
	mTexts.push(TextData(pos, text, color, 0.f));	
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
	line.mColor = color1;
	
	mScreenLines.push_back(line);
}

//----------------------------------------------------------------------------
void DebugHud::PreRender()
{
	mObjectConstants_WorldLine.gWorldViewProj = 
		gFBEnv->pRenderer->GetCamera()->GetViewProjMat();
}

void DebugHud::OnBeforeRenderingTransparents()
{
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
	D3DEventMarker mark("DebugHud::Render()");
	PreRender();

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();
	// object constant buffer
	BindRenderStates();
	mInputLayout->Bind();
	mLineShader->Bind();
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_LINELIST);
	pRenderer->UpdateObjectConstantsBuffer(&mObjectConstants_WorldLine);
	unsigned lineCount = mWorldLines.size();
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
					memcpy((char*)mapped.pData + base, &mWorldLines[numVertex/2], LINE_STRIDE*2);
					lineCount--;
				}
				mVertexBuffer->Unmap();
				mVertexBuffer->Bind();
				pRenderer->Draw(numVertex, 0);
			}
		}
		mWorldLines.clear();
	}

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
				pFont->Write((float)drawPos.x, (float)drawPos.y, 0.5f, it->mColor.Get4Byte(),
					(const char*)it->mText.c_str(), -1, FONT_ALIGN_LEFT);
				

				it++;
			}

			
		}
	}
	PostRender();
}
//----------------------------------------------------------------------------
void DebugHud::PostRender()
{
}