#pragma once

namespace fastbird
{
class IParticleEmitter;
class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	static void InitializeParticleManager();
	static ParticleManager& GetParticleManager();
	static void FinalizeParticleManager();

	void Update(float elapsedTime);

	IParticleEmitter* GetParticleEmitter(const char* file);
	IParticleEmitter* GetParticleEmitter(unsigned id);

private:
	friend class ParticleEmitter;
	void AddActiveParticle(IParticleEmitter* pEmitter);
	void RemoveDeactiveParticle(IParticleEmitter* pEmitter);

private:
	static ParticleManager* sParticleManager;
	typedef std::vector< SmartPtr<IParticleEmitter> > Emitters;
	Emitters mActiveParticles;
	Emitters mPendingDeletes;

	// cache
	typedef std::map<unsigned, SmartPtr<IParticleEmitter> > PARTICLE_EMITTERS_BY_ID;
	PARTICLE_EMITTERS_BY_ID mParticleEmitters;
	typedef std::map<std::string, SmartPtr<IParticleEmitter> > PARTICLE_EMITTERS_BY_NAME;
	PARTICLE_EMITTERS_BY_NAME mParticleEmittersByName;

};
}