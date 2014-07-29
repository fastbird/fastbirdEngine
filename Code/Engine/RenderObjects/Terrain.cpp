#include "Engine/StdAfx.h"
#include <Engine/RenderObjects/Terrain.h>
#include <Engine/RenderObjects/TerrainPatch.h>
#include <Engine/IVertexBuffer.h>
#include <Engine/IIndexBuffer.h>
#include <Engine/IInputLayout.h>
#include <Engine/GlobalEnv.h>
#include <Engine/IScene.h>
#include <Engine/Renderer/TextureUtil.h>
#include <CommonLib/Profiler.h>
#include <CommonLib/StringUtils.h>
#include <Engine/Foundation/Keyboard.h>
#include <CommonLib/Unicode.h>

#include <CommonLib/Math/fbMath.h>
#include <CommonLib/Math/Vec4.h>

using namespace fastbird;

ITerrain* Terrain::sTerrain = 0;

ITerrain* ITerrain::CreateTerrainInstance(int numVertX, int numVertY, float distance, const char* heightmapFile)
{
	Terrain* pTerrain = new Terrain;
	pTerrain->Init(numVertX, numVertY, distance, heightmapFile);
	return pTerrain;
}

const int Terrain::PATCH_NUM_VERT = 17;

	

//----------------------------------------------------------------------------
Terrain::Terrain()
	: mInputListenerEnabled(false)
{
	sTerrain = this;
	mNumVertX = 0;
	mNumVertY = 0;
	mDistance = 0;
	mDebugging = false;
	mDebuggingLOD = 0;
	mDebuggingFlagLOD = 0;
	mDebuggingFlag = LOD_DIFF_FLAG_NONE;
	mDebuggingFlagIndex = 0;
	gFBEnv->pEngine->AddInputListener(this, IInputListener::INPUT_LISTEN_PRIORITY_UI, 0);
	mSelectedBar = 0;

	mGrassColor[0] = Color(0.45168f, 0.6258f, 0.1176f, 1.f);
	mGrassColor[1] = Color(0.8431f, 0.8392f, 0.4352f, 1.0f);
	mGrassLerp = 0.f;
}

//----------------------------------------------------------------------------
Terrain::~Terrain()
{
}

//----------------------------------------------------------------------------
#undef max
void Terrain::Init(int numVertX, int numVertY, float distance, 
	const char* heightmapFile)
{
	Profiler profiler("'Terrain Init'");
	IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Initializing Terrain...");
	// Num Patches
	assert(IsPowerOfTwo(numVertX-1));
	assert(IsPowerOfTwo(numVertY-1));
	mDistance = distance;
	mNumVertX = numVertX;
	mNumVertY = numVertY;

	// Create Patches
	{
		Profiler profiler("'Create Patches'");
		FIBITMAP* pImage = LoadImageFile(heightmapFile, PNG_DEFAULT);
		GenerateNormals(pImage, heightmapFile);

		mNumPatches = Vec2I(
			(numVertX-1) / (PATCH_NUM_VERT-1), (numVertY-1) / (PATCH_NUM_VERT-1));
		mPatches.reserve(mNumPatches.x*mNumPatches.y);
		for (int idy=0; idy<mNumPatches.y; idy++)
		{
			for (int idx=0; idx<mNumPatches.x; idx++)
			{
				TerrainPatch* pNewPatch = new TerrainPatch();
				mPatches.push_back(pNewPatch);
				pNewPatch->Build(idx, idy, numVertX, distance, pImage, this);
				gFBEnv->pEngine->GetScene()->AttachObject(pNewPatch);
			}
		}
		int patchIndex = 0;
		for (int idy=0; idy<mNumPatches.y; idy++)
		{
			for (int idx=0; idx<mNumPatches.x; idx++)
			{
				int left = idx - 1;
				if (left>=0)
				{
					mPatches[patchIndex]->SetLeftPatch(mPatches[idy * mNumPatches.x + left]);
				}
				int right = idx + 1;
				if (right < mNumPatches.x)
				{
					mPatches[patchIndex]->SetRightPatch(mPatches[idy * mNumPatches.x + right]);
				}
				int up = idy + 1;
				if (up < mNumPatches.y)
				{
					mPatches[patchIndex]->SetUpPatch(mPatches[up * mNumPatches.x + idx]);
				}
				int down = idy-1;
				if (down >= 0)
				{
					mPatches[patchIndex]->SetDownPatch(mPatches[down * mNumPatches.x + idx]);
				}
			
				patchIndex++;
			}
		}

		FreeImage_Unload(pImage);
	}

	// CreateIndexBuffer
	for (int lod=0; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			if (toTheRight)
			{
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					indices.push_back(lowi + col);
					indices.push_back(highi + col);
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					indices.push_back(highi + col);
					indices.push_back(lowi + col);
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}		
		mIndexBuffers[lod] = gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}

	CreateRefinedIndexBuffers();
		

	// Init Material
	mMaterial = fastbird::IMaterial::CreateMaterial("data/materials/terrain.material");
	assert(mMaterial);

	IRenderer* pRenderer = gFBEnv->pEngine->GetRenderer();

	size_t nP = mPatches.size();
	for (size_t i=0; i<nP; i++)
	{
		mPatches[i]->SetMaterial(mMaterial);
		mPatches[i]->SetIndexBuffer(mIndexBuffers[0]);
	}

	IEngine::Log(FB_DEFAULT_DEBUG_ARG, "Terrain Initialized.");
}

//----------------------------------------------------------------------------
void Terrain::GenerateNormals(FIBITMAP* pImage, const char* filename)
{
	Profiler profiler("'Terrain GenerateNormals'");
	bool cacheHit = false;
	char cacheName[MAX_PATH];
	strcpy_s(cacheName, MAX_PATH, filename);
	StripExtension(cacheName);
	strcat_s(cacheName, MAX_PATH, ".fbnormal");
	// Check cache
	HANDLE imageHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (imageHandle==INVALID_HANDLE_VALUE)
	{
		FB_LOG("Failed to check height map image file time stamp(CreateFile)!");
		FB_LOG_LAST_ERROR();
	}
	else
	{
		FILETIME imageTime;
		BOOL result = GetFileTime(imageHandle, 0, 0, &imageTime);
		if (!result)
		{
			FB_LOG("Failed to get image file time!");
			FB_LOG_LAST_ERROR();
		}
		else
		{			
			HANDLE cacheHandle = CreateFile(cacheName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (cacheHandle==INVALID_HANDLE_VALUE)
			{
				FB_LOG("Normal cache does not exist.");
			}
			else
			{
				FILETIME cacheTime;
				result = GetFileTime(cacheHandle, 0, 0, &cacheTime);
				if (!result)
				{
					FB_LOG("Failed to get normal cache time!");
					FB_LOG_LAST_ERROR();
				}
				else
				{
					// if first file time is earlier than second file time.
					if (CompareFileTime(&imageTime, &cacheTime)==-1)
					{
						cacheHit = true;
					}
				}
			}
			CloseHandle(cacheHandle);
		}			
	}
	CloseHandle(imageHandle);

	if (cacheHit)
	{
		FILETIME lastWriteTime, currentTime;
		unsigned width, height;
		std::ifstream file(cacheName, std::ifstream::binary);
		DWORD version=0;
		file.read((char*)&version, sizeof(DWORD));
		file.read((char*)&lastWriteTime, sizeof(FILETIME));
		file.read((char*)&currentTime, sizeof(FILETIME));
		file.read((char*)&width, sizeof(unsigned int));
		file.read((char*)&height, sizeof(unsigned int));
		unsigned int numNormals;
		file.read((char*)&numNormals, sizeof(unsigned int));
		mNormals.resize(numNormals);
		for (unsigned i=0; i<numNormals; i++)
		{
			file.read((char*)&mNormals[i], sizeof(Vec3));
		}
	}
	else
	{
		// Cache miss!
		assert(mDistance != 0);
		assert(mNumVertX != 0);
		assert(mNumVertY != 0);
		assert(pImage != 0);
		unsigned width = FreeImage_GetWidth(pImage);
		unsigned height = FreeImage_GetHeight(pImage);
		unsigned pitch = FreeImage_GetPitch(pImage);
		BYTE* pBits = FreeImage_GetBits(pImage);
		assert(width == mNumVertX);
		assert(height == mNumVertY);
		mNormals.reserve(mNumVertX*mNumVertY);
		//mTangents.reserve(mNumVertX*mNumVertY);
		//mBinormals.reserve(mNumVertX*mNumVertY);

		for (int y=0; y<mNumVertY; y++)
		{
			for (int x=0; x<mNumVertX; x++)
			{
				if (x==0 || y==0 || x==mNumVertX-1 || y==mNumVertY-1)
				{
					mNormals.push_back(Vec3(0.f, 0.f, 1.f));
				}
				else
				{
					BYTE* pPixel = pBits + pitch*y + x;
					float height = Terrain::ConvertHeight((int)*pPixel);
					Vec3 currentPos(x*mDistance, y*mDistance, height);
					height = Terrain::ConvertHeight((int)*(pPixel+1));
					Vec3 nextPos((x+1)*mDistance, y*mDistance, height);
					height = Terrain::ConvertHeight((int)*(pPixel-1));
					Vec3 prevPos((x-1)*mDistance, y*mDistance, height);
				
					pPixel = pBits + pitch*(y+1) + x;
					height = Terrain::ConvertHeight((int)*pPixel);
					Vec3 upPos(x*mDistance, (y+1)*mDistance, height);
					pPixel = pBits + pitch*(y-1) + x;
					height = Terrain::ConvertHeight((int)*pPixel);
					Vec3 downPos(x*mDistance, (y-1)*mDistance, height);

					Vec3 toNext = nextPos - currentPos;
					Vec3 toPrev = prevPos - currentPos;
					Vec3 toUp = upPos - currentPos;
					Vec3 toDown = downPos - currentPos;

					Vec3 normal = toNext.Cross(toUp);
					normal.Normalize();

					Vec3 temp = toUp.Cross(toPrev);
					temp.Normalize();
					normal += temp;

					temp = toPrev.Cross(toDown);
					temp.Normalize();
					normal += temp;

					temp = toDown.Cross(toNext);
					temp.Normalize();
					normal += temp;
					normal.Normalize();

					mNormals.push_back(normal);

				}
			
			}
		}

		// save normal cache
		// last write time of the heightmap image
		HANDLE imageHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (imageHandle==INVALID_HANDLE_VALUE)
		{
			FB_LOG("Failed to check height map image file time stamp(CreateFile)!");
			FB_LOG_LAST_ERROR();
		}
		else
		{
			FILETIME lastWriteTime;
			BOOL result = GetFileTime(imageHandle, 0, 0, &lastWriteTime);
			if (!result)
			{
				FB_LOG("Failed to check height map image file time stamp(GetFileTime)!");
				FB_LOG_LAST_ERROR();
			}
			else
			{
				FILETIME currentTime;
				GetSystemTimeAsFileTime(&currentTime);
				char cacheName[MAX_PATH];
				strcpy_s(cacheName, MAX_PATH, filename);
				StripExtension(cacheName);
				strcat_s(cacheName, MAX_PATH, ".fbnormal");
				std::ofstream file(cacheName, std::ofstream::binary);
			
				//----------------------------------------------------------------
				// .fbnormal file format
				// DWORD version
				// FILETIME last modification date of the image file.
				// FILETIME currentTime
				// uint width
				// uint height
				// uint numNormals
				// Vec3s normals
				file.write((const char*)&NORMAL_CACHE_VERSION, sizeof(DWORD));
				file.write((const char*)&lastWriteTime, sizeof(FILETIME));
				file.write((const char*)&currentTime, sizeof(FILETIME));
				file.write((const char*)&width, sizeof(unsigned int));
				file.write((const char*)&height, sizeof(unsigned int));
				unsigned int numNormals = mNormals.size();
				file.write((const char*)&numNormals, sizeof(unsigned int));
				for (unsigned i=0; i<numNormals; i++)
				{
					file.write((const char*)&mNormals[i], sizeof(Vec3));
				}
			}
		}
		CloseHandle(imageHandle);
	}
}

//----------------------------------------------------------------------------
IIndexBuffer* Terrain::GetIndexBufferLOD(int lod, int diffFlag) const
{
	if (mDebugging)
	{
		if (mDebuggingFlagIndex==0)
		{
			return mIndexBuffers[mDebuggingLOD];
		}
		else
		{
			return mIndexBuffersRefined[mDebuggingFlagLOD][ConvertDiffFlag(mDebuggingFlag)];
		}
	}
	assert(lod>=0 && lod<5);
	if (diffFlag==LOD_DIFF_FLAG_NONE)
	{
		return mIndexBuffers[lod];
	}
	else
	{
		assert(lod-1<4);
		unsigned idx = ConvertDiffFlag((Terrain::LOD_DIFF_FLAG)diffFlag);
		return mIndexBuffersRefined[lod-1][idx];
	}
}

void Terrain::Update()
{
	int x = 2;
	int y=400;
	int yStep = 16;
	wchar_t msg[255];
	ColorRamp& cr = mMaterial->GetColorRamp(2, BINDING_SHADER_PS);
	unsigned numBars = cr.NumBars();
	for (unsigned i=0; i<numBars; i++)
	{
		Vec3 color(1, 1, 1);
		if (i==mSelectedBar)
		{
			color = Vec3(1, 1, 0);
		}
		swprintf_s(msg, 255, L"Bar0 Pos = %.2f", cr.GetBar(i).position); 
		gFBEnv->pRenderer->DrawText(Vec2I(x, y), msg, color);
		y+=yStep;
	}
}

//----------------------------------------------------------------------------
void Terrain::CreateRefinedIndexBuffers()
{
	Profiler profiler("'Terrain CreateRefinedIndexBuffers'");
	//lod1
	// C == Combination
	// 4C1 == 4
	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT
#pragma region LOD_DIFF_FLAG_LEFT
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col==0)
					{
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(highi);
						indices.push_back(highi + colStep);
						indices.push_back(highi + colStep);
						//indices.push_back(middlei+colStep);
						//indices.push_back(middlei+colStep);				
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT * rowStep);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==0)
					{
						indices.push_back(highi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(highi);
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);						
					}			
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_UP
#pragma region LOD_DIFF_FLAG_UP
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			if (lod==4)
			{
				indices.push_back(highi+colStep);
				indices.push_back(lowi+colStep);
				indices.push_back(highi + colStep/2);
				indices.push_back(highi);
				indices.push_back(highi);
				indices.push_back(lowi + colStep);
				indices.push_back(lowi);
				break;
			}
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					indices.push_back(lowi + col);
					indices.push_back(highi + col);
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT * rowStep);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (row+rowStep == PATCH_NUM_VERT-1)
					{
						if (col!=0)
						{
							indices.push_back(highi+col);
							indices.push_back(lowi+col);
							indices.push_back(highi + col - colStep/2);
							indices.push_back(highi + col - colStep);
							indices.push_back(highi + col - colStep);
							indices.push_back(lowi + col);
							indices.push_back(lowi + col - colStep);
							indices.push_back(lowi + col - colStep);
						}
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_UP)] = gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_UP

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_RIGHT
#pragma region LOD_DIFF_FLAG_RIGHT
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col == PATCH_NUM_VERT-1)
					{
						indices.push_back(lowi+col); // 4
						indices.push_back(middlei+col); // 5
						indices.push_back(middlei+col); // 5
						indices.push_back(highi+col - colStep); // 3
						indices.push_back(highi+col); // 6
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 6
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 10
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 10
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==PATCH_NUM_VERT-1)
					{
						indices.push_back(highi + col); // 9
						indices.push_back(middlei + col); // 10
						indices.push_back(highi + col - colStep); // 8
						indices.push_back(lowi + col); // 6
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_RIGHT)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_RIGHT

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_DOWN
#pragma region LOD_DIFF_FLAG_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			if (toTheRight)
			{
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (row==0)
					{
						if (col == PATCH_NUM_VERT-1)
						{
							continue;
						}
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
						indices.push_back(lowi + col + colStep/2);
						indices.push_back(lowi + col + colStep);
						indices.push_back(lowi + col + colStep);
						indices.push_back(highi + col);
						indices.push_back(highi + col + colStep);
						indices.push_back(highi + col + colStep);
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT * rowStep);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					indices.push_back(highi + col);
					indices.push_back(lowi + col);
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}		
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_DOWN

	// 4C2 = 6
	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_UP
#pragma region LOD_DIFF_FLAG_LEFT_UP
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (lod==4)
			{
				indices.push_back(highi+colStep);
				indices.push_back(lowi+colStep);
				indices.push_back(highi+colStep/2); // 18
				indices.push_back(highi); // 17
				indices.push_back(highi); // 17
				indices.push_back(lowi+colStep); // 12
				indices.push_back(middlei); // 16
				indices.push_back(lowi); // 11
				break;
			}
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col==0)
					{
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(highi);
						indices.push_back(highi + colStep);
						indices.push_back(highi + colStep);
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT * rowStep);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (row+rowStep == PATCH_NUM_VERT-1) // last row
					{
						if (col==0)
						{
							indices.push_back(highi+colStep/2); // 18
							indices.push_back(highi); // 17
							indices.push_back(highi); // 17
							indices.push_back(lowi+colStep); // 12
							indices.push_back(middlei); // 16
							indices.push_back(lowi); // 11
							indices.push_back(lowi); // 11
						}
						else
						{
							if (col>colStep)
							{
								indices.push_back(highi+col);
								indices.push_back(lowi+col);
								indices.push_back(highi + col - colStep/2);
								indices.push_back(highi + col - colStep);
								indices.push_back(highi + col - colStep);
								indices.push_back(lowi + col);
								indices.push_back(lowi + col - colStep);
								indices.push_back(lowi + col - colStep);
							}
							else
							{
								indices.push_back(highi+col);
								indices.push_back(lowi+col);
							}
						}						
					}
					else
					{
						if (col==0)
						{
							indices.push_back(highi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(lowi);		
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(middlei);
					indices.push_back(middlei);
					indices.push_back(highi);	
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_UP)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_UP

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_RIGHT
#pragma region LOD_DIFF_FLAG_LEFT_RIGHT
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col==0)
					{
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(highi);
						indices.push_back(highi + colStep);
						indices.push_back(highi + colStep);
						indices.push_back(middlei+colStep);
						indices.push_back(middlei+colStep);						
					}
					else if (col == PATCH_NUM_VERT-1)
					{
						indices.push_back(lowi+col); // 4
						indices.push_back(middlei+col); // 5
						indices.push_back(middlei+col); // 5
						indices.push_back(highi+col - colStep); // 3
						indices.push_back(highi+col); // 6
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 6
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 10
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 10
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==0)
					{
						indices.push_back(highi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(highi);
					}
					else if (col==PATCH_NUM_VERT-1)
					{
						indices.push_back(highi + col); // 9
						indices.push_back(middlei + col); // 10
						indices.push_back(highi + col - colStep); // 8
						indices.push_back(lowi + col); // 6
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);						
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_RIGHT)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_RIGHT

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_DOWN
#pragma region LOD_DIFF_FLAG_LEFT_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (row==0 && col==0)
					{
						indices.push_back(highi); // 4
						indices.push_back(middlei + colStep/2); // 6
						indices.push_back(middlei); // 3
						indices.push_back(lowi); // 0
						indices.push_back(lowi); // 0
						indices.push_back(middlei + colStep/2); // 6
						indices.push_back(lowi + colStep/2); // 1
						indices.push_back(lowi + colStep); //2
						indices.push_back(lowi + colStep); //2
						indices.push_back(middlei + colStep/2); // 6
						indices.push_back(highi + colStep); // 5
						indices.push_back(highi); // 4
						indices.push_back(highi); // 4
						indices.push_back(middlei + colStep/2); // 6
						indices.push_back(middlei + colStep/2); // 6
						indices.push_back(lowi + colStep); //2
					}
					else if (row==0)
					{
						if (col == PATCH_NUM_VERT-1)
						{
							continue;
						}
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
						indices.push_back(lowi + col + colStep/2);
						indices.push_back(lowi + col + colStep);
						indices.push_back(lowi + col + colStep);
						indices.push_back(highi + col);
						indices.push_back(highi + col + colStep);
						indices.push_back(highi + col + colStep);
					}
					else if (col==0)
					{
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(highi);
						indices.push_back(highi + colStep);
						indices.push_back(highi + colStep);
						indices.push_back(middlei+colStep);
						indices.push_back(middlei+colStep);						
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==0)
					{
						indices.push_back(highi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(highi);
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);						
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_DOWN

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_UP_RIGHT
#pragma region LOD_DIFF_FLAG_UP_RIGHT
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(highi); // 0s
						indices.push_back(highi + colStep/2); // 1s
						indices.push_back(middlei + colStep/2); //6s
						indices.push_back(highi + colStep); // 2s
						indices.push_back(middlei + colStep); // 3s
						indices.push_back(middlei + colStep); // 3s
						indices.push_back(middlei + colStep/2); //6s
						indices.push_back(lowi + colStep); // 4s
						indices.push_back(lowi); // 5s
						indices.push_back(lowi); // 5s
						indices.push_back(middlei + colStep/2); //6s
						indices.push_back(highi); // 0s
						break;
					}
					if (col == PATCH_NUM_VERT-1) // last column
					{
							indices.push_back(lowi+col);
							indices.push_back(middlei+col);
							indices.push_back(middlei+col);
							indices.push_back(highi+col - colStep);
							indices.push_back(highi+col);

						
					}
					else // !last column
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (row+rowStep == PATCH_NUM_VERT-1) // last line
					{
						// last column
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi+col); // 23
							indices.push_back(middlei+col); // 25
							indices.push_back(middlei+col-colStep/2); //24
							indices.push_back(lowi+col); // 15
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col-colStep); // 21
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep); // 21
						}
						else if (col!=0)
						{
							indices.push_back(highi+col);
							indices.push_back(lowi+col);
							indices.push_back(highi + col - colStep/2);
							indices.push_back(highi + col - colStep);
							indices.push_back(highi + col - colStep);
							indices.push_back(lowi + col);
							indices.push_back(lowi + col - colStep);
							indices.push_back(lowi + col - colStep);
						}
					}
					else
					{
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi + col); // 9
							indices.push_back(middlei + col); // 10
							indices.push_back(highi + col - colStep); // 8
							indices.push_back(lowi + col); // 6
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_UP_RIGHT)] = gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_UP_RIGHT

#pragma region LOD_DIFF_FLAG_UP_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(lowi); // 0
						indices.push_back(highi); // 1
						indices.push_back(lowi + colStep/2); // 2
						indices.push_back(lowi + colStep); //4
						indices.push_back(lowi + colStep); // 4
						indices.push_back(highi); // 1
						indices.push_back(highi + colStep/2); //3
						indices.push_back(highi + colStep/2); //3
						indices.push_back(lowi + colStep); // 4
						indices.push_back(highi + colStep); // 5
						break;
					}
					if (row==0)
					{
						if (col == PATCH_NUM_VERT-1)
						{
							continue;
						}
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
						indices.push_back(lowi + col + colStep/2);
						indices.push_back(lowi + col + colStep);
						indices.push_back(lowi + col + colStep);
						indices.push_back(highi + col);
						indices.push_back(highi + col + colStep);
						indices.push_back(highi + col + colStep);
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1));
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT);
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (row+rowStep == PATCH_NUM_VERT-1)
					{
						if (col!=0)
						{
							indices.push_back(highi+col);
							indices.push_back(lowi+col);
							indices.push_back(highi + col - colStep/2);
							indices.push_back(highi + col - colStep);
							indices.push_back(highi + col - colStep);
							indices.push_back(lowi + col);
							indices.push_back(lowi + col - colStep);
							indices.push_back(lowi + col - colStep);
						}
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_UP_DOWN)] = gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_UP_DOWN

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_RIGHT_DOWN
#pragma region LOD_DIFF_FLAG_RIGHT_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (lod==4)
			{
				int col = colStep;
				indices.push_back(lowi);
				indices.push_back(highi);
				indices.push_back(middlei+col-colStep/2); // 12
				indices.push_back(highi + col); //11
				indices.push_back(middlei + col); // 10
				indices.push_back(middlei + col); // 10
				indices.push_back(middlei+col-colStep/2); // 12
				indices.push_back(lowi+col); // 9
				indices.push_back(lowi+col-colStep/2); // 8
				indices.push_back(lowi+col-colStep/2); // 8
				indices.push_back(middlei+col-colStep/2); // 12
				indices.push_back(lowi+col-colStep); // 6
				break;
			}
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col == PATCH_NUM_VERT-1)
					{
						if (row==0)
						{
							indices.push_back(middlei+col-colStep/2); // 12
							indices.push_back(highi + col); //11
							indices.push_back(middlei + col); // 10
							indices.push_back(middlei + col); // 10
							indices.push_back(middlei+col-colStep/2); // 12
							indices.push_back(lowi+col); // 9
							indices.push_back(lowi+col-colStep/2); // 8
							indices.push_back(lowi+col-colStep/2); // 8
							indices.push_back(middlei+col-colStep/2); // 12
							indices.push_back(lowi+col-colStep); // 6
							indices.push_back(lowi+col-colStep); // 6
							indices.push_back(middlei+col-colStep/2); // 12
							indices.push_back(middlei+col-colStep/2); // 12
							indices.push_back(highi + col); //11
							indices.push_back(highi + col); //11
						}
						else
						{
							indices.push_back(lowi+col); // 4
							indices.push_back(middlei+col); // 5
							indices.push_back(middlei+col); // 5
							indices.push_back(highi+col - colStep); // 3
							indices.push_back(highi+col); // 6
						}
					}
					else if (row==0 && col + colStep != PATCH_NUM_VERT-1)
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
						indices.push_back(lowi + col + colStep/2);
						indices.push_back(lowi + col + colStep);
						indices.push_back(lowi + col + colStep);
						indices.push_back(highi + col);
						indices.push_back(highi + col + colStep);
						indices.push_back(highi + col + colStep);
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row==0)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 15
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 15
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep); // 13
				}
				else if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 13
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 16
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 16
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==PATCH_NUM_VERT-1)
					{
						indices.push_back(highi + col); // 13
						indices.push_back(middlei + col); // 15
						indices.push_back(highi + col - colStep); // 14
						indices.push_back(lowi + col); // 11
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_RIGHT_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_RIGHT_DOWN

	// 4C3 = 4
	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_UP_RIGHT
#pragma region LOD_DIFF_FLAG_LEFT_UP_RIGHT
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{
				if (lod==4)
				{
					indices.push_back(lowi); // 0
					indices.push_back(middlei+colStep/2); // 7
					indices.push_back(lowi+colStep); // 1
					indices.push_back(middlei+colStep); // 2
					indices.push_back(middlei+colStep); // 2
					indices.push_back(middlei+colStep/2); // 7
					indices.push_back(highi+colStep); // 3
					indices.push_back(highi+colStep/2); //4
					indices.push_back(highi+colStep/2); //4
					indices.push_back(middlei+colStep/2); // 7
					indices.push_back(highi); //5
					indices.push_back(middlei); //6
					indices.push_back(middlei); //6
					indices.push_back(middlei+colStep/2); // 7
					indices.push_back(lowi); // 0
					break;
				}
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (col==0)
					{
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(highi);
						indices.push_back(highi + colStep);
						indices.push_back(highi + colStep);
						indices.push_back(middlei+colStep);
						indices.push_back(middlei+colStep);				
					}
					else if (col == PATCH_NUM_VERT-1)
					{
						indices.push_back(lowi+col); // 7
						indices.push_back(lowi+col); // 7
						indices.push_back(middlei+col); // 8
						indices.push_back(highi+col - colStep); // 6
						indices.push_back(highi+col); // 9
					}
					else
					{
						indices.push_back(lowi + col);
						indices.push_back(highi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					//last row
					if (row + rowStep == PATCH_NUM_VERT-1)
					{
						// last column
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi+col); // 23
							indices.push_back(middlei+col); // 25
							indices.push_back(middlei+col-colStep/2); //24
							indices.push_back(lowi+col); // 15
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col-colStep); // 21
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep); // 21
						}
						// first column
						else if (col==0)
						{
							indices.push_back(highi+colStep/2); // 18
							indices.push_back(highi); // 17
							indices.push_back(highi); // 17
							indices.push_back(lowi+colStep); // 12
							indices.push_back(middlei); // 16
							indices.push_back(lowi); // 11
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
							if (col>1)
							{
								indices.push_back(highi + col - colStep/2);
								indices.push_back(highi + col - colStep);
								indices.push_back(highi + col - colStep);
								indices.push_back(lowi + col);
							}
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(highi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(highi);			
						}
						else if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi + col); // 9
							indices.push_back(middlei + col); // 10
							indices.push_back(highi + col - colStep); // 8
							indices.push_back(lowi + col); // 6
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_UP_RIGHT)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_UP_RIGHT

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_RIGHT_DOWN
#pragma region LOD_DIFF_FLAG_LEFT_RIGHT_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(middlei); //1
						indices.push_back(highi); //0
						indices.push_back(middlei + colStep/2); //7
						indices.push_back(highi+colStep); // 6
						indices.push_back(middlei+colStep); // 5
						indices.push_back(middlei+colStep); // 5
						indices.push_back(middlei + colStep/2); //7
						indices.push_back(lowi+colStep); //4
						indices.push_back(lowi+colStep/2); //3
						indices.push_back(lowi+colStep/2); //3
						indices.push_back(middlei + colStep/2); //7
						indices.push_back(lowi); //2
						indices.push_back(middlei); //1
						break;
					}
					if (row==0)
					{
						if (col==0)
						{
							indices.push_back(lowi); // 0
							indices.push_back(middlei); // 5
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(highi); // 4
							indices.push_back(highi + colStep); // 3
							indices.push_back(highi + colStep); // 3
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi+colStep); // 2
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi); // 0
							indices.push_back(lowi); // 0
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep); // 2			
						}
						else if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(highi+col); // 4
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col); // 2
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col); // 4
						}
						else
						{
							if (col < PATCH_NUM_VERT-1-colStep)
							{
								indices.push_back(lowi + col); // 0
								indices.push_back(highi + col); // 4
								indices.push_back(lowi + col + colStep/2); // 1
								indices.push_back(lowi + col + colStep); // 2
								indices.push_back(lowi + col + colStep); //2
								indices.push_back(highi + col); //4
								indices.push_back(highi + col + colStep); //3
								indices.push_back(highi + col + colStep); //3
							}
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(highi);
							indices.push_back(highi + colStep);
							indices.push_back(highi + colStep);
							indices.push_back(middlei+colStep);
							indices.push_back(middlei+colStep);				
						}
						else if (col == PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col); // 7
							indices.push_back(lowi+col); // 7
							indices.push_back(middlei+col); // 8
							indices.push_back(highi+col - colStep); // 6
							indices.push_back(highi+col); // 9
						}
						else
						{
							indices.push_back(lowi + col);
							indices.push_back(highi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					if (col==0)
					{
						indices.push_back(highi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(lowi + colStep);
						indices.push_back(lowi);
						indices.push_back(middlei);
						indices.push_back(middlei);
						indices.push_back(highi);			
					}
					else if (col==PATCH_NUM_VERT-1)
					{
						indices.push_back(highi + col); // 9
						indices.push_back(middlei + col); // 10
						indices.push_back(highi + col - colStep); // 8
						indices.push_back(lowi + col); // 6
					}
					else
					{
						indices.push_back(highi + col);
						indices.push_back(lowi + col);
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_RIGHT_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_RIGHT_DOWN

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_UP_DOWN
#pragma region LOD_DIFF_FLAG_LEFT_UP_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(lowi); // 0
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(lowi+colStep/2); //1
						indices.push_back(lowi+colStep); // 2
						indices.push_back(lowi+colStep); // 2
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(highi+colStep);//3
						indices.push_back(highi+colStep/2);//4
						indices.push_back(highi+colStep/2);//4
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(highi);//5
						indices.push_back(middlei);//6
						indices.push_back(middlei);//6
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(lowi); // 0
						break;
					}
					if (row==0)
					{
						if (col==0)
						{
							indices.push_back(lowi); // 0
							indices.push_back(middlei); // 5
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(highi); // 4
							indices.push_back(highi + colStep); // 3
							indices.push_back(highi + colStep); // 3
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi+colStep); // 2
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi); // 0
							indices.push_back(lowi); // 0
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep); // 2			
						}
						else
						{
							if (col < PATCH_NUM_VERT-1)
							{
								indices.push_back(lowi + col); // 0
								indices.push_back(highi + col); // 4
								indices.push_back(lowi + col + colStep/2); // 1
								indices.push_back(lowi + col + colStep); // 2
								indices.push_back(lowi + col + colStep); //2
								indices.push_back(highi + col); //4
								indices.push_back(highi + col + colStep); //3
								indices.push_back(highi + col + colStep); //3
							}
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(highi);
							indices.push_back(highi + colStep);
							indices.push_back(highi + colStep);
							indices.push_back(middlei+colStep);
							indices.push_back(middlei+colStep);				
						}
						else
						{
							indices.push_back(lowi + col);
							indices.push_back(highi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					//last row
					if (row + rowStep == PATCH_NUM_VERT-1)
					{
						// first column
						if (col==0)
						{
							indices.push_back(highi+colStep/2); // 18
							indices.push_back(highi); // 17
							indices.push_back(highi); // 17
							indices.push_back(lowi+colStep); // 12
							indices.push_back(middlei); // 16
							indices.push_back(lowi); // 11
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
							if (col>1)
							{
								indices.push_back(highi + col - colStep/2);
								indices.push_back(highi + col - colStep);
								indices.push_back(highi + col - colStep);
								indices.push_back(lowi + col);
								//indices.push_back(lowi + col);
							}
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(highi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(highi);			
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_UP_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_UP_DOWN

	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_UP_RIGHT_DOWN
#pragma region LOD_DIFF_FLAG_UP_RIGHT_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(lowi); // 0
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(lowi+colStep/2); //1
						indices.push_back(lowi+colStep); // 2
						indices.push_back(lowi+colStep); // 2
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(middlei+colStep);//3
						indices.push_back(highi+colStep);//4
						indices.push_back(highi+colStep);//4
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(highi+colStep/2);//5
						indices.push_back(highi);//6
						indices.push_back(highi);//6
						indices.push_back(middlei+colStep/2); // 7
						indices.push_back(lowi); // 0
						break;
					}
					if (row==0)
					{
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(highi+col); // 4
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col); // 2
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col); // 4
						}
						else if (col < PATCH_NUM_VERT-1-colStep)
						{
							indices.push_back(lowi + col); // 0
							indices.push_back(highi + col); // 4
							indices.push_back(lowi + col + colStep/2); // 1
							indices.push_back(lowi + col + colStep); // 2
							indices.push_back(lowi + col + colStep); //2
							indices.push_back(highi + col); //4
							indices.push_back(highi + col + colStep); //3
							indices.push_back(highi + col + colStep); //3
						}
					}
					else
					{
						if (col == PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col); // 4
							indices.push_back(middlei+col); // 5
							indices.push_back(middlei+col); // 5
							indices.push_back(highi+col - colStep); // 3
							indices.push_back(highi+col); // 6
						}
						else
						{
							indices.push_back(lowi + col);
							indices.push_back(highi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					//last row
					if (row + rowStep == PATCH_NUM_VERT-1)
					{
						// last column
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi+col); // 23
							indices.push_back(middlei+col); // 25
							indices.push_back(middlei+col-colStep/2); //24
							indices.push_back(lowi+col); // 15
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col-colStep); // 21
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep); // 21
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
							if (col>1)
							{
								indices.push_back(highi + col - colStep/2);
								indices.push_back(highi + col - colStep);
								indices.push_back(highi + col - colStep);
								indices.push_back(lowi + col);
								//indices.push_back(lowi + col);
							}
						}
					}
					else
					{
						if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi + col); // 9
							indices.push_back(middlei + col); // 10
							indices.push_back(highi + col - colStep); // 8
							indices.push_back(lowi + col); // 6
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_UP_RIGHT_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_UP_RIGHT_DOWN

	// select all = 1
	//------------------------------------------------------------------------
	// LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN
#pragma region LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN
	for (int lod=1; lod < 5; lod++)
	{
		int LOD_VERT_NUM = ((PATCH_NUM_VERT-1) >> lod) + 1;
		std::vector<WORD> indices;
		indices.reserve(LOD_VERT_NUM*LOD_VERT_NUM*2);
		int rowStep = 1 << lod;
		int colStep = 1 << lod;
		bool toTheRight = true;
		for (int row = 0; row< PATCH_NUM_VERT-1; row+=rowStep)
		{
			const int lowi = row * PATCH_NUM_VERT;
			const int highi = (row+rowStep) * PATCH_NUM_VERT;
			const int middlei = (row+rowStep/2) * PATCH_NUM_VERT;
			if (toTheRight)
			{				
				for (int col=0; col<PATCH_NUM_VERT; col+=colStep)
				{
					if (lod==4)
					{
						indices.push_back(lowi); // 0
						indices.push_back(middlei+colStep/2); // 8
						indices.push_back(lowi+colStep/2); // 1
						indices.push_back(lowi+colStep); // 2
						indices.push_back(lowi+colStep); // 2
						indices.push_back(middlei+colStep/2); // 8
						indices.push_back(middlei+colStep); // 3
						indices.push_back(highi+colStep); // 4
						indices.push_back(highi+colStep); // 4
						indices.push_back(middlei+colStep/2); // 8
						indices.push_back(highi+colStep/2); // 5
						indices.push_back(highi); // 6
						indices.push_back(highi); // 6
						indices.push_back(middlei+colStep/2); // 8
						indices.push_back(middlei); // 7
						indices.push_back(lowi); // 0
						
						break;
					}
					if (row==0)
					{
						if (col==0)
						{
							indices.push_back(lowi); // 0
							indices.push_back(middlei); // 5
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(highi); // 4
							indices.push_back(highi + colStep); // 3
							indices.push_back(highi + colStep); // 3
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi+colStep); // 2
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(middlei + colStep/2); //6
							indices.push_back(lowi); // 0
							indices.push_back(lowi); // 0
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep/2); // 1
							indices.push_back(lowi+colStep); // 2			
						}
						else if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(highi+col); // 4
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col); //3
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col); // 2
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(lowi+col-colStep/2); // 1
							indices.push_back(middlei+col-colStep/2); //6
							indices.push_back(lowi+col-colStep); // 0
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col-colStep); // 5
							indices.push_back(highi+col); // 4
						}
						else if (col < PATCH_NUM_VERT-1-colStep)
						{
							indices.push_back(lowi + col); // 0
							indices.push_back(highi + col); // 4
							indices.push_back(lowi + col + colStep/2); // 1
							indices.push_back(lowi + col + colStep); // 2
							indices.push_back(lowi + col + colStep); //2
							indices.push_back(highi + col); //4
							indices.push_back(highi + col + colStep); //3
							indices.push_back(highi + col + colStep); //3
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(highi);
							indices.push_back(highi + colStep);
							indices.push_back(highi + colStep);
							indices.push_back(middlei+colStep);
							indices.push_back(middlei+colStep);				
						}
						else if (col == PATCH_NUM_VERT-1)
						{
							indices.push_back(lowi+col); // 4
							indices.push_back(middlei+col); // 5
							indices.push_back(middlei+col); // 5
							indices.push_back(highi+col - colStep); // 3
							indices.push_back(highi+col); // 6
						}
						else
						{
							indices.push_back(lowi + col);
							indices.push_back(highi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(highi + (PATCH_NUM_VERT-1)); // 9
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
					indices.push_back(highi + (PATCH_NUM_VERT-1) + PATCH_NUM_VERT*rowStep/2); // 14
				}
			}
			else // toTheLeft
			{
				for (int col=PATCH_NUM_VERT-1; col>=0; col-=colStep)
				{
					//last row
					if (row + rowStep == PATCH_NUM_VERT-1)
					{
						// first column
						if (col==0)
						{
							indices.push_back(highi+colStep/2); // 18
							indices.push_back(highi); // 17
							indices.push_back(highi); // 17
							indices.push_back(lowi+colStep); // 12
							indices.push_back(middlei); // 16
							indices.push_back(lowi); // 11
						}
						// last column
						else if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi+col); // 23
							indices.push_back(middlei+col); // 25
							indices.push_back(middlei+col-colStep/2); //24
							indices.push_back(lowi+col); // 15
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(lowi+col-colStep); // 13
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col-colStep); // 21
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(middlei+col-colStep/2); // 24
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col); // 23
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep/2); // 22
							indices.push_back(highi+col-colStep); // 21
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
							if (col>1)
							{
								indices.push_back(highi + col - colStep/2);
								indices.push_back(highi + col - colStep);
								indices.push_back(highi + col - colStep);
								indices.push_back(lowi + col);
								//indices.push_back(lowi + col);
							}
						}
					}
					else
					{
						if (col==0)
						{
							indices.push_back(highi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(lowi + colStep);
							indices.push_back(lowi);
							indices.push_back(middlei);
							indices.push_back(middlei);
							indices.push_back(highi);			
						}
						else if (col==PATCH_NUM_VERT-1)
						{
							indices.push_back(highi + col); // 9
							indices.push_back(middlei + col); // 10
							indices.push_back(highi + col - colStep); // 8
							indices.push_back(lowi + col); // 6
						}
						else
						{
							indices.push_back(highi + col);
							indices.push_back(lowi + col);
						}
					}
				}
				if (row+rowStep < PATCH_NUM_VERT-1)
				{
					indices.push_back(lowi);
					indices.push_back(highi);
				}
			}
			toTheRight = !toTheRight;
		}
		mIndexBuffersRefined[lod-1][ConvertDiffFlag(LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN)] = 
			gFBEnv->pEngine->GetRenderer()->CreateIndexBuffer((void*)&indices[0], indices.size(), INDEXBUFFER_FORMAT_16BIT);
	}
#pragma endregion LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN
}

unsigned int Terrain::ConvertDiffFlag(LOD_DIFF_FLAG flag) const
{
	switch(flag)
	{
	case LOD_DIFF_FLAG_LEFT:
		return 0;
	case LOD_DIFF_FLAG_UP:
		return 1;
	case LOD_DIFF_FLAG_RIGHT:
		return 2;
	case LOD_DIFF_FLAG_DOWN:
		return 3;
	case LOD_DIFF_FLAG_LEFT_UP:
		return 4;
	case LOD_DIFF_FLAG_LEFT_RIGHT:
		return 5;
	case LOD_DIFF_FLAG_LEFT_DOWN:
		return 6;
	case LOD_DIFF_FLAG_UP_RIGHT:
		return 7;
	case LOD_DIFF_FLAG_UP_DOWN:
		return 8;
	case LOD_DIFF_FLAG_RIGHT_DOWN:
		return 9;
	case LOD_DIFF_FLAG_LEFT_UP_RIGHT:
		return 10;
	case LOD_DIFF_FLAG_LEFT_RIGHT_DOWN:
		return 11;
	case LOD_DIFF_FLAG_LEFT_UP_DOWN:
		return 12;
	case LOD_DIFF_FLAG_UP_RIGHT_DOWN:
		return 13;
	case LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN:
		return 14;
	default:
		assert(0 && "Wrong flag!");
	}

	return -1;
}

std::string Terrain::ConvertDiffFlagToString(LOD_DIFF_FLAG flag) const
{
	switch(flag)
	{
	case LOD_DIFF_FLAG_LEFT:
		return std::string("LOD_DIFF_FLAG_LEFT");
	case LOD_DIFF_FLAG_UP:
		return std::string("LOD_DIFF_FLAG_UP");
	case LOD_DIFF_FLAG_RIGHT:
		return std::string("LOD_DIFF_FLAG_RIGHT");
	case LOD_DIFF_FLAG_DOWN:
		return std::string("LOD_DIFF_FLAG_DOWN");
	case LOD_DIFF_FLAG_LEFT_UP:
		return std::string("LOD_DIFF_FLAG_LEFT_UP");
	case LOD_DIFF_FLAG_LEFT_RIGHT:
		return std::string("LOD_DIFF_FLAG_LEFT_RIGHT");
	case LOD_DIFF_FLAG_LEFT_DOWN:
		return std::string("LOD_DIFF_FLAG_LEFT_DOWN");
	case LOD_DIFF_FLAG_UP_RIGHT:
		return std::string("LOD_DIFF_FLAG_UP_RIGHT");
	case LOD_DIFF_FLAG_UP_DOWN:
		return std::string("LOD_DIFF_FLAG_UP_DOWN");
	case LOD_DIFF_FLAG_RIGHT_DOWN:
		return std::string("LOD_DIFF_FLAG_RIGHT_DOWN");
	case LOD_DIFF_FLAG_LEFT_UP_RIGHT:
		return std::string("LOD_DIFF_FLAG_LEFT_UP_RIGHT");
	case LOD_DIFF_FLAG_LEFT_RIGHT_DOWN:
		return std::string("LOD_DIFF_FLAG_LEFT_RIGHT_DOWN");
	case LOD_DIFF_FLAG_LEFT_UP_DOWN:
		return std::string("LOD_DIFF_FLAG_LEFT_UP_DOWN");
	case LOD_DIFF_FLAG_UP_RIGHT_DOWN:
		return std::string("LOD_DIFF_FLAG_UP_RIGHT_DOWN");
	case LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN:
		return std::string("LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN");
	default:
		return std::string("LOD_DIFF_FLAG_NONE");
	}
}

//---------------------------------------------------------------------------
void Terrain::OnInput(IMouse* pMouse, IKeyboard* pKeyboard)
{
	std::vector<LOD_DIFF_FLAG> flags;
	flags.resize(16);
	flags[0] = LOD_DIFF_FLAG_NONE;
	flags[1] = LOD_DIFF_FLAG_LEFT;
	flags[2] = LOD_DIFF_FLAG_UP;
	flags[3] = LOD_DIFF_FLAG_RIGHT;
	flags[4] = LOD_DIFF_FLAG_DOWN;
	flags[5] = LOD_DIFF_FLAG_LEFT_UP;
	flags[6] = LOD_DIFF_FLAG_LEFT_RIGHT;
	flags[7] = LOD_DIFF_FLAG_LEFT_DOWN;
	flags[8] = LOD_DIFF_FLAG_UP_RIGHT;
	flags[9] = LOD_DIFF_FLAG_UP_DOWN;
	flags[10] =LOD_DIFF_FLAG_RIGHT_DOWN;
	flags[11] = LOD_DIFF_FLAG_LEFT_UP_RIGHT;
	flags[12] = LOD_DIFF_FLAG_LEFT_RIGHT_DOWN;
	flags[13] = LOD_DIFF_FLAG_LEFT_UP_DOWN;
	flags[14] = LOD_DIFF_FLAG_UP_RIGHT_DOWN;
	flags[15] = LOD_DIFF_FLAG_LEFT_UP_RIGHT_DOWN;

	if (pKeyboard && pKeyboard->IsValid())
	{
		if (pKeyboard->IsKeyPressed(VK_NUMPAD2))
		{
			mSelectedBar++;
			ColorRamp& cr = mMaterial->GetColorRamp(2, BINDING_SHADER_PS);
			if (mSelectedBar >= (int)cr.NumBars())
				mSelectedBar = cr.NumBars()-1;
		}
		else if (pKeyboard->IsKeyPressed(VK_NUMPAD8))
		{
			mSelectedBar--;
			if (mSelectedBar <0)
				mSelectedBar = 0;
		}

		if (pKeyboard->IsKeyDown(VK_ADD))
		{
			ColorRamp& cr = mMaterial->GetColorRamp(2, BINDING_SHADER_PS);
			cr.GetBar(mSelectedBar).position+=0.01f;
			mMaterial->RefreshColorRampTexture(2, BINDING_SHADER_PS);
		}
		else if (pKeyboard->IsKeyDown(VK_SUBTRACT))
		{
			ColorRamp& cr = mMaterial->GetColorRamp(2, BINDING_SHADER_PS);
			cr.GetBar(mSelectedBar).position-=0.01f;
			mMaterial->RefreshColorRampTexture(2, BINDING_SHADER_PS);
		}

		if (pKeyboard->IsKeyDown(VK_OEM_COMMA))
		{
			mGrassLerp-=0.02f;
			if (mGrassLerp<0)
				mGrassLerp=0;

			Color grassColor = Lerp(mGrassColor[0], mGrassColor[1], mGrassLerp);
			mMaterial->SetMaterialParameters(1, grassColor.GetVec4());

		}
		else if (pKeyboard->IsKeyDown(VK_OEM_PERIOD))
		{
			mGrassLerp+=0.02f;
			if(mGrassLerp>1.0f)
				mGrassLerp = 1.0f;
			Color grassColor = Lerp(mGrassColor[0], mGrassColor[1], mGrassLerp);
			mMaterial->SetMaterialParameters(1, grassColor.GetVec4());
		}
		
		if ( pKeyboard->IsKeyPressed(VK_OEM_3) )// `
		{
			mDebugging = !mDebugging;
		}

		if (pKeyboard->IsKeyPressed(VK_NEXT))
		{
			if (mDebuggingFlagIndex==0)
			{
				mDebuggingLOD += 1;
				Clamp(mDebuggingLOD, 0, 4);
			}
			else
			{
				mDebuggingFlagLOD += 1;
				Clamp(mDebuggingFlagLOD, 0, 3);
			}
		}
		else if (pKeyboard->IsKeyPressed(VK_PRIOR))
		{
			if (mDebuggingFlagIndex==0)
			{
				mDebuggingLOD -= 1;
				Clamp(mDebuggingLOD, 0, 4);
			}
			else
			{
				mDebuggingFlagLOD -= 1;
				Clamp(mDebuggingFlagLOD, 0, 3);
			}
		}

		if (pKeyboard->IsKeyPressed(VK_HOME))
		{
			mDebuggingFlagIndex-=1;
			Clamp(mDebuggingFlagIndex, 0, 15);
			mDebuggingFlag = flags[mDebuggingFlagIndex];
			printf("%s\n", ConvertDiffFlagToString(mDebuggingFlag).c_str());
		}
		else if (pKeyboard->IsKeyPressed(VK_END))
		{
			mDebuggingFlagIndex+=1;
			Clamp(mDebuggingFlagIndex, 0, 15);
			mDebuggingFlag = flags[mDebuggingFlagIndex];
			printf("%s\n", ConvertDiffFlagToString(mDebuggingFlag).c_str());
		}
	}
}

void Terrain::EnableInputListener(bool enable)
{
	mInputListenerEnabled = enable;
}

bool Terrain::IsEnabledInputLIstener() const
{
	return mInputListenerEnabled;
}