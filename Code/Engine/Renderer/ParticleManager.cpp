#include <Engine/StdAfx.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/Renderer/ParticleEmitter.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>
namespace fastbird
{

ParticleManager* ParticleManager::sParticleManager=0;
void ParticleManager::InitializeParticleManager()
{
	sParticleManager = new ParticleManager();
}
ParticleManager& ParticleManager::GetParticleManager()
{
	assert(sParticleManager);
	return *sParticleManager;
}
void ParticleManager::FinalizeParticleManager()
{
	SAFE_DELETE(sParticleManager);
}

ParticleManager::ParticleManager()
{
}

ParticleManager::~ParticleManager()
{
	FB_FOREACH(it, mActiveParticles)
	{
		(*it)->Active(false);
	}

	mActiveParticles.clear();
	mPendingDeletes.clear();

	
	mParticleEmitters.clear();
	mParticleEmittersByName.clear();
	ParticleRenderObject::FinalizeRenderObjects();
}

void ParticleManager::Update(float elapsedTime)
{
	ParticleRenderObject::ClearParticles();

	for each(IParticleEmitter* p in mPendingDeletes)
	{
		mActiveParticles.erase(
			std::remove(mActiveParticles.begin(), mActiveParticles.end(), p), 
			mActiveParticles.end());
	}
	mPendingDeletes.clear();

	Emitters::iterator it = mActiveParticles.begin();
	for (; it!=mActiveParticles.end(); )
	{
		bool updated = (*it)->Update(elapsedTime);
		if (!updated)
			it = mActiveParticles.erase(it);
		else
			it++;
	}
}

void ParticleManager::AddActiveParticle(IParticleEmitter* pEmitter)
{
	mActiveParticles.push_back(pEmitter);	
}

void ParticleManager::RemoveDeactiveParticle(IParticleEmitter* pEmitter)
{
	mPendingDeletes.push_back(pEmitter);
}

IParticleEmitter* ParticleManager::GetParticleEmitter(const char* file)
{
	PARTICLE_EMITTERS_BY_NAME::iterator found =mParticleEmittersByName.find(file);
	if (found != mParticleEmittersByName.end())
	{
		return (IParticleEmitter*)found->second->Clone();
	}

	SmartPtr<IParticleEmitter> p = IParticleEmitter::CreateParticleEmitter();
	bool succ = p->Load(file);
	if (succ)
	{
		mParticleEmittersByName[file] = p;
		return (IParticleEmitter*)p->Clone();
	}
		
	return 0;
}

IParticleEmitter* ParticleManager::GetParticleEmitter(unsigned id)
{
	PARTICLE_EMITTERS_BY_ID::iterator found = mParticleEmitters.find(id);
	if (found != mParticleEmitters.end())
		return (IParticleEmitter*)found->second->Clone();

	return 0;
}

}