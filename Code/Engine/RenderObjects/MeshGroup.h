#pragma once
#include <Engine/IMeshGroup.h>
namespace fastbird
{
	class MeshGroup : public IMeshGroup
	{
	public:
		MeshGroup();
		virtual ~MeshGroup();
		
		virtual IObject* Clone() const;
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();

		// order of inserting meshes is important. parent first.
		// transform : parent space
		virtual size_t AddMesh(IMeshObject* mesh, const Transformation& transform, size_t parent);
		virtual const char* GetNameOfMesh(size_t idx);
		virtual void RotateMesh(size_t idx, const Quat& rot);
		virtual const Quat& GetRotation(size_t idx) const;
		virtual const Vec3& GetOffset(size_t idx) const;
		virtual const AUXILIARIES& GetAuxiliaries() const { return mAuxCloned ? *mAuxCloned : mAuxil; }
		virtual void SetAuxiliaries(const AUXILIARIES& aux) {mAuxil = aux; }

		struct Hierarchy
		{
			size_t mParentIndex;
			size_t mMyIndex;
			std::vector<size_t> mChildren;
		};

	private:
		typedef std::vector< std::pair< SmartPtr<IMeshObject>, Transformation> > MESH_OBJECTS;		
		MESH_OBJECTS mMeshObjects;
		typedef std::vector< Transformation > LOCAL_TRANSFORMATIONS;
		LOCAL_TRANSFORMATIONS mLocalTransforms;
		typedef std::vector< bool > CHANGES;
		CHANGES mChanges;
		typedef std::map<size_t, Hierarchy> HIERARCHY_MAP;
		HIERARCHY_MAP mHierarchyMap;
		AUXILIARIES mAuxil;
		AUXILIARIES* mAuxCloned;

	};
}