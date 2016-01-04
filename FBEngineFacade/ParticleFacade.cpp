/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "ParticleFacade.h"
#include "EngineFacade.h"
#include "FBSceneManager/Scene.h"
#include "FBParticleSystem/ParticleEmitter.h"
#include "FBParticleSystem/ParticleSystem.h"
using namespace fb;
class ParticleFacade::Impl{
public:
	ParticleEmitterPtr mParticleEmitter;

};

//---------------------------------------------------------------------------
std::vector<ParticleFacadeWeakPtr> sEmitters;

ParticleFacadePtr ParticleFacade::Create(){
	ParticleFacadePtr p(new ParticleFacade, [](ParticleFacade* obj){ delete obj; });
	sEmitters.push_back(p);
	return p;
}

ParticleFacade::ParticleFacade()
	: mImpl(new Impl)
{

}

ParticleFacade::~ParticleFacade(){
	for (auto it = sEmitters.begin(); it != sEmitters.end(); /**/)
	{
		IteratingWeakContainer(sEmitters, it, emitter);
	}
}

bool ParticleFacade::Load(unsigned id){
	auto mainScene = EngineFacade::GetInstance().GetMainScene();
	return Load(mainScene, id);
}

bool ParticleFacade::Load(IScenePtr scene, unsigned id){
	if (!scene){
		Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg.");
		return false;
	}
	mImpl->mParticleEmitter = ParticleSystem::GetInstance().GetParticleEmitter(scene, id);
	return mImpl->mParticleEmitter != 0;
}

bool ParticleFacade::IsValid() const{
	return mImpl->mParticleEmitter != 0;
}

void ParticleFacade::SetTransformation(const Transformation& t){
	if (mImpl->mParticleEmitter){
		mImpl->mParticleEmitter->SetLocation(t);
	}
}

const Transformation& ParticleFacade::GetTransformation() const{
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->GetLocation();
	}
	return Transformation::IDENTITY;
}

void ParticleFacade::SetPosition(const Vec3& pos){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetPosition(pos);
	}
}

void ParticleFacade::Active(bool active, bool pending){
	if (mImpl->mParticleEmitter){
		mImpl->mParticleEmitter->Active(active, pending);
	}
}

bool ParticleFacade::IsActive() const{
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->IsActive();
	}
	return false;
}

void ParticleFacade::StopImmediate(){
	if (mImpl->mParticleEmitter){
		mImpl->mParticleEmitter->StopImmediate();
	}
}

bool ParticleFacade::GetVisible(){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->GetVisible();
	}
	return false;
}

void ParticleFacade::SetVisible(bool visible){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetVisible(visible);
	}
}

float ParticleFacade::GetAlpha() const{
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->GetAlpha();
	}
	return 0.f;
}

void ParticleFacade::SetAlpha(float alpha){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetAlpha(alpha);
	}
}

void ParticleFacade::SetScale(const Vec3& scale){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetScale(scale);
	}
}

void ParticleFacade::SetLength(float length){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetLength(length);
	}
}

bool ParticleFacade::IsAlive() const{
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->IsAlive();
	}
	return false;
}

void ParticleFacade::Stop(){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->Stop();
	}
}

void ParticleFacade::RemoveShaderDefine(const char* def){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->RemoveShaderDefine(def);
	}
}

void ParticleFacade::AddShaderDefine(const char* def, const char* val){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->AddShaderDefine(def, val);
	}
}

//void ParticleFacade::ApplyShaderDefine(){
//	if (mImpl->mParticleEmitter){
//		return mImpl->mParticleEmitter->ApplyShaderDefine();
//	}
//}

void ParticleFacade::SetRelativeVelocity(const Vec3& dir, float speed){
	if (mImpl->mParticleEmitter){
		return mImpl->mParticleEmitter->SetRelativeVelocity(dir, speed);
	}
}

void ParticleFacade::SetTeamColor(const Color& color){
	if (mImpl->mParticleEmitter){
		mImpl->mParticleEmitter->SetTeamColor(color);
	}
}