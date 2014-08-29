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
		virtual void SetEnableHighlight(bool enable);

		virtual IMaterial* GetMaterial(int pass = 0) const;

		// order of inserting meshes is important. parent first.
		// transform : parent space
		virtual size_t AddMesh(IMeshObject* mesh, const Transformation& transform, size_t parent);
		virtual const char* GetNameOfMesh(size_t idx);
		virtual size_t GetNumMeshes() const;

		virtual void AddMeshRotation(size_t idx, const Quat& rot);
		virtual const Quat& GetMeshRotation(size_t idx) const;
		virtual void SetMeshRotation(size_t idx, const Quat& rot);
		virtual const Vec3& GetMeshOffset(size_t idx) const;

		virtual const AUXILIARIES* GetAuxiliaries(size_t idx) const;
		virtual void SetAuxiliaries(size_t idx, const AUXILIARIES& aux);
		virtual void AddAuxiliary(size_t idx, const AUXILIARIES::value_type& v);
		virtual void UpdateTransform(bool force = false);

		struct Hierarchy
		{
			size_t mParentIndex;
			size_t mMyIndex;
			std::vector<size_t> mChildren;
		};

	private:
		friend class Engine;
		virtual void Delete();

	private:
		typedef std::vector< std::pair< SmartPtr<IMeshObject>, Transformation> > MESH_OBJECTS;		
		MESH_OBJECTS mMeshObjects;
		typedef std::vector< Transformation > LOCAL_TRANSFORMATIONS;
		LOCAL_TRANSFORMATIONS mLocalTransforms;
		typedef std::vector< bool > CHANGES;
		CHANGES mChanges;
		typedef std::map<size_t, Hierarchy> HIERARCHY_MAP;
		HIERARCHY_MAP mHierarchyMap;

		typedef VectorMap< size_t, AUXILIARIES > AUXIL_MAP;
		AUXIL_MAP mAuxil;
		const AUXIL_MAP* mAuxCloned;
		Timer::FRAME_PRECISION mLastUpdateFrame;

	};
}