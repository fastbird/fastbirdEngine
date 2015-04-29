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
	static std::string FindParticleNameWithID(const char* particleFolder, unsigned id);

	void Update(float elapsedTime);
	void RenderProfile();

	IParticleEmitter* GetParticleEmitter(const char* file);
	IParticleEmitter* GetParticleEmitter(unsigned id);

	void ReloadParticle(const char* file);

	void EditThisParticle(const char* path);
	void ScaleEditingParticle(float scale);

	unsigned GetNumActiveParticles() const;

private:
	friend class ParticleEmitter;
	void AddActiveParticle(IParticleEmitter* pEmitter);
	void RemoveDeactiveParticle(IParticleEmitter* pEmitter);
	void AddActiveParticlePending(IParticleEmitter* pEmitter);

private:
	static ParticleManager* sParticleManager;
	typedef std::vector< SmartPtr<IParticleEmitter> > Emitters;
	Emitters mActiveParticles;
	Emitters mPendingAdds;
	Emitters mPendingDeletes;
	FB_CRITICAL_SECTION mActivePCS;
	FB_CRITICAL_SECTION mPendingDCS;
	FB_CRITICAL_SECTION mGetPCS;
	FB_CRITICAL_SECTION mGetPNameCS;

	// cache
	typedef std::map<unsigned, SmartPtr<IParticleEmitter> > PARTICLE_EMITTERS_BY_ID;
	PARTICLE_EMITTERS_BY_ID mParticleEmitters;
	typedef std::map<std::string, SmartPtr<IParticleEmitter> > PARTICLE_EMITTERS_BY_NAME;
	PARTICLE_EMITTERS_BY_NAME mParticleEmittersByName;
	
	SmartPtr<IParticleEmitter> mEditingParticle;
	size_t mEditCamIdx;
	Vec3 mEditingPos;
};
}