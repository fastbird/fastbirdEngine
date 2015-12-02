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
#include "Voxelizer.h"
#include "EngineFacade.h"
#include "FBSceneObjectFactory/SceneObjectFactory.h"
#include "FBSceneObjectFactory/MeshObject.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/Camera.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Texture.h"
using namespace fb;
class Voxelizer::Impl{
public:
	std::string mFilepath;
	float mVoxelSize;

	float* mDistanceMap;
	std::vector<Vec3I> mHulls;
	UINT mNumVoxels; // in one axis.
	MeshObjectPtr mMeshObject;

	//---------------------------------------------------------------------------
	bool RunVoxelizer(const char* filename, UINT numVoxels, bool swapYZ, bool oppositCull)
	{
		
		MeshImportDesc importDesc;
		importDesc.mergeMaterialGroups = true;
		importDesc.generateTangent = false;
		importDesc.keepMeshData = false;
		importDesc.oppositeCull = oppositCull;
		importDesc.useIndexBuffer = false;
		importDesc.yzSwap = swapYZ;
		mMeshObject = SceneObjectFactory::GetInstance().CreateMeshObject(filename, importDesc);
		if (!mMeshObject){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Failed to import(%s)", filename).c_str());
		}

		mNumVoxels = numVoxels;
		// Draw depth maps
		CalcDistanceMap();

		return true;
	}

	MeshObjectPtr GetMeshObject() const
	{
		return mMeshObject;
	}

	void CalcDistanceMap()
	{
		const UINT numVoxelsSQ = mNumVoxels*mNumVoxels;
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

		if (!mMeshObject)
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Voxelizing mesh %s is not found!", mFilepath.c_str()).c_str());
			return;
		}

		float radius = mMeshObject->GetBoundingVolume()->GetRadius() * 1.05f;
		// draw depth maps;
		// x axis
		RenderTargetParamEx param;
		param.mEveryFrame = false;
		param.mSize = Vec2I(mNumVoxels, mNumVoxels);
		param.mPixelFormat = PIXEL_FORMAT_R8G8B8A8_UNORM;
		param.mShaderResourceView = true;
		param.mMipmap = false;
		param.mCubemap = false;
		param.mWillCreateDepth = true;
		param.mUsePool = true;
		param.mSceneNameToCreateAndOwn = FormatString("Voxelizer_%x", this);
		RenderTargetPtr pDepthRT = EngineFacade::GetInstance().CreateRenderTarget(param);			
		pDepthRT->SetDepthStencilDesc(mNumVoxels, mNumVoxels, PIXEL_FORMAT_D32_FLOAT, false, false);
		auto pCam = pDepthRT->GetCamera();
		pCam->SetOrthogonal(true);
		pCam->SetWidth(radius*2.0f);
		pCam->SetHeight(radius*2.0f);
		pCam->SetNearFar(-radius, radius);
		pCam->SetPosition(Vec3(0, 0, 0));
		auto scene = pDepthRT->GetScene();
		scene->AttachObjectFB(mMeshObject);
		mMeshObject->SetMaterial("EssentialEngineData/materials/collada_mesh.material", PASS_NORMAL);
		auto material = mMeshObject->GetMaterial();
		material->SetRasterizerState(rd_cull_back);
		TexturePtr pDepthTexture = pDepthRT->GetDepthStencilTexture();
		TexturePtr pColorTexture = pDepthRT->GetRenderTargetTexture();
		assert(pDepthTexture);
		TexturePtr pStaging = Renderer::GetInstance().CreateTexture(0, mNumVoxels, mNumVoxels, PIXEL_FORMAT_D32_FLOAT,
			BUFFER_USAGE_STAGING, BUFFER_CPU_ACCESS_READ, TEXTURE_TYPE_DEFAULT);

		pCam->SetDirection(Vec3(-radius, 0, 0).NormalizeCopy());
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
		}
		pStaging->Unmap(0);

		// from x to -x;
		// yz plane
		for (size_t sz = 0; sz < mNumVoxels; sz++)
		{
			size_t src_row_i = sz * mNumVoxels;
			size_t dest_depth_i = (mNumVoxels - 1 - sz) * numVoxelsSQ;
			for (size_t sy = 0; sy<mNumVoxels; sy++)
			{
				size_t dest_row_i = sy * mNumVoxels;
				if (x_min[src_row_i + sy] == 1.0f || x_max[src_row_i + sy] == 1.0f)
				{
					for (size_t dx = 0; dx < mNumVoxels; dx++)
					{
						mDistanceMap[dest_depth_i + dest_row_i + dx] = -1.f;
					}
				}
				else
				{
					size_t xi_min = (size_t)(x_min[src_row_i + sy] * mNumVoxels);
					size_t xi_max = (size_t)(x_max[src_row_i + sy] * mNumVoxels);
					if (xi_max < xi_min)
						std::swap(xi_max, xi_min);

					for (size_t dx = xi_min; dx <= xi_max; dx++)
					{
						size_t destIdx = dest_depth_i + dest_row_i + dx;
						if (mDistanceMap[destIdx] != -1.f)
						{
							mDistanceMap[destIdx] = 1.f;
						}
					}

				}
			}
		}

		// y axis
		pCam->SetDirection(Vec3(0, radius, 0).NormalizeCopy());
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
		}
		pStaging->Unmap(0);

		// from -y to y;
		// xz plane
		for (size_t sz = 0; sz < mNumVoxels; sz++)
		{
			size_t src_row_i = sz * mNumVoxels;
			size_t dest_depth_i = (mNumVoxels - 1 - sz) * numVoxelsSQ;
			for (size_t sx = 0; sx<mNumVoxels; sx++)
			{
				if (y_min[src_row_i + sx] == 1.0f || y_max[src_row_i + sx] == 1.0f)
				{
					for (size_t dy = 0; dy < mNumVoxels; dy++)
					{
						mDistanceMap[dest_depth_i + dy*mNumVoxels + sx] = -1.f;
					}
				}
				else
				{
					size_t yi_min = (size_t)(y_min[src_row_i + sx] * mNumVoxels);
					size_t yi_max = (size_t)(y_max[src_row_i + sx] * mNumVoxels);

					for (size_t dy = yi_min; dy < yi_max; dy++)
					{
						size_t destIdx = dest_depth_i + dy*mNumVoxels + sx;
						if (mDistanceMap[destIdx] != -1.f)
						{
							mDistanceMap[destIdx] = 1.f;
						}
					}
				}
			}
		}

		// z axis
		pCam->SetDirection(Vec3(0, 0, -radius).NormalizeCopy());
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
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
			Logger::Log(FB_ERROR_LOG_ARG, "Error to lock the staging texture in voxelizer.");
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
				if (z_min[src_row_i + sx] == 1.0f || z_max[src_row_i + sx] == 1.0f)
				{
					for (size_t dz = 0; dz < mNumVoxels; dz++)
					{
						size_t dest_depth_i = dz * numVoxelsSQ;
						mDistanceMap[dest_depth_i + dest_row_i + sx] = -1.f;
					}
				}
				else
				{
					size_t zi_min = (size_t)(z_min[src_row_i + sx] * mNumVoxels);
					size_t zi_max = (size_t)(z_max[src_row_i + sx] * mNumVoxels);

					for (size_t dz = zi_min; dz < zi_max; dz++)
					{
						size_t dest_idx = (mNumVoxels - 1 - dz) * numVoxelsSQ + dest_row_i + sx;
						if (mDistanceMap[dest_idx] != -1.f)
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

	void CreateHull() // with mDistanceMap
	{
		mHulls.clear();
		UINT half = mNumVoxels / 2;
		const UINT numVoxelsSQ = mNumVoxels*mNumVoxels;
		for (size_t z = 0; z<mNumVoxels; z++)
		{
			size_t zIdx = z * numVoxelsSQ;
			for (size_t y = 0; y<mNumVoxels; y++)
			{
				size_t yIdx = y * mNumVoxels;
				for (size_t x = 0; x<mNumVoxels; x++)
				{
					size_t idx = zIdx + yIdx + x;
					if (mDistanceMap[idx] == 1.0f)
					{
						bool inner = true;
						//check +x, -x
						if (x<mNumVoxels - 1)
						{
							inner = inner && mDistanceMap[idx + 1] == 1.0f;
						}
						if (x>0)
						{
							inner = inner && mDistanceMap[idx - 1] == 1.0f;
						}

						// check +y, -y
						if (y<mNumVoxels - 1)
						{
							inner = inner && mDistanceMap[zIdx + (y + 1)*mNumVoxels + x] == 1.0f;
						}
						if (y>0)
						{
							inner = inner && mDistanceMap[zIdx + (y - 1)*mNumVoxels + x] == 1.0f;
						}

						// check +z, -z
						if (z<mNumVoxels - 1)
						{
							inner = inner && mDistanceMap[(z + 1) * numVoxelsSQ + yIdx + x] == 1.0f;
						}
						if (z>0)
						{
							inner = inner && mDistanceMap[(z - 1) * numVoxelsSQ + yIdx + x] == 1.0f;
						}

						if (!inner)
						{
							mHulls.push_back(Vec3I(x - half, y - half, z - half));
						}
					}
				}
			}
		}

		Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Total hull = %u", mHulls.size()).c_str());
	}
	const HULLS& GetHulls() const{
		return mHulls;
	}

};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(Voxelizer);

Voxelizer::Voxelizer()
	:mImpl(new Impl)
{

}
Voxelizer::~Voxelizer(){

}

bool Voxelizer::RunVoxelizer(const char* filename, UINT numVoxels, bool swapYZ, bool oppositCull) {
	return mImpl->RunVoxelizer(filename, numVoxels, swapYZ, oppositCull);
}

MeshObjectPtr Voxelizer::GetMeshObject() const {
	return mImpl->GetMeshObject();
}

const Voxelizer::HULLS& Voxelizer::GetHulls() const {
	return mImpl->GetHulls();
}

