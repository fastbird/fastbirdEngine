#include <Engine/StdAfx.h>
#include <Engine/RenderObjects/TerrainPatch.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/IInputLayout.h>
#include <CommonLib/Math/Vec3.h>
#include <Engine/RenderObjects/Terrain.h>
#include <Engine/GlobalEnv.h>
#include <Engine/ICamera.h>
#include <FreeImage.h>

using namespace fastbird;

TerrainPatch::TerrainPatch()
{
	mLOD = 0;
	mUpPatch = 0;
	mLeftPatch = 0;
	mRightPatch = 0;
	mDownPatch = 0;
	mLastUpdatedFrame = 0;
	SetDepthStencilState(DEPTH_STENCIL_DESC());
}

TerrainPatch::~TerrainPatch()
{

}

//----------------------------------------------------------------------------
// IObject
//----------------------------------------------------------------------------
void TerrainPatch::PreRender()
{
	ICamera* pCamera = gFBEnv->pEngine->GetRenderer()->GetCamera();
	float sqDist =  (pCamera->GetPos() - mBoundingVolume->GetCenter()).LengthSQ();
	if (sqDist < 10000.0f) // 100m
	{
		mLOD = 0;
	}
	else if (sqDist < 22500.0f) // 150m
	{
		mLOD = 1;
	}
	else if (sqDist < 40000.0f) // 200m
	{
		mLOD = 2;
	}
	else if (sqDist < 62500.0f) // 250m
	{
		mLOD = 3;
	}
	else
	{
		mLOD = 4;
	}

	mLastUpdatedFrame = gFBEnv->mFrameCounter;
}

//----------------------------------------------------------------------------
void TerrainPatch::Render()
{
	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();

	int lodFlag=0;
	if (mUpPatch)
	{
		if (mUpPatch->GetLOD() < mLOD)
			lodFlag |= Terrain::LOD_DIFF_FLAG_UP;
	}
	if (mLeftPatch)
	{
		if (mLeftPatch->GetLOD() < mLOD)
			lodFlag |= Terrain::LOD_DIFF_FLAG_LEFT;
	}
	if (mRightPatch)
	{
		if (mRightPatch->GetLOD() < mLOD)
			lodFlag |= Terrain::LOD_DIFF_FLAG_RIGHT;
	}
	if (mDownPatch)
	{
		if (mDownPatch->GetLOD() < mLOD)
			lodFlag |= Terrain::LOD_DIFF_FLAG_DOWN;
	}


	mIndexBuffer = Terrain::sTerrain->GetIndexBufferLOD(mLOD, lodFlag);
	mIndexBuffer->Bind();
	mMaterial->Bind(true);
	pRenderer->SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	BindRenderStates();
	const unsigned int numBuffers = 2;
	IVertexBuffer* buffers[numBuffers] = {mVertexBuffer, mVertexBuffer2};
	unsigned int strides[numBuffers] = {mVertexBuffer->GetStride(), mVertexBuffer2->GetStride()};
	unsigned int offsets[numBuffers] = {0, 0};
	pRenderer->SetVertexBuffer(0, numBuffers, buffers, strides, offsets);

	pRenderer->DrawIndexed(mIndexBuffer->GetNumIndices(), 0, 0);
}

//----------------------------------------------------------------------------
void TerrainPatch::PostRender()
{

}

//----------------------------------------------------------------------------
// OWN
//----------------------------------------------------------------------------
void TerrainPatch::Build(int idx, int idy, int numVertX, float distance, 
	FIBITMAP* pImage, Terrain* pTerrain)
{
	//srand(GetTickCount());
	std::vector<Vec3> positions;
	positions.reserve(Terrain::PATCH_NUM_VERT*Terrain::PATCH_NUM_VERT);
	struct SecondBuffer
	{
		Vec3 normal;
		//Vec3 tangent;
		//Vec3 binormal;
	};
	std::vector<SecondBuffer> bufferData(Terrain::PATCH_NUM_VERT*Terrain::PATCH_NUM_VERT, SecondBuffer());

	int starty = (Terrain::PATCH_NUM_VERT-1) * idy;
	int startx = (Terrain::PATCH_NUM_VERT-1) * idx;
	unsigned width = FreeImage_GetWidth(pImage);
	unsigned height = FreeImage_GetHeight(pImage);
	unsigned pitch = FreeImage_GetPitch(pImage);
	BYTE* pBits = FreeImage_GetBits(pImage);
	int count=0;
	for (int y=0; y<Terrain::PATCH_NUM_VERT; y++)
	{
		float depth = (y+starty) * distance;
		for (int x=0; x<Terrain::PATCH_NUM_VERT; x++)
		{
			float height = 0.f;
			if (pBits)
			{
				BYTE* pPixel = pBits + pitch*(starty+y) + (startx+x);
				height = Terrain::ConvertHeight((int)*pPixel);
			}
			positions.push_back(Vec3((x+startx) * distance, depth, height));
			pTerrain->GetNormal(x+startx, y+starty, bufferData[count].normal);
			count++;
		}
	}
	mVertexBuffer = gFBEnv->pEngine->GetRenderer()->CreateVertexBuffer(
		(void*)&positions[0], sizeof(Vec3), positions.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);

	mBoundingVolume->ComputeFromData(&positions[0], positions.size());
	mBoundingVolumeWorld->SetCenter(mBoundingVolume->GetCenter());
	mBoundingVolumeWorld->SetRadius(mBoundingVolume->GetRadius());

	// build second buffer
	// normal
	// tangent
	/*
	int i = 0;
	for (int y=0; y<Terrain::PATCH_NUM_VERT-1; y++) // except last position
	{
		const int depth = y * Terrain::PATCH_NUM_VERT;
		for (int x=0; x<Terrain::PATCH_NUM_VERT-1; x++) // except last position
		{
			const int curIndex = depth+x;
			const int xNextIndex = depth+x+1;
			const int yNextIndex = depth + Terrain::PATCH_NUM_VERT + x;
			Vec3 v1 = positions[xNextIndex] - positions[curIndex];
			Vec3 v2 = positions[yNextIndex] - positions[curIndex];
			bufferData[curIndex].normal = v1.Cross(v2);
			bufferData[curIndex].normal.Normalize();
			bufferData[curIndex].tangent = Vec3(0.f, 1.f, 0.f);
			bufferData[curIndex].binormal = bufferData[curIndex].tangent.Cross(bufferData[curIndex].normal);
			bufferData[curIndex].binormal.Normalize();
			bufferData[curIndex].tangent = bufferData[curIndex].normal.Cross(bufferData[curIndex].binormal);

		}
	}
	// last x positions
	for (int y= 0; y<Terrain::PATCH_NUM_VERT-1; y++)
	{
		const int x = Terrain::PATCH_NUM_VERT-1;
		const int depth = y * Terrain::PATCH_NUM_VERT;
		const int curIndex = depth+x;
		const int xPrevIndex = depth + x - 1;
		const int yPrevIndex = depth + Terrain::PATCH_NUM_VERT + x;
		Vec3 v1 = positions[xPrevIndex] - positions[curIndex];
		Vec3 v2 = positions[yPrevIndex] - positions[curIndex];
		bufferData[curIndex].normal = v2.Cross(v1);
		bufferData[curIndex].normal.Normalize();
		bufferData[curIndex].tangent = Vec3(0.f, 1.f, 0.f);
		bufferData[curIndex].binormal = bufferData[curIndex].tangent.Cross(bufferData[curIndex].normal);
		bufferData[curIndex].binormal.Normalize();
		bufferData[curIndex].tangent = bufferData[curIndex].normal.Cross(bufferData[curIndex].binormal);
	}

	// last y positions
	for (int x= 0; x<Terrain::PATCH_NUM_VERT-1; x++)
	{
		const int y = Terrain::PATCH_NUM_VERT-1;
		const int depth = y * Terrain::PATCH_NUM_VERT;
		const int curIndex = depth+x;
		const int xPrevIndex = depth + x + 1;
		const int yPrevIndex = depth - Terrain::PATCH_NUM_VERT + x;
		Vec3 v1 = positions[xPrevIndex] - positions[curIndex];
		Vec3 v2 = positions[yPrevIndex] - positions[curIndex];
		bufferData[curIndex].normal = v2.Cross(v1);
		bufferData[curIndex].normal.Normalize();
		bufferData[curIndex].tangent = Vec3(0.f, 1.f, 0.f);
		bufferData[curIndex].binormal = bufferData[curIndex].tangent.Cross(bufferData[curIndex].normal);
		bufferData[curIndex].binormal.Normalize();
		bufferData[curIndex].tangent = bufferData[curIndex].normal.Cross(bufferData[curIndex].binormal);
	}
	// last corner
	{
		const int y = Terrain::PATCH_NUM_VERT-1;
		const int x = Terrain::PATCH_NUM_VERT-1;
		const int curIndex = Terrain::PATCH_NUM_VERT * y + x;
		const int xPrevIndex = Terrain::PATCH_NUM_VERT * y + x-1;
		const int yPrevIndex = Terrain::PATCH_NUM_VERT * (y-1) + x;
		Vec3 v1 = positions[xPrevIndex] - positions[curIndex];
		Vec3 v2 = positions[yPrevIndex] - positions[curIndex];
		bufferData[curIndex].normal = v1.Cross(v2);
		bufferData[curIndex].normal.Normalize();
		bufferData[curIndex].tangent = Vec3(0.f, 1.f, 0.f);
		bufferData[curIndex].binormal = bufferData[curIndex].tangent.Cross(bufferData[curIndex].normal);
		bufferData[curIndex].binormal.Normalize();
		bufferData[curIndex].tangent = bufferData[curIndex].normal.Cross(bufferData[curIndex].binormal);
	}*/
	mVertexBuffer2 = gFBEnv->pEngine->GetRenderer()->CreateVertexBuffer(
		(void*)&bufferData[0], sizeof(SecondBuffer), bufferData.size(), BUFFER_USAGE_IMMUTABLE, BUFFER_CPU_ACCESS_NONE);
}

//----------------------------------------------------------------------------
void TerrainPatch::SetUpPatch(TerrainPatch* pPatch)
{
	assert(pPatch!=this);
	mUpPatch = pPatch;
}

//----------------------------------------------------------------------------
void TerrainPatch::SetRightPatch(TerrainPatch* pPatch)
{
	assert(pPatch!=this);
	mRightPatch = pPatch;
}

//----------------------------------------------------------------------------
void TerrainPatch::SetDownPatch(TerrainPatch* pPatch)
{
	assert(pPatch!=this);
	mDownPatch = pPatch;
}

//----------------------------------------------------------------------------
void TerrainPatch::SetLeftPatch(TerrainPatch* pPatch)
{
	assert(pPatch!=this);
	mLeftPatch = pPatch;
}