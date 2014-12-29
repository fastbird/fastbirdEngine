#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
namespace fastbird
{
	typedef std::vector< std::pair<std::string, Transformation> > AUXILIARIES;
	class IMeshObject;
	class IMeshGroup : public SpatialObject
	{
	public:
		virtual IObject* Clone() const{assert(0); return 0;}
		virtual size_t AddMesh(IMeshObject* mesh, const Transformation& transform, size_t parent) = 0;
		virtual const char* GetNameOfMesh(size_t idx) = 0;
		virtual size_t GetNumMeshes() const = 0;
		
		virtual void AddMeshRotation(size_t idx, const Quat& rot) = 0;
		virtual const Quat& GetMeshRotation(size_t idx) const = 0;
		virtual void SetMeshRotation(size_t idx, const Quat& rot) = 0;
		virtual const Vec3& GetMeshOffset(size_t idx) const = 0;
		
		
		virtual const AUXILIARIES* GetAuxiliaries(size_t idx) const = 0;
		virtual void SetAuxiliaries(size_t idx, const AUXILIARIES& aux) = 0;
		virtual void AddAuxiliary(size_t idx, const AUXILIARIES::value_type& v) = 0;
		virtual void AddCollisionShape(size_t idx, std::pair<ColShape::Enum, Transformation>& data) = 0;

		virtual void UpdateTransform(bool force = false) = 0;

		virtual void SetAnimationData(const char* meshName, const AnimationData& anim, const char* actionName) = 0;
		virtual void PlayAction(const std::string& name, bool immediate, bool reverse) = 0;
		virtual bool IsActionDone(const char* action) const = 0;

	private:
		friend class Engine;
		virtual void Delete() = 0;
	};
}