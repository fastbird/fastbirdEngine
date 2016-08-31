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
#include "ColladaImporter.h"
#include "FBColladaData.h"
#include "FBDebugLib/Logger.h"
#include "FBStringLib/StringLib.h"
#include "FBFileSystem/FileSystem.h"
#include "FBAnimation/AnimationData.h"
#include <COLLADAFWIWriter.h>
#include <libxml/parser.h>
#include <unordered_map>

static const float PI = 3.1415926535897932384626433832795f;
namespace fb{
typedef std::vector< collada::CollisionInfo > COLLISION_INFOS;
class ColladaImporter::Impl : public COLLADAFW::IWriter{
public:	
	typedef std::vector<float> FLOAT_DATA;
	ColladaImporter* mSelf;
	ImportOptions mOptions;
	ColladaMeshObjects mMeshObjects;
	ColladaMeshObjects mCollisionMeshes;
	collada::CAMERA_DATAS mCameraDatas;
	collada::MeshGroupPtr mMeshGroup;
	std::string mFilepath;	
	
	typedef std::vector<unsigned> INDICES;
	typedef std::vector< INDICES > INDICES_PRIMITIVES;// indices per primitives			
	typedef std::vector< std::string > MATERIALS_PRIMITIVES; // materials per primitives
	//std::vector< MATERIALS_PRIMITIVES > mMaterials; // materials per mesh

	struct MeshInfo
	{
		std::string mName;
		std::string mUniqueId;
		FLOAT_DATA mPos;
		FLOAT_DATA mNormals;
		FLOAT_DATA mUVs;
		std::vector<bool> mHasUVs;
		int mNumPrimitives;
		// per primitives
		INDICES_PRIMITIVES mPosIndices;
		INDICES_PRIMITIVES mNormalIndices;
		INDICES_PRIMITIVES mUVIndices;
		MATERIALS_PRIMITIVES mMaterials;

	};
	std::vector<MeshInfo> mMeshInfos;
	std::map<std::string, AnimationDataPtr> mAnimData;

	//---------------------------------------------------------------------------
	Impl(ColladaImporter* self)
		: mSelf(self)
		, mMeshGroup(collada::MeshGroupPtr(new collada::MeshGroup))
	{
	}

	bool ImportCollada(const char* filepath){
		return ImportCollada(filepath, ImportOptions());

	}
	bool ImportCollada(const char* filepath, const ImportOptions& options){		
		assert(filepath);
		mMeshObjects.clear();
		mOptions = options;
		bool successful = false;		
		if (strcmp(filepath, mFilepath.c_str()))
		{
			// different file.
			mFilepath = filepath;
			COLLADASaxFWL::Loader loader;
			auto resourcePath = FileSystem::GetResourcePathIfPathNotExists(filepath);
			successful = loader.loadDocument(resourcePath.c_str(), this);
			if (!successful)
			{
				Logger::Log(FB_ERROR_LOG_ARG, FormatString("Importing dae(%s) is failed!", filepath).c_str());
			}
		}
		else
		{
			for (auto& meshInfo : mMeshInfos)
			{
				FeedGeometry(&meshInfo);
			}
			successful = true;
		}

		// Set Animation
		if (!mOptions.mUseMeshGroup)
		{
			for (const auto& animData : mAnimData)
			{
				
				std::string actionFile = FileSystem::ReplaceExtension(filepath, "");
				actionFile += ".actions";
				for (auto& it : mMeshObjects)
				{
					auto meshobj = it.second;
					if (meshobj->mName == animData.first)
					{
						meshobj->mAnimationData = animData.second;
						meshobj->mAnimationData->ParseAction(actionFile.c_str());
						break;
					}
				}
			}
		}
		else
		{
			if (mMeshGroup)
			{
				for (auto& animData : mAnimData)
				{
					std::string actionFile = FileSystem::ReplaceExtension(filepath, "");
					actionFile += ".actions";
					for (auto& data : mMeshGroup->mMeshes){
						if (data.second.mMesh->mName == animData.first){
							data.second.mMesh->mAnimationData = animData.second;
							data.second.mMesh->mAnimationData->ParseAction(actionFile.c_str());
						}
					}
				}
			}
		}

		return successful;
	}

	collada::MeshPtr GetMeshObject() const{
		if (mOptions.mUseMeshGroup){
			Logger::Log(FB_ERROR_LOG_ARG, "You parsed the .dae as a MeshGroup but now retrieving as a Mesh.");
		}
		if (mMeshObjects.empty())
			return 0;
		return mMeshObjects.begin()->second;
	}

	// private
	collada::MeshPtr GetMeshObject(const char* id) const{
		for (auto it : mMeshObjects)
		{
			if (strcmp(it.first.c_str(), id) == 0)
				return it.second;
		}
		for (auto it : mCollisionMeshes)
		{
			if (strcmp(it.first.c_str(), id) == 0)
				return it.second;
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Collada Importer cannot fild a mesh with id(%s).", id).c_str());		
		return 0;
	}

	collada::MeshGroupPtr GetMeshGroup() const{
		return mMeshGroup;
	}

	IteratorWrapper<ColladaMeshObjects> GetMeshIterator(){
		return IteratorWrapper<ColladaMeshObjects>(mMeshObjects);
	}

	unsigned GetNumMeshes() const {
		return mMeshObjects.size();
	}

	const collada::CameraData& GetCameraData(const char* id) const{
		auto it = mCameraDatas.find(id);
		if (it == mCameraDatas.end()){
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Collada Camera %s is not found", id).c_str());
			static collada::CameraData dummy;
			return dummy;
		}
		return it->second;
	}
	
	// Private functions
	std::string GetMaterialFilepath(const char* sz)
	{
		if (!sz || strlen(sz) == 0)
			return std::string();

		// example of 'sz'
		// in case dae file exported from Blender.
		// data_materials_hull2_material-material

		struct funcObj
		{
			bool operator()(char v)
			{
				return v == '_';

			}
		};
		const char* p = strrchr(sz, '-');
		std::string ret(sz, sz + (p - sz));
		std::replace_if(ret.begin(), ret.end(), funcObj(), '/');

		size_t dotPos = ret.find_last_of('/');
		if (dotPos != std::string::npos)
			ret[dotPos] = '.';
		else
			return std::string();
		return ret;

	}

	MeshInfo* CopyData(COLLADAFW::Mesh* pColladaMesh){
		using namespace COLLADAFW;
		mMeshInfos.push_back(MeshInfo());
		MeshInfo& meshInfo = mMeshInfos.back();
		meshInfo.mName = pColladaMesh->getName().c_str();
		meshInfo.mUniqueId = pColladaMesh->getUniqueId().toAscii();

		// positions
		GetFloatOrDouble(meshInfo.mPos, pColladaMesh->getPositions());
		//normals	
		GetFloatOrDouble(meshInfo.mNormals, pColladaMesh->getNormals());
		// uvs
		GetFloatOrDouble(meshInfo.mUVs, pColladaMesh->getUVCoords());

		MeshPrimitiveArray& meshPrimitives = pColladaMesh->getMeshPrimitives();
		size_t pc = meshPrimitives.getCount();
		meshInfo.mHasUVs.assign(pc, false);
		meshInfo.mNumPrimitives = pc;
		for (size_t i = 0; i<pc; i++)
		{
			meshInfo.mPosIndices.push_back(INDICES());
			meshInfo.mNormalIndices.push_back(INDICES());
			meshInfo.mUVIndices.push_back(INDICES());
			INDICES& posIndices = meshInfo.mPosIndices.back();
			INDICES& normalIndices = meshInfo.mNormalIndices.back();
			INDICES& uvIndices = meshInfo.mUVIndices.back();
			MeshPrimitive::PrimitiveType type = meshPrimitives[i]->getPrimitiveType();
			switch (type)
			{
			case MeshPrimitive::POLYGONS:
			case MeshPrimitive::POLYLIST:
			case MeshPrimitive::TRIANGLES:
			{
				UIntValuesArray& pi = meshPrimitives[i]->getPositionIndices();
				UIntValuesArray& ni = meshPrimitives[i]->getNormalIndices();
				size_t ic = pi.getCount();
				size_t ic_n = ni.getCount();
				assert(ic == ic_n);
				posIndices.assign(pi.getData(), pi.getData() + ic);
				normalIndices.assign(ni.getData(), ni.getData() + ic);

				meshInfo.mHasUVs[i] = meshPrimitives[i]->hasUVCoordIndices();
				if (meshInfo.mHasUVs[i])
				{
					UIntValuesArray& ui = meshPrimitives[i]->getUVCoordIndices(0)->getIndices();
					size_t ic_u = ui.getCount();
					assert(ic == ic_u);
					uvIndices.assign(ui.getData(), ui.getData() + ic);
				}
			}
			break;
			default:
				Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Ignored mesh primitive type: %d", type).c_str());
			}

			// material
			if (!mOptions.mMergeMaterialGroups)
			{
				for (size_t i = 0; i<pc; i++)
				{
					std::string file = GetMaterialFilepath(meshPrimitives[i]->getMaterial().c_str());
					if (file.empty())
					{
						file = "EssentialEngineData/materials/missing.material";
					}
					meshInfo.mMaterials.push_back(file);					
				}
			}
		}
		return &meshInfo;
	}

	void GetFloatOrDouble(FLOAT_DATA& dest, COLLADAFW::FloatOrDoubleArray& src){
		using namespace COLLADAFW;
		if (src.getType() == FloatOrDoubleArray::DATA_TYPE_FLOAT)
		{
			COLLADAFW::FloatArray* pFloatArray = src.getFloatValues();
			size_t numFloats = pFloatArray->getCount();
			dest.assign(pFloatArray->getData(), pFloatArray->getData() + numFloats);
		}
		else if (src.getType() == FloatOrDoubleArray::DATA_TYPE_DOUBLE)
		{
			DoubleArray* pDoubleArray = src.getDoubleValues();
			size_t numDoubles = pDoubleArray->getCount();
			dest.clear();
			dest.reserve(numDoubles);
			for (size_t i = 0; i<numDoubles; i++)
			{
				dest.push_back((float)((*pDoubleArray)[i]));
			}
		}
	}

	void AssignProjections(collada::ModelTriangle* pTri, const std::vector<collada::Vec3>& positions)
	{
		collada::Vec3 N;

		N.x = abs(pTri->faceNormal.x);
		N.y = abs(pTri->faceNormal.y);
		N.z = abs(pTri->faceNormal.z);

		if (N.x > N.y)
		{
			if (N.x > N.z)
			{
				// x is the dominant axis
				pTri->dominantAxis = 0;
				pTri->v0Proj.x = positions[pTri->v[0]].y;
				pTri->v0Proj.y = positions[pTri->v[0]].z;
				pTri->v1Proj.x = positions[pTri->v[1]].y;
				pTri->v1Proj.y = positions[pTri->v[1]].z;
				pTri->v2Proj.x = positions[pTri->v[2]].y;
				pTri->v2Proj.y = positions[pTri->v[2]].z;
			}
			else
			{
				// z is the dominant axis
				pTri->dominantAxis = 2;
				pTri->v0Proj.x = positions[pTri->v[0]].x;
				pTri->v0Proj.y = positions[pTri->v[0]].y;
				pTri->v1Proj.x = positions[pTri->v[1]].x;
				pTri->v1Proj.y = positions[pTri->v[1]].y;
				pTri->v2Proj.x = positions[pTri->v[2]].x;
				pTri->v2Proj.y = positions[pTri->v[2]].y;
			}
		}
		else
		{
			if (N.y > N.z)
			{
				// y is the dominant axis
				pTri->dominantAxis = 1;
				pTri->v0Proj.x = positions[pTri->v[0]].x;
				pTri->v0Proj.y = positions[pTri->v[0]].z;
				pTri->v1Proj.x = positions[pTri->v[1]].x;
				pTri->v1Proj.y = positions[pTri->v[1]].z;
				pTri->v2Proj.x = positions[pTri->v[2]].x;
				pTri->v2Proj.y = positions[pTri->v[2]].z;
			}
			else
			{
				// z is the dominant axis
				pTri->dominantAxis = 2;
				pTri->v0Proj.x = positions[pTri->v[0]].x;
				pTri->v0Proj.y = positions[pTri->v[0]].y;
				pTri->v1Proj.x = positions[pTri->v[1]].x;
				pTri->v1Proj.y = positions[pTri->v[1]].y;
				pTri->v2Proj.x = positions[pTri->v[2]].x;
				pTri->v2Proj.y = positions[pTri->v[2]].y;
			}
		}
	}

	collada::MeshPtr FeedGeometry(MeshInfo* meshInfo){
		if (meshInfo->mName.find("_COL_MESH") != std::string::npos)
		{
			return FeedGeometry_Collision(meshInfo);
		}

		if (meshInfo->mPosIndices.empty())
			return 0;

		unsigned elemOffset[] = { 0, 1, 2 };
		if (mOptions.mSwapYZ)
		{
			std::swap(elemOffset[1], elemOffset[2]);
		}
		unsigned indexOffset[] = { 0, 1, 2 };
		if (mOptions.mOppositeCull)
		{
			std::swap(indexOffset[1], indexOffset[2]);
		}

		auto itFind = mMeshObjects.find(meshInfo->mUniqueId);
		assert(itFind == mMeshObjects.end());
		collada::MeshPtr pMeshObject(new collada::Mesh, [](collada::Mesh* obj) { delete obj; });
		mMeshObjects.insert(std::make_pair(meshInfo->mUniqueId, pMeshObject));
		pMeshObject->mName = meshInfo->mName;
		std::vector<collada::Vec3> positions;
		positions.reserve(10000);
		std::vector<collada::Vec3> normals;
		normals.reserve(10000);
		std::vector<collada::Vec2> uvs;
		uvs.reserve(10000);
		std::vector<collada::ModelTriangle> triangles;
		triangles.reserve(3000);

		size_t nextIdx = 0;
		INDICES indices;
		indices.reserve(10000);
		std::unordered_map<collada::DEFAULT_INPUTS::V_PNT, unsigned> vertToIdx;		
		for (int pri = 0; pri<meshInfo->mNumPrimitives; pri++)
		{
			const INDICES& posIndices = meshInfo->mPosIndices[pri];
			const INDICES& norIndices = meshInfo->mNormalIndices[pri];
			const INDICES& uvIndices = meshInfo->mUVIndices[pri];
			size_t numIndices = posIndices.size();
			if (!numIndices)
				continue;
			assert(numIndices % 3 == 0);
			assert(numIndices == norIndices.size());
			assert(uvIndices.size() == 0 || numIndices == uvIndices.size());
			if (mOptions.mUseIndexBuffer)
			{
				for (size_t i = 0; i<numIndices; i += 3)
				{
					collada::ModelTriangle tri;
					for (int k = 0; k<3; k++)
					{
						size_t pi = posIndices[i + indexOffset[k]] * 3;
						size_t ni = norIndices[i + indexOffset[k]] * 3;
						collada::Vec2 uvCoord{ 0, 0 };
						if (meshInfo->mHasUVs[pri])
						{
							size_t ui = uvIndices[i + indexOffset[k]] * 2;
							uvCoord = { meshInfo->mUVs[ui], meshInfo->mUVs[ui + 1] };
						}

						collada::DEFAULT_INPUTS::V_PNT vert(
							collada::Vec3(meshInfo->mPos[pi], meshInfo->mPos[pi + elemOffset[1]], meshInfo->mPos[pi + elemOffset[2]]),
							collada::Vec3(meshInfo->mNormals[ni], meshInfo->mNormals[ni + elemOffset[1]], meshInfo->mNormals[ni + elemOffset[2]]),
							uvCoord);

						auto it = vertToIdx.find(vert);
						if (it != vertToIdx.end()) {
							// existing combination
							indices.push_back(it->second);
						}
						else {
							// new combination
							vertToIdx[vert] = nextIdx;
							indices.push_back(nextIdx++);
							positions.push_back(vert.p);
							normals.push_back(vert.n);
							uvs.push_back(vert.uv);
						}						
					}					
					collada::Vec3 vEdge1 = positions[tri.v[1]] - positions[tri.v[0]];
					collada::Vec3 vEdge2 = positions[tri.v[2]] - positions[tri.v[0]];
					tri.faceNormal = vEdge1.Cross(vEdge2).NormalizeCopy();
					tri.d = tri.faceNormal.Dot(positions[tri.v[0]]);
					AssignProjections(&tri, positions);
					triangles.push_back(tri);
				}
			}
			else // not using index buffer
			{
				for (size_t i = 0; i<numIndices; i += 3)
				{
					collada::ModelTriangle tri;
					for (int k = 0; k<3; k++)
					{
						size_t pi = posIndices[i + indexOffset[k]] * 3;
						size_t ni = norIndices[i + indexOffset[k]] * 3;
						collada::Vec2 uvCoord{ 0, 0 };
						if (meshInfo->mHasUVs[pri])
						{
							size_t ui = uvIndices[i + indexOffset[k]] * 2;
							uvCoord = { meshInfo->mUVs[ui + 0], meshInfo->mUVs[ui + 1] };
						}

						positions.push_back(collada::Vec3(meshInfo->mPos[pi], meshInfo->mPos[pi + elemOffset[1]], meshInfo->mPos[pi + elemOffset[2]]));
						normals.push_back(collada::Vec3(meshInfo->mNormals[ni], meshInfo->mNormals[ni + elemOffset[1]], meshInfo->mNormals[ni + elemOffset[2]]));
						uvs.push_back(uvCoord);

						tri.v[k] = positions.size() - 1;
					}
					collada::Vec3 vEdge1 = positions[tri.v[1]] - positions[tri.v[0]];
					collada::Vec3 vEdge2 = positions[tri.v[2]] - positions[tri.v[0]];
					tri.faceNormal = vEdge1.Cross(vEdge2).NormalizeCopy();
					tri.d = tri.faceNormal.Dot(positions[tri.v[0]]);
					AssignProjections(&tri, positions);
					triangles.push_back(tri);
				}
			} // mUseIndexBuffer

			if (!mOptions.mMergeMaterialGroups)
			{
				if (positions.empty())
				{
					Logger::Log(FB_ERROR_LOG_ARG, "Collada import found a geometry that has no position data.");					
					continue;
				}
				size_t added = positions.size();
				assert(added == normals.size());
				assert(added == uvs.size());
				assert((added / 3 == triangles.size()) || (indices.size()/3 == triangles.size()));


				pMeshObject->mMaterialGroups[pri].mPositions.swap(positions);
				pMeshObject->mMaterialGroups[pri].mNormals.swap(normals);
				pMeshObject->mMaterialGroups[pri].mUVs.swap(uvs);
				pMeshObject->mMaterialGroups[pri].mTriangles.swap(triangles);
				pMeshObject->mMaterialGroups[pri].mIndexBuffer.swap(indices);
				pMeshObject->mMaterialGroups[pri].mMaterialPath = meshInfo->mMaterials[pri];

				// clear for only not merge
				vertToIdx.clear();
				nextIdx = 0;
			} // !mMergeMaterialGroups
		} // pri

		if (mOptions.mMergeMaterialGroups)
		{
			if (positions.empty())
			{
				assert(0);
				return 0;
			}
			size_t added = positions.size();
			assert(added == normals.size());
			assert(added == uvs.size());
			assert((added / 3 == triangles.size()) || (indices.size() / 3 == triangles.size()));
			pMeshObject->mMaterialGroups[0].mPositions.swap(positions);
			pMeshObject->mMaterialGroups[0].mNormals.swap(normals);
			pMeshObject->mMaterialGroups[0].mUVs.swap(uvs);
			pMeshObject->mMaterialGroups[0].mTriangles.swap(triangles);
			pMeshObject->mMaterialGroups[0].mIndexBuffer.swap(indices);
		}

		return pMeshObject;
	}

	collada::MeshPtr FeedGeometry_Collision(MeshInfo* meshInfo){
		if (meshInfo->mPosIndices.empty())
			return 0;

		unsigned elemOffset[] = { 0, 1, 2 };
		if (mOptions.mSwapYZ)
		{
			std::swap(elemOffset[1], elemOffset[2]);
		}
		unsigned indexOffset[] = { 0, 1, 2 };
		if (mOptions.mOppositeCull)
		{
			std::swap(indexOffset[1], indexOffset[2]);
		}

		auto itfind = mCollisionMeshes.find(meshInfo->mUniqueId);
		assert(itfind == mCollisionMeshes.end());
		collada::MeshPtr pMeshObject(new collada::Mesh, [](collada::Mesh* obj){ delete obj; });
		mCollisionMeshes.insert(std::make_pair(meshInfo->mUniqueId, pMeshObject));
		pMeshObject->mName = meshInfo->mName; // this will be overwrtten by the node name		
		std::vector<collada::Vec3> positions;
		positions.reserve(10000);
		for (int pri = 0; pri<meshInfo->mNumPrimitives; pri++)
		{
			const INDICES& posIndices = meshInfo->mPosIndices[pri];
			size_t numIndices = posIndices.size();
			if (!numIndices)
				continue;
			assert(numIndices % 3 == 0);

			for (size_t i = 0; i<numIndices; i += 3)
			{
				for (int k = 0; k<3; k++)
				{
					size_t pi = posIndices[i + indexOffset[k]] * 3;
					positions.push_back(collada::Vec3(meshInfo->mPos[pi], meshInfo->mPos[pi + elemOffset[1]], meshInfo->mPos[pi + elemOffset[2]]));					
				}
			}
		} // pri

		if (positions.empty())
		{
			assert(0);
			return 0;
		}

		pMeshObject->mMaterialGroups[0].mPositions = std::move(positions);
		return pMeshObject;
	}

	collada::ColShape GetColShape(const char* str)
	{
		int len = strlen(str);
		assert(len > 5);
		int start = 5;
		int end = start;
		for (int i = start; i < len; ++i)
		{
			if (str[i] == '_')
			{
				end = i;
				break;
			}
		}
		if (start == end)
			end = len;

		std::string typestring(str, start, end - start);
		ToLowerCase(typestring);
		return collada::ConvertColShapeStringToEnum(typestring.c_str());
	}

	collada::Vec4 ConvertData(const COLLADABU::Math::Quaternion& src){
		return collada::Vec4((float)src.x, (float)src.y, (float)src.z, (float)src.w);
	}
	collada::Vec3 ConvertData(const COLLADABU::Math::Vector3& src){
		return collada::Vec3((float)src.x, (float)src.y, (float)src.z);
	}
	void WriteChildNode(const COLLADAFW::Node* node, size_t parentMeshIdx){
		using namespace COLLADAFW;

		std::string name = node->getName();
		COLLADABU::Math::Matrix4 mat = node->getTransformationMatrix();
		COLLADABU::Math::Vector3 scale = mat.getScale();
		COLLADABU::Math::Quaternion rot = mat.extractQuaternion();
		rot.normalise();
		COLLADABU::Math::Vector3 trans = mat.getTrans();
		collada::Location location;
		location.mPos = ConvertData(trans);
		location.mScale = ConvertData(scale);
		location.mQuat = ConvertData(rot);		
		bool auxiliaryNode = name.find("_POS") == 0;
		bool collisionNode = name.find("_COL") == 0;
		bool cameraNode = name.find("_CAM") == 0;
		bool parsingChildMesh = parentMeshIdx != -1;
		if (auxiliaryNode)
		{
			assert(mMeshGroup->mMeshes[parentMeshIdx].mMesh);
			assert(parsingChildMesh);
			mMeshGroup->mMeshes[parentMeshIdx].mMesh->mAuxiliaries.push_back(collada::AUXILIARIES::value_type(name, location));
		}
		else if (collisionNode)
		{
			collada::CollisionInfo colInfo(GetColShape(name.c_str()), location, 0);
			if (mOptions.mUseMeshGroup)
			{
				assert(mMeshGroup->mMeshes[parentMeshIdx].mMesh);
				assert(parsingChildMesh);
				mMeshGroup->mMeshes[parentMeshIdx].mMesh->mCollisionInfo.push_back(colInfo);
			}
			else
			{
				assert(!mMeshObjects.empty());
				mMeshObjects.begin()->second->mCollisionInfo.push_back(colInfo);				
			}
		}
		else if (cameraNode){
			collada::CameraInfo camInfo(name, location);
			const auto& instance_cameras = node->getInstanceCameras();
			auto numCameras = instance_cameras.getCount();
			assert(numCameras == 1); // currently support only one
			auto strCameraId = instance_cameras[0]->getInstanciatedObjectId().toAscii();
			camInfo.mData = GetCameraData(strCameraId.c_str());			 
			if (mOptions.mUseMeshGroup){
				assert(mMeshGroup->mMeshes[parentMeshIdx].mMesh);
				assert(parsingChildMesh);
				auto& camInfos = mMeshGroup->mMeshes[parentMeshIdx].mMesh->mCameraInfo;
				assert(camInfos.find(name) == camInfos.end());
				camInfos.insert(std::make_pair(name, camInfo));
			}
			else{
				assert(!mMeshObjects.empty());
				auto& camInfos = mMeshObjects.begin()->second->mCameraInfo;
				assert(camInfos.find(name) == camInfos.end());
				camInfos.insert(std::make_pair(name, camInfo));
			}
		}

		const InstanceGeometryPointerArray& ga = node->getInstanceGeometries();
		size_t gaCount = ga.getCount();
		assert(gaCount <= 1);
		// Currently the engine doesn't support serveral geometries in one node.
		// So we are assuming one node have one mesh. This has no problem for now.
		size_t meshIdx = -1;
		if (gaCount > 0)
		{
			std::string id = ga[0]->getInstanciatedObjectId().toAscii();
			collada::MeshPtr pMeshObject = GetMeshObject(id.c_str());
			if (pMeshObject)
			{
				// write node name
				pMeshObject->mName = name;
				if (parsingChildMesh)
				{
					meshIdx = mMeshGroup->mMeshes.size();
					mMeshGroup->mMeshes[meshIdx].mParentMeshIdx = parentMeshIdx;
					mMeshGroup->mMeshes[meshIdx].mMesh = pMeshObject;
					mMeshGroup->mMeshes[meshIdx].mTransformation = location;

					bool partMesh = name.find("_PART") == 0;
					if (partMesh)
					{
						collada::AUXILIARIES aux;
						aux.push_back(collada::AUXILIARIES::value_type(name, location));
						mMeshGroup->mMeshes[meshIdx].mMesh->mAuxiliaries = aux;						
					}
				}
				else if (collisionNode) // This collision has seperated mesh object for physics.
				{
					assert(!mMeshObjects.empty());
					mMeshObjects.begin()->second->mCollisionInfo.back().mCollisionMesh = pMeshObject;	
				}				
			}
		}

		if (meshIdx != -1)
		{
			const NodePointerArray& na = node->getChildNodes();
			size_t naCount = na.getCount();
			for (size_t n = 0; n< naCount; n++)
			{
				WriteChildNode(na[n], meshIdx);
			}
		}
	}

	/** This method will be called if an error in the loading process occurred and the loader cannot
	continue to to load. The writer should undo all operations that have been performed.
	@param errorMessage A message containing informations about the error that occurred.
	*/
	void cancel(const COLLADAFW::String& errorMessage){

	}

	/** This is the method called. The writer hast to prepare to receive data.*/
	void start(){

	}

	/** This method is called after the last write* method. No other methods will be called after this.*/
	void finish(){

	}

	/** When this method is called, the writer must write the global document asset.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeGlobalAsset(const COLLADAFW::FileInfo* asset){
		return true;
	}

	/** When this method is called, the writer must write the scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeScene(const COLLADAFW::Scene* scene){
		return true;
	}

	/** When this method is called, the writer must write the entire visual scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeVisualScene(const COLLADAFW::VisualScene* visualScene){
		if (mOptions.mUseMeshGroup)
			mMeshGroup = collada::MeshGroupPtr(new collada::MeshGroup, [](collada::MeshGroup* obj) { delete obj; });

		using namespace COLLADAFW;
		const NodePointerArray& node = visualScene->getRootNodes();
		size_t count = node.getCount();
		for (size_t i = 0; i<count; i++)
		{
			std::string name = node[i]->getName();
			COLLADABU::Math::Matrix4 mat = node[i]->getTransformationMatrix();
			COLLADABU::Math::Vector3 scale = mat.getScale();
			COLLADABU::Math::Quaternion rot = mat.extractQuaternion();
			rot.normalise();
			COLLADABU::Math::Vector3 trans = mat.getTrans();
			collada::Location location;
			location.mPos = ConvertData(trans);
			location.mScale = ConvertData(scale);
			location.mQuat = ConvertData(rot);
			size_t idx = -1;
			if (mOptions.mUseMeshGroup)
			{
				const InstanceGeometryPointerArray& ga = node[i]->getInstanceGeometries();
				size_t gaCount = ga.getCount();
				assert(gaCount <= 1);// this is temporary.
				// Currently the engine doesn't support serveral geometries in one node.
				// So we are assuming one node have one mesh. This has no problem for now.
				if (gaCount > 0)
				{
					std::string id = ga[0]->getInstanciatedObjectId().toAscii();
					auto pMeshObject = GetMeshObject(id.c_str());
					if (pMeshObject)
					{
						// node name;
						pMeshObject->mName = name; 
						idx = mMeshGroup->mMeshes.size();
						mMeshGroup->mMeshes[idx].mMesh = pMeshObject;
						mMeshGroup->mMeshes[idx].mParentMeshIdx = -1;
						mMeshGroup->mMeshes[idx].mTransformation = location;
						if (name.find("_PART") == 0)
						{
							collada::AUXILIARIES aux;
							aux.push_back(collada::AUXILIARIES::value_type(name, location));
							mMeshGroup->mAuxiliaries = aux;
						}
					}
				}
			}
			else
			{
				const InstanceGeometryPointerArray& ga = node[i]->getInstanceGeometries();
				size_t gaCount = ga.getCount();
				for (size_t g = 0; g < gaCount; g++)
				{
					std::string id = ga[g]->getInstanciatedObjectId().toAscii();
					auto pMeshObject = GetMeshObject(id.c_str());
					if (pMeshObject)
					{
						pMeshObject->mName = name;
					}
				}
			}

			// auxiliaries
			bool auxiliaryNode = name.find("_POS") == 0;
			bool collisionNode = name.find("_COL") == 0;
			bool cameraNode = name.find("_CAM") == 0;
			if (auxiliaryNode)
			{
				if (mOptions.mUseMeshGroup){
					mMeshGroup->mAuxiliaries.push_back(collada::AUXILIARIES::value_type(name, location));
				}
				else{
					mMeshObjects.begin()->second->mAuxiliaries.push_back(collada::AUXILIARIES::value_type(name, location));
				}
			}
			else if (collisionNode)
			{
				auto shape = GetColShape(name.c_str());
				if (mOptions.mUseMeshGroup){
					mMeshGroup->mCollisionInfo.push_back(collada::CollisionInfo(shape, location, 0));
				}
				else{
					mMeshObjects.begin()->second->mCollisionInfo.push_back(collada::CollisionInfo(shape, location, 0));
				}
			}
			else if (cameraNode){
				collada::CameraInfo camInfo(name, location);				
				const auto& instance_cameras = node[i]->getInstanceCameras();
				auto numCameras = instance_cameras.getCount();
				assert(numCameras == 1); // currently support only one
				auto strCameraId = instance_cameras[0]->getInstanciatedObjectId().toAscii();
				camInfo.mData = GetCameraData(strCameraId.c_str());
				if (mOptions.mUseMeshGroup){
					auto& camInfos = mMeshGroup->mCameraInfo;
					assert(camInfos.find(name) == camInfos.end());
					camInfos.insert(std::make_pair(name, camInfo));
				}
				else{
					assert(!mMeshObjects.empty());
					auto& camInfos = mMeshObjects.begin()->second->mCameraInfo;
					assert(camInfos.find(name) == camInfos.end());
					camInfos.insert(std::make_pair(name, camInfo));
				}
			}


			const NodePointerArray& na = node[i]->getChildNodes();
			size_t naCount = na.getCount();
			for (size_t n = 0; n< naCount; n++)
			{
				WriteChildNode(na[n], idx);
			}
		}
		return true;
	}

	/** When this method is called, the writer must handle all nodes contained in the
	library nodes.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeLibraryNodes(const COLLADAFW::LibraryNodes* libraryNodes){
		return true;
	}

	/** When this method is called, the writer must write the geometry.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeGeometry(const COLLADAFW::Geometry* geometry){
		using namespace COLLADAFW;
		Geometry::GeometryType type = geometry->getType();
		if (type == Geometry::GEO_TYPE_MESH || type == Geometry::GEO_TYPE_CONVEX_MESH)
		{
			auto pColladaMesh = dynamic_cast<COLLADAFW::Mesh*>(const_cast<Geometry*>(geometry));
			if (pColladaMesh)
			{
				auto meshInfo = CopyData(pColladaMesh);
				FeedGeometry(meshInfo);
			}
		}
		return true;
	}

	/** When this method is called, the writer must write the material.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeMaterial(const COLLADAFW::Material* material){
		return true;
	}

	/** When this method is called, the writer must write the effect.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeEffect(const COLLADAFW::Effect* effect){
		return true;
	}

	/** When this method is called, the writer must write the camera.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeCamera(const COLLADAFW::Camera* camera){
		auto uniqueId = camera->getUniqueId().toAscii();
		collada::CameraData data;
		data.mAspectRatio = (float)camera->getAspectRatio();
		data.mNear = (float)camera->getNearClippingPlane();
		data.mFar = (float)camera->getFarClippingPlane();
		data.mXFov = (float)camera->getXFov();
		data.mYFov = (float)camera->getYFov();
		data.mType = (float)camera->getCameraType() == COLLADAFW::Camera::ORTHOGRAPHIC ?
			collada::CameraData::Orthogonal : 
			collada::CameraData::Perspective;

		mCameraDatas.insert(std::make_pair(uniqueId, data));
		return true;
	}

	/** When this method is called, the writer must write the image.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeImage(const COLLADAFW::Image* image){
		return true;
	}

	/** When this method is called, the writer must write the light.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeLight(const COLLADAFW::Light* light){
		return true;
	}

	float Radian(float degree)
	{
		return degree / 180.0f * PI;
	}

	/** When this method is called, the writer must write the Animation.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeAnimation(const COLLADAFW::Animation* animation){
		auto type = animation->getAnimationType();
		assert(type == COLLADAFW::Animation::ANIMATION_CURVE);
		auto uniqueId = animation->getUniqueId();
		COLLADAFW::AnimationCurve* animationCurve = (COLLADAFW::AnimationCurve*)animation;
		auto dimention = animationCurve->getOutDimension();
		assert(dimention == 1);
		auto keyCount = animationCurve->getKeyCount();
		auto& inputValues = animationCurve->getInputValues();
		auto& outputValues = animationCurve->getOutputValues();
		FLOAT_DATA inputData, outputData;
		GetFloatOrDouble(inputData, inputValues);
		GetFloatOrDouble(outputData, outputValues);
		auto originalId = animation->getOriginalId();
		StringVector ids;
		std::string animName;
		if (originalId.find("_PART_") != std::string::npos)
		{
			ids = Split(&originalId[6], "_");
			animName.assign(originalId.begin(), originalId.begin() + 6);
			animName += ids[0];
		}
		else
		{
			ids = Split(originalId, "_");
			animName = ids[0];
		}

		int animationType = -1; // 0 = location, 1 = rotation, 2 = scale
		AnimationData::PosComp component;
		if (ids[1] == "location")
		{
			animationType = 0;
			if (ids[2] == "X")
				component = AnimationData::X;
			else if (ids[2] == "Y")
				component = AnimationData::Y;
			else if (ids[2] == "Z")
				component = AnimationData::Z;
			else
			{
				assert(0);
				return true;
			}
		}
		else if (ids[1] == "rotation")
		{
			animationType = 1;
			assert(ids[2] == "euler");
			if (ids[3] == "X")
				component = AnimationData::X;
			else if (ids[3] == "Y")
				component = AnimationData::Y;
			else if (ids[3] == "Z")
				component = AnimationData::Z;
			else
			{
				assert(0);
				return true;
			}
		}
		else if (ids[1] == "scale")
		{
			animationType = 2;
			if (ids[2] == "X")
				component = AnimationData::X;
			else if (ids[2] == "Y")
				component = AnimationData::Y;
			else if (ids[2] == "Z")
				component = AnimationData::Z;
			else
			{
				assert(0);
				return true;
			}
		}
		else
		{
			assert(0);
			return true;
		}

		auto it = mAnimData.find(animName);
		if (it == mAnimData.end()){
			mAnimData.insert({ animName, AnimationData::Create() });
		}
		for (size_t i = 0; i < keyCount; ++i)
		{
			switch (animationType)
			{
			case 0: // position
				mAnimData[animName]->AddPosition(inputData[i], outputData[i], component);
				break;

			case 1: // rotation
				mAnimData[animName]->AddRotEuler(inputData[i], Radian(outputData[i]), component);
				break;

			case 2: // scale
				// not support for now.
				//mAnimData[ids[0]].AddScale(inputData[i], outputData[i], component);
				break;
			}
		}

		return true;
	}

	/** When this method is called, the writer must write the AnimationList.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeAnimationList(const COLLADAFW::AnimationList* animationList){
		const auto& bindings = animationList->getAnimationBindings();
		auto numAnimationBindings = bindings.getCount();
		for (unsigned i = 0; i < numAnimationBindings; i++)
		{
			auto binding = bindings.getData() + 1;
		}
		return true;
	}

	/** When this method is called, the writer must write the skin controller data.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeSkinControllerData(const COLLADAFW::SkinControllerData* skinControllerData){
		return true;
	}

	/** When this method is called, the writer must write the controller.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeController(const COLLADAFW::Controller* controller){
		return true;
	}

	/** When this method is called, the writer must write the formulas. All the formulas of the entire
	COLLADA file are contained in @a formulas.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeFormulas(const COLLADAFW::Formulas* formulas){
		return true;
	}

	/** When this method is called, the writer must write the kinematics scene.
	@return The writer should return true, if writing succeeded, false otherwise.*/
	bool writeKinematicsScene(const COLLADAFW::KinematicsScene* kinematicsScene){
		return true;
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(ColladaImporter);
void ColladaImporter::CleanUP(){
	xmlCleanupParser();
}

ColladaImporter::ColladaImporter()
	: mImpl(new Impl(this))
{

}

ColladaImporter::~ColladaImporter(){

}

bool ColladaImporter::ImportCollada(const char* filepath) {
	return mImpl->ImportCollada(filepath);
}

bool ColladaImporter::ImportCollada(const char* filepath, const ImportOptions& options) {
	return mImpl->ImportCollada(filepath, options);
}

collada::MeshPtr ColladaImporter::GetMeshObject() const {
	return mImpl->GetMeshObject();
}

collada::MeshGroupPtr ColladaImporter::GetMeshGroup() const {
	return mImpl->GetMeshGroup();
}

IteratorWrapper<ColladaMeshObjects> ColladaImporter::GetMeshIterator(){
	return mImpl->GetMeshIterator();
}

unsigned ColladaImporter::GetNumMeshes() const {
	return mImpl->GetNumMeshes();
}

}
