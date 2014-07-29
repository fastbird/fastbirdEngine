#pragma once
#include <Engine/SceneGraph/SpatialObject.h>
namespace fastbird
{
	class IParticleEmitter : public SpatialObject
	{
	public:
		virtual IObject* Clone() const{assert(0); return 0;}
		virtual unsigned GetEmitterID() const = 0;
		virtual bool Load(const char* filepath) = 0;
		virtual bool Update(float elapsedTime)= 0;
		virtual void Active(bool a) = 0;
		virtual void Stop() = 0;
	private:
		friend class ParticleManager;
		static IParticleEmitter* CreateParticleEmitter();
	};
}