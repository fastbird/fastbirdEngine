#pragma once
namespace fb {
	FB_DECLARE_SMART_PTR_STRUCT(BoxShape);
	FB_DECLARE_SMART_PTR_STRUCT(SphereShape);
	FB_DECLARE_SMART_PTR_STRUCT(CylinderShape);
	FB_DECLARE_SMART_PTR_STRUCT(CapsuleShape);
	FB_DECLARE_SMART_PTR_STRUCT(MeshShape);

	class FB_DLL_PHYSICS CollisionShapeFactory
	{
	public:
		static BoxShapePtr CreateBoxShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		static SphereShapePtr CreateSphereShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, void* userPtr = 0);
		static CylinderShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, const Vec3& extent, void* userPtr = 0);
		static CapsuleShapePtr CreateCylinderShape(const Vec3& pos, const Quat& rot, const Vec3& actorScale, float radius, float height, void* userPtr = 0);
		static MeshShapePtr CreateMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices, const Vec3& scale,
			bool staticObj, void* userPtr = 0);
		static MeshShapePtr CreateConvexMeshShape(const Vec3& pos, const Quat& rot, Vec3* vertices, unsigned numVertices,
			const Vec3& scale, void* userPtr = 0);
	};
}
