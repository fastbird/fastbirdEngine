#include <Engine/StdAfx.h>
#include <Engine/Voxelizer.h>
#include <Engine/IRenderTarget.h>
#include <Engine/ICamera.h>
#include <Engine/IColladaImporter.h>

namespace fastbird
{

IVoxelizer* IVoxelizer::CreateVoxelizer()
{
	return FB_NEW(Voxelizer);
}

void IVoxelizer::DeleteVoxelizer(IVoxelizer* p)
{
	FB_SAFE_DEL(p);
}

Voxelizer::Voxelizer()
	: mDistanceMap(0)
{
	mColladaImporter = IColladaImporter::CreateColladaImporter();
}

Voxelizer::~Voxelizer()
{
	free(mDistanceMap);
	mColladaImporter = 0;
}

void Voxelizer::FinishSmartPtr(){
	assert(NumRefs() == 0);
	FB_DELETE(this);
}

bool Voxelizer::RunVoxelizer(const char* filename, UINT numVoxels, bool swapYZ, bool oppositCull)
{
	mColladaImporter = 0;
	mColladaImporter = IColladaImporter::CreateColladaImporter();
	assert(filename);	
	mColladaImporter->ImportCollada(filename, swapYZ, oppositCull, false, true, false, false, false);
	
	mNumVoxels = numVoxels;
	// Draw depth maps
	CalcDistanceMap();

	return true;
}

IMeshObject* Voxelizer::GetMeshObject() const
{
	return mColladaImporter->GetMeshObject();
}

void Voxelizer::CalcDistanceMap()
{
	if (!gFBEnv || !gFBEnv->pRenderer)
		return;
	const UINT numVoxelsSQ= mNumVoxels*mNumVoxels;
	const UINT numVoxelsCB = mNumVoxels*mNumVoxels*mNumVoxels;
	free(mDistanceMap);
	mDistanceMap = (float*)malloc(sizeof(float) * numVoxelsCB);
	memset(mDistanceMap, 0, sizeof(float) * numVoxelsCB);

	std::vector<float> x_min, x_max;
	std::vector<float> y_min, y_max;
	std::vector<float> z_min, z_max;
	RASTERIZER_DESC rd_cull_front;
	rd_cull_front.CullMode = CULL_MODE_FRONT;
	RASTERIZER_DESC rd_cull_back;

	IMeshObject* pMeshObject = mColladaImporter->GetMeshObject();
	if (!pMeshObject)
	{
		Log("Voxelizing mesh %s is not found!", mFilepath.c_str());
		return;
	}
		
	float radius = pMeshObject->GetBoundingVolume()->GetRadius() * 1.05f;
	// draw depth maps;
	// x axis
	RenderTargetParam param;
	param.mEveryFrame = false;
	param.mSize = Vec2I(mNumVoxels, mNumVoxels);
	param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
	param.mShaderResourceView = true;
	param.mMipmap = false;
	param.mCubemap = false;
	param.mWillCreateDepth = true;
	param.mUsePool = true;

	SmartPtr<IRenderTarget> pDepthRT = gFBEnv->pRenderer->CreateRenderTarget(param);
	pDepthRT->SetDepthStencilDesc(mNumVoxels, mNumVoxels, PIXEL_FORMAT_D32_FLOAT, false, false);
	ICamera* pCam = pDepthRT->GetCamera();
	pCam->SetOrthogonal(true);
	pCam->SetWidth((int)(radius*2.0f));
	pCam->SetHeight((int)(radius*2.0f));
	pCam->SetNearFar(-radius, radius);
	pCam->SetPos(Vec3(0, 0, 0));
	auto scene = pDepthRT->CreateScene();
	scene->AttachObject(pMeshObject);
	pMeshObject->SetMaterial("es/materials/collada_mesh.material");
	auto material = pMeshObject->GetMaterial();
	material->SetRasterizerState(rd_cull_back);
	ITexture* pDepthTexture = pDepthRT->GetDepthStencilTexture();
	ITexture* pColorTexture = pDepthRT->GetRenderTargetTexture();
	assert(pDepthTexture);
	SmartPtr<ITexture> pStaging = gFBEnv->pRenderer->CreateTexture(0, mNumVoxels, mNumVoxels, PIXEL_FORMAT_D32_FLOAT, 
		BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);

	
	pCam->SetDir(Vec3(-radius, 0, 0).NormalizeCopy());	
	pDepthRT->Render();
	pColorTexture->SaveToFile("test_x.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	MapData data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	x_min.assign(numVoxelsSQ, 0);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&x_min[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);


	// inverse
	material->SetRasterizerState(rd_cull_front);
	pDepthRT->Render();
	//pColorTexture->SaveToFile("test.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	x_max.assign(numVoxelsSQ, 0);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&x_max[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);

	// from x to -x;
	// yz plane
	for (size_t sz = 0; sz < mNumVoxels; sz++)
	{
		size_t src_row_i = sz * mNumVoxels;
		size_t dest_depth_i = (mNumVoxels-1-sz) * numVoxelsSQ;
		for (size_t sy = 0; sy<mNumVoxels; sy++)
		{
			size_t dest_row_i = sy * mNumVoxels;
			if (x_min[src_row_i+sy]==1.0f || x_max[src_row_i+sy]==1.0f)
			{
				for (size_t dx = 0; dx < mNumVoxels; dx++)
				{
					mDistanceMap[dest_depth_i+dest_row_i+dx] = -1.f;
				}
			}
			else
			{
				size_t xi_min = (size_t)(x_min[src_row_i+sy] * mNumVoxels);
				size_t xi_max = (size_t)(x_max[src_row_i+sy] * mNumVoxels);
				if (xi_max < xi_min)
					std::swap(xi_max, xi_min);
				
				for (size_t dx = xi_min; dx <= xi_max; dx++)
				{
					size_t destIdx = dest_depth_i+dest_row_i+dx;
					if (mDistanceMap[destIdx] !=-1.f)
					{
						mDistanceMap[destIdx] = 1.f;
					}
				}

			}
		}
	}

	// y axis
	pCam->SetDir(Vec3(0, radius, 0).NormalizeCopy());
	material->SetRasterizerState(rd_cull_back);
	pDepthRT->Render();
	//pColorTexture->SaveToFile("test.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	y_min.assign(numVoxelsSQ, 0);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&y_min[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);

	//inverse
	material->SetRasterizerState(rd_cull_front);
	pDepthRT->Render();
	//pColorTexture->SaveToFile("test.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	y_max.assign(numVoxelsSQ, 0);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&y_max[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}		
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);

	// from -y to y;
	// xz plane
	for (size_t sz = 0; sz < mNumVoxels; sz++)
	{
		size_t src_row_i = sz * mNumVoxels;
		size_t dest_depth_i = (mNumVoxels-1-sz) * numVoxelsSQ;
		for (size_t sx = 0; sx<mNumVoxels; sx++)
		{
			if (y_min[src_row_i+sx]==1.0f || y_max[src_row_i+sx]==1.0f)
			{
				for (size_t dy = 0; dy < mNumVoxels; dy++)
				{
					mDistanceMap[dest_depth_i + dy*mNumVoxels + sx] = -1.f;
				}
			}
			else
			{
				size_t yi_min = (size_t)(y_min[src_row_i+sx] * mNumVoxels);
				size_t yi_max = (size_t)(y_max[src_row_i+sx] * mNumVoxels);
				
				for (size_t dy = yi_min; dy < yi_max; dy++)
				{
					size_t destIdx = dest_depth_i + dy*mNumVoxels + sx;
					if (mDistanceMap[destIdx] !=-1.f)
					{
						mDistanceMap[destIdx] = 1.f;
					}
				}
			}
		}
	}

	// z axis
	pCam->SetDir(Vec3(0, 0, -radius).NormalizeCopy());
	material->SetRasterizerState(rd_cull_back);
	pDepthRT->Render();
	//pColorTexture->SaveToFile("test.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	z_min.assign(numVoxelsSQ, 0.f);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&z_min[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);

	//inverse
	material->SetRasterizerState(rd_cull_front);
	pDepthRT->Render();
	//pColorTexture->SaveToFile("test.bmp");
	pDepthTexture->CopyToStaging(pStaging, 0, 0, 0, 0, 0, 0);
	data = pStaging->Map(0, MAP_TYPE_READ, MAP_FLAG_NONE);
	z_max.assign(numVoxelsSQ, 0.f);
	if (data.pData)
	{
		float* pData = (float*)data.pData;
		BYTE* byteData = (BYTE*)data.pData;
		for (unsigned row = 0; row < mNumVoxels; ++row){
			memcpy(&z_max[row * mNumVoxels], byteData + row*data.RowPitch, mNumVoxels*sizeof(float));
		}
	}
	else
	{
		Log("Error to lock the staging texture in voxelizer.");
	}
	pStaging->Unmap(0);

	// from z to -z;
	// xy plane
	for (size_t sy = 0; sy < mNumVoxels; sy++)
	{
		size_t src_row_i = sy * mNumVoxels;
		size_t dest_row_i = (mNumVoxels - 1 - sy) * mNumVoxels;
		for (size_t sx = 0; sx<mNumVoxels; sx++)
		{
			if (z_min[src_row_i+sx]==1.0f || z_max[src_row_i+sx]==1.0f)
			{
				for (size_t dz = 0; dz < mNumVoxels; dz++)
				{
					size_t dest_depth_i = dz * numVoxelsSQ;
					mDistanceMap[dest_depth_i+dest_row_i+sx] = -1.f;
				}
			}
			else
			{
				size_t zi_min = (size_t)(z_min[src_row_i+sx] * mNumVoxels);
				size_t zi_max = (size_t)(z_max[src_row_i+sx] * mNumVoxels);
				
				for (size_t dz = zi_min; dz < zi_max; dz++)
				{
					size_t dest_idx = (mNumVoxels-1-dz) * numVoxelsSQ + dest_row_i + sx;
					if (mDistanceMap[dest_idx] !=-1.f)
					{
						mDistanceMap[dest_idx] = 1.f;
					}
				}
			}
		}
	}

	material->SetRasterizerState(rd_cull_back);

	CreateHull();
}

void Voxelizer::CreateHull() // with mDistanceMap
{
	mHulls.clear();
	UINT half = mNumVoxels/2;
	const UINT numVoxelsSQ = mNumVoxels*mNumVoxels;
	for (size_t z=0; z<mNumVoxels; z++)
	{
		size_t zIdx = z * numVoxelsSQ;
		for (size_t y=0; y<mNumVoxels; y++)
		{
			size_t yIdx = y * mNumVoxels;
			for (size_t x=0; x<mNumVoxels; x++)
			{
				size_t idx = zIdx + yIdx + x;
				if (mDistanceMap[idx]==1.0f)
				{
					bool inner = true;
					//check +x, -x
					if (x<mNumVoxels-1)
					{
						inner = inner && mDistanceMap[idx+1]==1.0f;
					}
					if (x>0)
					{
						inner = inner && mDistanceMap[idx-1] ==1.0f;
					}

					// check +y, -y
					if (y<mNumVoxels-1)
					{
						inner = inner && mDistanceMap[zIdx + (y+1)*mNumVoxels + x]==1.0f;
					}
					if (y>0)
					{
						inner = inner && mDistanceMap[zIdx + (y-1)*mNumVoxels + x]==1.0f;
					}

					// check +z, -z
					if (z<mNumVoxels-1)
					{
						inner = inner && mDistanceMap[(z+1) * numVoxelsSQ + yIdx + x]==1.0f;
					}
					if (z>0)
					{
						inner = inner && mDistanceMap[(z-1) * numVoxelsSQ + yIdx + x]==1.0f;
					}

					if (!inner)
					{
						mHulls.push_back(Vec3I(x-half, y-half, z-half));
					}
				}
			}
		}
	}

	Log("Total hull = %u", mHulls.size());
}
}