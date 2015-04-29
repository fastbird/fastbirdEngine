#pragma once

class btTriangleMesh;
namespace fastbird
{
	namespace CollisionShapes
	{
		enum Enum {
			Box,
			Sphere,
			Cylinder,
			Capsule,
			StaticMesh,
			DynamicMesh,
			Convex,

			Num,
		};

		static const char* strings[] = {
			"Box",
			"Sphere",
			"Cylinder",
			"Capsule",
			"StaticMesh",
			"DynamicMesh",
			"Convex",

			"Num",
		};

		inline const char* ConvertToString(Enum a)
		{
			assert(a >= 0 && a <= Num);
			return strings[a];
		}

		inline Enum ConverToEnum(const char* str)
		{
			for (int i = 0; i < Num; i++)
			{
				if (_stricmp(str, strings[i]) == 0)
					return Enum(i);
			}

			return Num;
		}
	}

	struct CollisionShape
	{
		CollisionShape()
		:mPos(0, 0, 0), mUserPtr(0)
		, mScale(1, 1, 1)
		{
			
		}

		virtual void ChangeScale(const Vec3& scale){}
		virtual ~CollisionShape(){}
		CollisionShapes::Enum mType;
		Vec3 mPos;
		Quat mRot;
		Vec3 mScale;
		void* mUserPtr;
	};

	struct BoxShape : public CollisionShape
	{
		virtual void ChangeScale(const Vec3& scale)
		{
			mScale = scale;
		}
		Vec3 mExtent;		
	};

	struct SphereShape : public CollisionShape
	{
		virtual void ChangeScale(const Vec3& scale)
		{
			mScale = scale;
		}
		float mRadius;
	};

	struct CylinderShape : public CollisionShape
	{
		virtual void ChangeScale(const Vec3& scale)
		{
			mScale = scale;
		}
		Vec3 mExtent;
	};

	struct CapsuleShape : public CollisionShape
	{
		virtual void ChangeScale(const Vec3& scale)
		{
			mRadius *= scale.x;
			mHeight *= scale.y;
		}
		float mRadius;
		float mHeight;
	};

	struct MeshShape : public CollisionShape
	{
		MeshShape()
		:mTriangleMesh(0), mNumVertices(0)
		{

		}
		~MeshShape();
		
		virtual void ChangeScale(const Vec3& scale);

		Vec3* mVertices;
		unsigned mNumVertices;
		btTriangleMesh* GetTriangleMesh();

	private:
		void CreateTriangleMesh();
		btTriangleMesh* mTriangleMesh;
	};

	class CLASS_DECLSPEC_PHYSICS CollisionShapeMan
	{
	public:
		static BoxShape* CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale,const Vec3& extent, void* userPtr = 0);
		static SphereShape* CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr = 0);
		static CylinderShape* CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		static CapsuleShape* CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr = 0);
		static MeshShape* CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
			bool staticObj, void* userPtr = 0);
		static MeshShape* CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, 
			const Vec3& scale, void* userPtr = 0);
		static void DestroyShape(CollisionShape* shape);
	};
}