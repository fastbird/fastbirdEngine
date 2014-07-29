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
		virtual void RotateMesh(size_t idx, const Quat& rot) = 0;
		virtual const Quat& GetRotation(size_t idx) const = 0;
		virtual const Vec3& GetOffset(size_t idx) const = 0;
		virtual const AUXILIARIES& GetAuxiliaries() const = 0;
		virtual void SetAuxiliaries(const AUXILIARIES& aux) = 0;
	};
}