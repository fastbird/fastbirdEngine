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

#pragma once
#include <boost/config.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/tracking.hpp>
#include "FBCommonHeaders/Types.h"
#include "FBCommonHeaders/Helpers.h"
#include "FBCommonHeaders/VectorMap.h"
#include "FBAnimation/AnimationData.h"
#include <string>
#include <memory>
namespace fb{
	namespace collada{
		typedef std::vector<unsigned> IndexBuffer;
		enum ColShape : int
		{
			ColShapeSphere,
			ColShapeCube,
			ColShapeMesh,
			ColShapeNum
		};
		/// \a colShape is lower case
		ColShape ConvertColShapeStringToEnum(const char* colShape);
		
		struct Vec2{
			float x, y;

			bool IsEqual(const Vec2& other, float epsilon = 0.0005f) const {
				return !(abs(other.x - x) > epsilon || abs(other.y - y) > epsilon);
			}

			size_t Hash() const {
				return hash_combine_ret(std::hash<float>()(x), std::hash<float>()(y));				
			}
		};

		struct Vec3{
			float x, y, z;

			Vec3(){

			}
			Vec3(float x_, float y_, float z_)
				:x(x_), y(y_), z(z_)
			{
			}

			bool operator==(const Vec3& other) {
				return !(x != other.x || y != other.y || z != other.z);
			}

			bool IsEqual(const Vec3& other, float epsilon = 0.0005f) const {
				return !(abs(other.x - x) > epsilon || abs(other.y - y) > epsilon || abs(other.z - z) > epsilon);
			}

			size_t Hash() const {
				auto h = std::hash<float>()(x);
				hash_combine(h, std::hash<float>()(y));
				return hash_combine_ret(h, std::hash<float>()(z));
			}

			Vec3 operator- (const Vec3& r) const;
			Vec3 Cross(const Vec3& rVector) const;
			float Dot(const Vec3& vec) const;
			float Normalize();
			Vec3 NormalizeCopy() const;
		};

		struct Vec4{
			float x, y, z, w;

			Vec4(){
			}

			Vec4(float x_, float y_, float z_, float w_)
				: x(x_), y(y_), z(z_), w(w_)
			{
			}
		};

		struct ModelTriangle {
			unsigned        v[3];
			// cached data for optimized ray-triangle intersections
			Vec2   v0Proj;           // 2D projection of vertices along the dominant axis
			Vec2   v1Proj;
			Vec2   v2Proj;
			Vec3   faceNormal;
			float  d;                // distance from triangle plane to origin
			int    dominantAxis;     // dominant axis of the triangle plane

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & v;
				ar & v0Proj & v1Proj & v2Proj;
				ar & faceNormal;
				ar & d & dominantAxis;
			}
		};

		struct MaterialGroup{
			std::string mMaterialPath;
			IndexBuffer mIndexBuffer;
			std::vector<Vec3> mPositions;
			std::vector<Vec3> mNormals;
			std::vector<Vec2> mUVs;
			std::vector<ModelTriangle> mTriangles;
			std::vector<Vec3> mTangents;
			
		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mMaterialPath;
				ar & mIndexBuffer;
				ar & mPositions;
				ar & mNormals;
				ar & mUVs;
				ar & mTriangles;
				ar & mTangents;
			}
		};

		struct Mesh;
		typedef std::shared_ptr<Mesh> MeshPtr;

		struct Location{
			Vec3 mScale;
			Vec4 mQuat; // w, x, y, z order.
			Vec3 mPos;
		};

		struct CollisionInfo
		{
			ColShape mColShapeType;
			Location mTransform;
			MeshPtr mCollisionMesh;

			CollisionInfo(){

			}

			CollisionInfo(ColShape type, const Location& transform, MeshPtr colMesh)
				:mColShapeType(type), mTransform(transform), mCollisionMesh(colMesh)
			{

			}

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mColShapeType;
				ar & mTransform;
				if (Archive::is_saving()) {
					bool exist = mCollisionMesh != nullptr;
					ar & exist;
					if (exist)
						ar & (*mCollisionMesh);
				}
				else {
					bool exist;
					ar & exist;
					if (exist) {
						mCollisionMesh = std::make_shared<Mesh>();
						ar & (*mCollisionMesh);
					}
				}
			}
		};
		typedef std::vector< CollisionInfo > COLLISION_INFOS;

		
		typedef std::vector< std::pair<std::string, Location> > AUXILIARIES;

		struct CameraData{
			float mXFov;
			float mYFov;
			float mAspectRatio;
			float mNear;
			float mFar;

			enum Type
			{
				Perspective,
				Orthogonal,
			};
			Type mType;

			CameraData()
				: mXFov(1.5708)
				, mYFov(1.5708)
				, mAspectRatio(1.f)
				, mNear(0.1f)
				, mFar(1000.f)
				, mType(Perspective)
			{
			}

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mXFov;
				ar & mYFov;
				ar & mAspectRatio;
				ar & mNear;
				ar & mFar;
			}
		};

		// actual camera data.
		typedef std::unordered_map<std::string, CameraData> CAMERA_DATAS;

		struct CameraInfo{
			std::string mName;
			Location mLocation;
			CameraData mData;
			CameraInfo()				
			{
			}

			CameraInfo(std::string name, const Location& l)
				: mName(name), mLocation(l)				
			{

			}

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mName;
				ar & mLocation;
				ar & mData;
			}
		};

		// node camera combination
		typedef std::unordered_map<std::string, CameraInfo> CAMERA_INFOS;

		struct Mesh{
			std::string mName;
			std::map<int, MaterialGroup> mMaterialGroups;			
			// Animation Data is vaild while the ColladaImporter is alive.
			AnimationDataPtr mAnimationData;
			AUXILIARIES mAuxiliaries;
			COLLISION_INFOS mCollisionInfo;
			CAMERA_INFOS mCameraInfo;

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mName;
				ar & mMaterialGroups;
				if (Archive::is_saving()) {
					bool has_anim_data = mAnimationData != nullptr;
					ar & has_anim_data;
					if (has_anim_data)
						ar & *mAnimationData;
				}
				else {
					bool has_anim_data;
					ar & has_anim_data;
					if (has_anim_data) {
						mAnimationData = AnimationData::Create();
						ar & *mAnimationData;
					}
				}
				ar & mAuxiliaries;
				ar & mCollisionInfo;
				ar & mCameraInfo;
			}
		};

		struct MeshGroup
		{
			struct Data{
				int mParentMeshIdx;
				MeshPtr mMesh;
				Location mTransformation; // In Local Space(Parent Space)				

			private:
				friend class boost::serialization::access;
				template<class Archive>
				void serialize(Archive & ar, const unsigned int version)
				{
					ar & mParentMeshIdx;
					if (Archive::is_saving()) {
						ar & (*mMesh);
					}
					else {
						mMesh = std::make_shared<Mesh>();
						ar & (*mMesh);
					}
					ar & mTransformation;
				}
			};

			std::map<int, Data> mMeshes;
			AUXILIARIES mAuxiliaries;			
			COLLISION_INFOS mCollisionInfo;
			CAMERA_INFOS mCameraInfo;

		private:
			friend class boost::serialization::access;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version)
			{
				ar & mMeshes;
				ar & mAuxiliaries;
				ar & mCollisionInfo;
				ar & mCameraInfo;
			}
		};

		typedef std::shared_ptr<MeshGroup> MeshGroupPtr;		
		namespace DEFAULT_INPUTS{
			struct POSITION_NORMAL_TEXCOORD_V
			{
				POSITION_NORMAL_TEXCOORD_V() {}
				POSITION_NORMAL_TEXCOORD_V(const Vec3& _p, const Vec3& _n,
					const Vec2 _uv)
					: p(_p), n(_n), uv(_uv){}

				size_t Hash() const {
					auto h = p.Hash();
					hash_combine(h, n.Hash());
					return hash_combine_ret(h, uv.Hash());
				}

				bool operator==(const POSITION_NORMAL_TEXCOORD_V& other) const {
					return Hash() == other.Hash();
				}

				Vec3 p;	// 12
				Vec3 n;	// 12
				Vec2 uv;	// 8
			};
			typedef POSITION_NORMAL_TEXCOORD_V V_PNT;
		}
	}	
}

BOOST_CLASS_IMPLEMENTATION(fb::collada::Vec2, boost::serialization::primitive_type);
BOOST_CLASS_IMPLEMENTATION(fb::collada::Vec3, boost::serialization::primitive_type);
BOOST_CLASS_IMPLEMENTATION(fb::collada::Vec4, boost::serialization::primitive_type);
BOOST_CLASS_IMPLEMENTATION(fb::collada::Location, boost::serialization::primitive_type);

namespace std {
	template<>
	struct hash<fb::collada::DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD_V>
		: public _Bitwise_hash<fb::collada::DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD_V>
	{
		typedef fb::collada::DEFAULT_INPUTS::POSITION_NORMAL_TEXCOORD_V _Kty;
		typedef _Bitwise_hash<_Kty> _Mybase;

		size_t operator()(const _Kty& _Keyval) const
		{
			return _Keyval.Hash();
		}
	};
}